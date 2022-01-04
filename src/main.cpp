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

#define onboard        2   // ESP32 LED pin
#define SLEEP_HOURS    1
#define TIME_TO_SLEEP  1   /* Time ESP32 will go to sleep (in seconds) */

char productId[] = "58109219-d923-49fc-b349-d713f2c7d2a3";
char verificationId[] = "204ed6d7-efb4-4b55-99f1-50704d984219";
char potType[] = "small-planter-v1";
char color[] = "blue-white-v1";
char plant[] = "sedum-morganianum_costco_pid";
int fwVersion = 1;

const char * TEMP_USER;

int tock = 0;

int _wifi_state = WL_IDLE_STATUS;

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

              await_get_shadow();

              init_shadow(productId, fwVersion);
              add_device_id(CLIENT_ID);
              add_signoff(verificationId);

              unsigned long time = get_time();
              std::string output = get_new_shadow(time);

              int n = output.length();
              char payload2[n+1];
              strcpy(payload2, output.c_str());

              update_shadow(payload2);

              
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