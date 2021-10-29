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

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

char bluetoothPrefix[] = "JuJuPot-";

Preferences preferences;
char preference_name[] = "juju-app";
char wifi_ssid_key[] = "wifi_ssid";
char wifi_password[] = "wifi_password";

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
      Serial.println("MyCallbacks onRead");

      std::string value = pCharacteristic->getValue();
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.println("MyCallbacks onWrite");
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();

        preferences.begin(preference_name, false);
        if (value.rfind("wifi:: ", 0) == 0) {
          const char * temp_ssid = value.substr(7).c_str();
          
          Serial.print("Saving WIFI ssid to preference .. ");
          Serial.println(temp_ssid);

          preferences.putString(wifi_ssid_key , temp_ssid);
        } else if (value.rfind("pass:: ", 0) == 0) {
          const char * temp_pass = value.substr(7).c_str();

          Serial.print("Saving WIFI pass to preference .. ");
          Serial.println(temp_pass);

          preferences.putString(wifi_password , temp_pass);
        } else {
          Serial.print("Could not find preference category (wifi:: || pass::)");

          char val[value.length()+1];
          strcpy(val, value.c_str());

          Serial.println(val);
        }

        preferences.end();
        Serial.println("*********");
      }
    }
}; 

int bluetooth_loop() {
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

      pCharacteristic->setValue("Juju-pot connected.");
      pCharacteristic->notify();

      Serial.println("BLE App sucessfully connected.");
    }

    return 1;
}

void run_bluetooth(const char productId[37]) {
  int p = strlen(productId);
  std::string post = std::string(productId).substr(p-5, p-1);
  std::string bt = bluetoothPrefix + post;

  char btname[bt.length()+1];
  strcpy(btname, bt.c_str());
  BLEDevice::init(btname);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                     CHARACTERISTIC_UUID,
                                     BLECharacteristic::PROPERTY_READ   |
                                     BLECharacteristic::PROPERTY_WRITE  |
                                     BLECharacteristic::PROPERTY_NOTIFY |
                                     BLECharacteristic::PROPERTY_INDICATE
                                     );

  //pCharacteristic->setValue("ESP32 TEST");
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setReadProperty(true);
  pCharacteristic->setCallbacks(new MyCallbacks());
  //pCharacteristic->setValue("Sending love from Juju-pot <3");

  pService->addCharacteristic(pCharacteristic);
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  Serial.println("Waiting a client connection to notify...");

  int bt_status = 1;
  while(bt_status) {
    bt_status = bluetooth_loop();
  }
}