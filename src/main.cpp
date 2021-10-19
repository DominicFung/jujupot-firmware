#include <Arduino.h>
#include <string>

#include <ArduinoJson.h>
#include <AWS_IOT.h>
#include <WiFi.h>

#include <driver/rtc_io.h>
#include "BluetoothSerial.h"

#include "awscomm.h"
#include "potshadow.h"
#include "wifiutil.h"
#include "potutil.h"

#define onboard 2    // ESP32 LED pin
char bluetoothPrefix[] = "JuJuPot-";

char productId[] = "58109219-d923-49fc-b349-d713f2c7d2a3";
char verificationId[] = "204ed6d7-efb4-4b55-99f1-50704d984219";
char potType[] = "small-planter-v1";
char color[] = "blue-white-v1";
char plant[] = "sedum-morganianum_costco_pid";
int fwVersion = 1;

const int num_reads = 10;
int tick=0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void flash_led() {
  digitalWrite(onboard, LOW);
  delay(1000);
  digitalWrite(onboard, HIGH);
  delay(1000);
  digitalWrite(onboard, LOW);
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

void step_1_read_shadow() {
  digitalWrite(onboard, HIGH);
  connect_wifi();
  aws_connect();
  delay(2000);

  std::string old_shadow = aws_get_shadow();
  char json[old_shadow.length()+1];
  strcpy(json, old_shadow.c_str());
  load_shadow(json);

  disconnect_wifi();

  digitalWrite(onboard, LOW);
  delay(1000);
}

void step_2_sensor_input() {
  init_shadow(productId, fwVersion);
  add_pot_info(potType, color, plant);
  add_signoff(verificationId);

  int svalue = check_moisture();
  add_sensor_value("moisture", airvalue, watervalue, 6, svalue);

  int nlvalue = check_nlight();
  add_sensor_value("light1", lightvaluehigh, lightvaluelow, 6, nlvalue);
}

void step_3_write_shadow() {
  digitalWrite(onboard, HIGH);
  connect_wifi();
  aws_connect();
  delay(2000);

  unsigned long time = get_time();
  std::string output = get_new_shadow(time);
  aws_send(output);
  tick=0;

  digitalWrite(onboard, LOW);

  // wait for aknowledgment
  while (1) {
    digitalWrite(onboard, HIGH);
    if(msgReceived > 0) {
      Serial.print("Recieved Message Status: ");
      Serial.println(msgReceived);

      Serial.print("Received Message: ");
      Serial.println(rcvdPayload);

      msgReceived = 0;

      // ESP light sleep
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
      esp_light_sleep_start();
    } else {
      tick++;
      if(tick >= 5) {
        tick = 0;
        
        Serial.println("Resending ...");
        aws_send(output);
      }
      delay(500);
      digitalWrite(onboard, LOW);
      delay(500);
    }
  }
}

int bluetooth_loop() {
  // if (Serial.available()) {
  //   SerialBT.write(Serial.read());
  // }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());

    // 1 char at a time!
  }
  delay(20);

  return 1;
}

void run_bluetooth() {
  int p = strlen(productId);
  std::string post = std::string(productId).substr(p-5, p-1);
  std::string bt = bluetoothPrefix + post;

  char btname[bt.length()+1];
  strcpy(btname, bt.c_str());
  SerialBT.begin(btname);

  int bt_status = 1;
  while(bt_status) {
    bt_status = bluetooth_loop();
  }
}

void setup() {
  pinMode(onboard, OUTPUT);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Running Setup ..");

  // IF USER is defined ,, this device is activated and we should update the cloud
  //step_1_read_shadow();
  //step_2_sensor_input();
  //step_3_write_shadow();

  rtc_gpio_pulldown_en((gpio_num_t) GPIO_NUM_34);
  esp_sleep_enable_ext0_wakeup((gpio_num_t) GPIO_NUM_34, RISING);

  // ESP light sleep
  // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  // esp_light_sleep_start();
}

// Forgo loop usage?
void loop() {
  int GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println((log(GPIO_reason))/log(2), 0);

  esp_sleep_wakeup_cause_t w_reason;
  w_reason = print_wakeup_reason();
  
  if (w_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("ACTIVATE BLUETOOTH");


  }

  flash_led();
  Serial.println("Going to sleep ...");
  esp_deep_sleep_start();
}