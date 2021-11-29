#include <Arduino.h>
#include <string>

#include <Preferences.h>
#include "../storage.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* Time ESP32 will go to sleep (in seconds) */

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristicWrite = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

char bluetoothPrefix[] = "JuJuPot-";

Preferences preferences;
char preference_name[] = "juju-app";

/**
 * @brief 
 *  - N = New       - not all aspects are configured
 *  - C = Configure - Bluetooth is turned on for configuration
 *  - S = Standard  - Wifi is turned on and connected to AWS. 
 *                    Used when UserId, Wifi, Password is defined.
 */
char preference_state_id[] = "jstate";
char _STATE = 'N';

char wifi_ssid_key[] = "wifi_ssid";
char wifi_password[] = "wifi_password";
char user_id_key[] = "juju_user_id";

char temp_ssid[50];
char temp_pass[50];
char temp_userid[50];

std::string product_prefix = "product:: ";
bool _INCONFIG = true;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("MyServerCallback: onConnect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("MyServerCallback: onDisconnect");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) {
      Serial.println("== Sending Product ID ==");
      std::string sendproduct = product_prefix + productId;
      pCharacteristic->setValue(sendproduct);
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        preferences.begin(preference_name, false);
        Serial.print("Begin preferences: ");
        Serial.print(preference_name);
        Serial.println(" ...");

        if (value.rfind("wifi:: ", 0) == 0) {
          strcpy(temp_ssid, value.substr(7).c_str());
          
          Serial.print("Saving WIFI ssid: ");
          Serial.print(temp_ssid);

          int l = strlen(temp_ssid);
          Serial.print(", len: ");
          Serial.println(l);
          if (temp_ssid[l] == '\0') { 
            Serial.println("(OK) has terminator.");
          } else { 
            Serial.println("(NOT OK). Adding null term.");
          }

          preferences.putString(wifi_ssid_key, temp_ssid);
        } else if (value.rfind("pass:: ", 0) == 0) {
          strcpy(temp_pass, value.substr(7).c_str());

          Serial.print("Saving WIFI pass: ");
          Serial.print(temp_pass);

          int l = strlen(temp_pass);
          Serial.print(", len: ");
          Serial.println(l);
          if (temp_ssid[l] == '\0') { Serial.println("(OK) has terminator."); }
          else { Serial.println("(NOT OK)"); }

          preferences.putString(wifi_password, temp_pass);
        } else if (value.rfind("user:: ", 0) == 0){
          strcpy(temp_userid, value.substr(7).c_str());
          
          Serial.print("Saving User:      ");
          Serial.print(temp_userid);

          int l = strlen(temp_userid);
          Serial.print(", len: ");
          Serial.println(l);
          if (temp_ssid[l] == '\0') { Serial.println("(OK) has terminator."); }
          else { Serial.println("(NOT OK)"); }

          preferences.putString(user_id_key, temp_userid);
        } else {
          Serial.print("Could not find preference category (wifi:: || pass:: || user::)");
          char val[value.length()+1];
          strcpy(val, value.c_str());
          Serial.println(val);
        }

        preferences.end();
        Serial.println("*********");
        Serial.println();

        if (strlen(temp_ssid) != 0 && strlen(temp_pass) != 0 && strlen(temp_userid) != 0) {
          Serial.println("wifi/pass & userid defined. Will now sleep and wake to register device.");
          
          delay(1000);
          _INCONFIG = false;

          // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
          // esp_deep_sleep_start();
        }
      }
    }
}; 

bool bluetooth_loop() {
  // notify changed value
    if (deviceConnected) {
      /*  bluetooth stack will go into congestion, if too many packets are sent, 
           - in 6 hours test i was able to go as low as 3ms - (author)
           - 100 seems stable. - Dom
      */
      delay(100); // 3
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }

    // connecting
    if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
      Serial.println("BLE App sucessfully connected.");
    }

    return _INCONFIG;
}

void bt_timeout (void * parameter) {
  Serial.println("Bluetooth Timeout timer started. 20 mins");

  int bt_timeout_minutes = 20;
  delay(bt_timeout_minutes * 60UL * 1000UL); 

  Serial.println("Bluetooth Timeout complete.");

  preferences.begin(preference_name, false);
  preferences.putChar(preference_state_id, 'S');
  preferences.end();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void run_bluetooth(const char productId[37]) {
  int p = strlen(productId);
  std::string post = std::string(productId).substr(p-5, p-1);
  std::string bt = bluetoothPrefix + post;

  // set a timeout:
  xTaskCreatePinnedToCore(
    bt_timeout,
    "bt_timeout",  // Task name
    5000,             // Stack size (bytes)
    NULL,             // Parameter
    0,                // Task priority
    NULL,             // Task handle
    0 /* Core where the task should run */
  );

  char btname[bt.length()+1];
  strcpy(btname, bt.c_str());
  BLEDevice::init(btname);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristicWrite = pService->createCharacteristic(
                                     CHARACTERISTIC_UUID,
                                     BLECharacteristic::PROPERTY_READ   |
                                     BLECharacteristic::PROPERTY_WRITE  |
                                     BLECharacteristic::PROPERTY_NOTIFY |
                                     BLECharacteristic::PROPERTY_INDICATE |
                                     BLECharacteristic::PROPERTY_WRITE_NR |
                                     BLECharacteristic::PROPERTY_BROADCAST
                                    );

  pCharacteristicWrite->addDescriptor(new BLE2902());
  pCharacteristicWrite->setReadProperty(true);
  pCharacteristicWrite->setWriteProperty(true);
  pCharacteristicWrite->setNotifyProperty(true);
  pCharacteristicWrite->setCallbacks(new MyCallbacks());

  pService->addCharacteristic(pCharacteristicWrite);
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  pServer->startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  while(_INCONFIG) { bluetooth_loop(); }
}