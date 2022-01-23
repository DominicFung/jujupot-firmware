#include <Arduino.h>
#include <WiFi.h>

#include <Preferences.h>
#include <driver/rtc_io.h>

#include "pot/potshadow.h"
#include "wifi/wifiutil.h"
#include "aws/awscomm.h"
#include "pot/potutil.h"
#include "ble/bt.h"

#include "secret_aws.h"
#include "storage.h"
#include "product_config.h"

#define onboard        2   // ESP32 LED pin
#define SLEEP_HOURS    1
#define TIME_TO_SLEEP  1   /* Time ESP32 will go to sleep (in seconds) */

const char* TEMP_USER;
int tock = 0;

int _wifi_state = WL_IDLE_STATUS;

// Set up the rgb led names
uint8_t ledR = 23;
uint8_t ledG = 22;
uint8_t ledB = 21; 

uint8_t ledArray[3] = {1, 2, 3}; // three led channels

uint8_t color = 0;          // a value from 0 to 255 representing the hue
uint32_t R, G, B;           // the Red Green and Blue color components
uint8_t brightness = 255;  // 255 is maximum brightness, but can be changed.  Might need 256 for common anode to fully turn off.

void RGB_color(int red_light_value, int green_light_value, int blue_light_value) {
  ledcWrite(1,   red_light_value);
  ledcWrite(2, green_light_value);
  ledcWrite(3,  blue_light_value);
}

void flash_led(int times) {
  int tock2 = 0;
  while (tock2 < times) {
    delay(200);
    digitalWrite(onboard, HIGH);
    delay(200);
    digitalWrite(onboard, LOW);
    tock2++;
  }
}

esp_sleep_wakeup_cause_t print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }

  return wakeup_reason;
}

void clear_preferences() {
  Serial.print("Clearing preferences: ");
  Serial.print(preference_name);
  Serial.println(" ...");

  preferences.begin(preference_name, false);
  preferences.clear();

  // WE NEED TO PUT BACK "Configure" STATE:
  preferences.putChar(preference_state_id, 'C');

  preferences.end();
}

// BT button - used for turning on BT even after device is activated.
void check_bt_button(void * parameter) {
  for(;;) {
    int push_state = digitalRead(GPIO_NUM_34);
    if ( push_state == HIGH ) {
      Serial.println("-- RESET BUTTON PRESS --");

      preferences.begin(preference_name, false);
      preferences.putChar(preference_state_id, 'S'); // TODO: C
      preferences.end();

      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      esp_deep_sleep_start();
    }

    delay(200);
  }
}

/**
 * @brief 
 * Before sensor_work, we expect the shadow (JSON) to already be initialized
 */
void sensor_work(void * parameter) {
  for (;;) {
    Serial.println(" -- seonsor work -- ");

    int val = check_moisture();
    Serial.print("Final moisture value: ");
    Serial.println(val);

    // update Shadow (object)

    int sensor_delay_hours = 2;
    delay(sensor_delay_hours * 60UL * 60UL * 1000UL); 
  }
}

void setup() {
    pinMode(onboard, OUTPUT);
    Serial.begin(9600); // 115200 by default
    delay(2000);
    Serial.println('\n');
    flash_led(4);
    
    ledcAttachPin(ledR, 1);
    ledcAttachPin(ledG, 2);
    ledcAttachPin(ledB, 3);

    ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcSetup(2, 12000, 8);
    ledcSetup(3, 12000, 8);

    RGB_color(0, 0, 0); // red

    // We set the wake, since device is asleep when we make the device.
    rtc_gpio_pulldown_en((gpio_num_t) GPIO_NUM_34);
    esp_sleep_enable_ext0_wakeup((gpio_num_t) GPIO_NUM_34, RISING);
    pinMode(GPIO_NUM_34, INPUT);

    int GPIO_reason = esp_sleep_get_ext1_wakeup_status();
    Serial.print("GPIO that triggered the wake up: GPIO ");
    Serial.println((log(GPIO_reason))/log(2), 0);

    esp_sleep_wakeup_cause_t w_reason;
    w_reason = print_wakeup_reason();

    // lets get the _STATE from preferences
    preferences.begin(preference_name, true);
    _STATE = preferences.getChar(preference_state_id, 'N');
    Serial.print("_STATE: ");
    Serial.println(_STATE);
    preferences.end();

    if (_STATE == 'C' || w_reason == ESP_SLEEP_WAKEUP_EXT0) {
      Serial.print("Configure Wake Reason: ");
      Serial.print(_STATE);
      Serial.print(" || ");
      Serial.println(ESP_SLEEP_WAKEUP_EXT0);
      
      // clear_preferences();
      run_bluetooth(productId);

      Serial.println("run_bluetooth stopped. SetState and Sleeping.");

      //_STATE = 'S'; // should save over quick sleep (restart).
      preferences.begin(preference_name, false);
      preferences.putChar(preference_state_id, 'S');
      preferences.end();

      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      esp_deep_sleep_start();

    } else if (_STATE == 'S') {
      preferences.begin(preference_name, true);
      TEMP_USER = preferences.getString(user_id_key, "").c_str();
      preferences.end();

      if (strcmp(TEMP_USER, "") != 0) {
        Serial.print("User Found: ");
        Serial.println(TEMP_USER);

        if ( load_stored_wifi() ) {
          xTaskCreatePinnedToCore(
            keepWiFiAlive,
            "keepWiFiAlive",  // Task name
            5000,             // Stack size (bytes)
            NULL,             // Parameter
            0,                // Task priority
            NULL,             // Task handle
            0 /* Core where the task should run */
          );

          xTaskCreatePinnedToCore(
            check_bt_button,
            "checkBtButton",  // Task name
            5000,             // Stack size (bytes)
            NULL,             // Parameter
            1,                // Task priority
            NULL,             // Task handle
            0 /* Core where the task should run */
          );

          delay(3000);
        } else {
          // Never set a sleep time. This will help us conserver battery life.
          // Wake button will still be available.
          Serial.println("WIFI ssid/pass is not available, No next wake time set.");
          
          preferences.begin(preference_name, false);
          preferences.putChar(preference_state_id, 'N');
          preferences.end();

          esp_deep_sleep_start();
        }
      } else {
        Serial.println("User is not available, sleeping indefinitely.");

        preferences.begin(preference_name, false);
        preferences.putChar(preference_state_id, 'N');
        preferences.end();

        esp_deep_sleep_start();
      }
    } else if (_STATE == 'N') {
      Serial.println(" -- STATE == N, going to sleep -- ");
      esp_deep_sleep_start();
    } else {
      Serial.println(" ERROR: We cannot figure out the state. sleeping.");
      esp_deep_sleep_start();
    }
}


/* 
  We hit this loop if _STATE = S = "Standard"
  core 1 with priority 1.

  Loops main purpose is to make sure AWS is connected.
*/
void loop() {
    if (_STATE == 'S') {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[AWS] Detected disconnected Wifi - returning");
        _wifi_state = WiFi.status();
        delay(2000);
        return;
      } else if (WiFi.status() == WL_CONNECTED && _wifi_state != WL_CONNECTED) {
        Serial.println("[AWS] Waiting for WiFi connection ..");
        _wifi_state = WL_CONNECTED;
        aws_connect();

        /* ***********************************************************
         *    ON CONNECTION PROCEEDURE (1 TIME)
         * *********************************************************** */
              char shadowfromcloud[512];
              await_get_shadow(shadowfromcloud);
              if (std::string(shadowfromcloud).find("product-id") != std::string::npos) {
                Serial.println("[AWS] Shadow found loading as JSON ...");
                if (load_shadow(shadowfromcloud)) {
                  Serial.println("[JSON] Shadow loaded successfully.");
                  return;
                } else {
                  Serial.println("[JSON] ERROR did not load properly. EXIT");
                  while(1); // generally we expect shadows that pass AWS iot core to be valid.
                }
              } else {
                Serial.println("[AWS] Shadow not found with product-id. Creating new one.");
                Serial.println("====== INIT SHADOW ======");

                init_shadow(productId, fwVersion);
                add_device_id(CLIENT_ID);
                add_signoff(verificationId);

                load_init_sensor(sensor_init_json);

                Serial.println(controlable_init_json);
                load_init_controlable(controlable_init_json);

                unsigned long time = get_time();
                std::string output = get_new_shadow(time);

                int n = output.length();
                char payload2[n+1];
                strcpy(payload2, output.c_str());

                update_shadow(payload2);
              }
        /* ***********************************************************/
        /* ***********************************************************/

      } else {
        // Connected and runs continously.
        if (tock >= 5) { tock=0; keep_alive(); }
        vTaskDelay(12 * 1000 / portTICK_RATE_MS); 
        tock++;
      }
    } else {
      Serial.println(" ERROR: In loop but _STATE is not S. sleeping.");
      esp_deep_sleep_start();
    }
}