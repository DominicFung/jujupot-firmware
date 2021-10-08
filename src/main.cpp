#include <Arduino.h>
#include <string>

#include <ArduinoJson.h>
#include <AWS_IOT.h>
#include <WiFi.h>

#include "awsutil.h"

/**
 * PLEASE NOTE:
 *  - 15 = p15
 *  - 12 = p32
 *  - 25 = p27
 *  - 26 = p26 
 *  - 27 = p25
 */

#define onboard 2    // ESP32 LED pin
#define soilpin 15   // Capacitive Soil  Moisture Sensor v1.2

#define nldrpin 12  // North LDR 
#define eldrpin 25  // East LDR
#define sldrpin 26  // South LDR
#define wldrpin 27  // West LDR

#define temppin 13  // LM35 Temp sensor

int status = WL_IDLE_STATUS;

char WIFI_SSID[]="WF5B";                // your Wifi SSID
char WIFI_PASSWORD[]="908EE03518";      // Wifi Password

const int num_reads = 10;

// Moisture Sensor
const int airvalue = 4095;
const int watervalue = 2023;
int soilmoisturevalue=0;

// Light Level
const int lightinit = 100;
int lightvalue=0;


int tick=0;

void connect_wifi_aws(const int &smv) {
  digitalWrite(onboard, HIGH);
  while (status != WL_CONNECTED)
  {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(WIFI_SSID);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

      // wait 5 seconds for connection:
      delay(5000);
  }

  Serial.println("Connected to wifi");
  aws_connect();
  delay(2000);

  //send data
  StaticJsonDocument<200> doc;
  doc["unixtime"] = 1632682156;
  doc["deviceid"] = "test001";

  JsonObject soilmoisture = doc.createNestedObject("soilmoisture");
  soilmoisture["watervalue"] = watervalue;
  soilmoisture["airvalue"] = airvalue;
  soilmoisture["value"] = smv;

  std::string output;
  serializeJson(doc, output);

  Serial.print("doc: ");
  Serial.println(output.c_str());

  aws_send(output); //"{\"soilmosture\" : { \"min\" : \"\"}"
  tick=0;

  digitalWrite(onboard, LOW);

  // wait for aknowledgment
  while (1) {
    digitalWrite(onboard, HIGH);
    if(msgReceived == 1) {
      msgReceived = 0;
      Serial.print("Received Message:");
      Serial.println(rcvdPayload);

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

void disconnect_wifi() {
  status = WiFi.disconnect();
  Serial.print("Wifi Status: ");
  Serial.print(status);
}

void check_moisture() {
  int tick = 0;
  int MULTI_READ = 6;
  
  while (tick < MULTI_READ) {
    soilmoisturevalue = analogRead(soilpin);
    Serial.print("Soil Mosture Reading: ");
    Serial.println(soilmoisturevalue);
    tick++;

    delay(1000);
  }

  Serial.print("FINAL Soil Mosture Reading: ");
  Serial.println(soilmoisturevalue);
}

void check_nlight() {
  int tick = 0;
  int MULTI_READ = 6;
  
  while (tick < MULTI_READ) {
    lightvalue = analogRead(nldrpin);
    Serial.print("North LDR Reading: ");
    Serial.println(lightvalue);
    tick++;

    delay(1000);
  }

  Serial.print("FINAL North LDR Reading: ");
  Serial.println(lightvalue);
}

void setup() {
  pinMode(onboard, OUTPUT);
  //pinMode(nldrpin, INPUT);

  Serial.begin(9600);
  Serial.println("Running Setup ..");

  // aws_connect();
  // aws_get_shadow();
  // disconnect_wifi();

  // aws_connect();
  // aws_get_shadow();
  // disconnect_wifi();

}


// Forgo loop usage?
void loop() {
  //[E][esp32-hal-adc.c:135] __analogRead(): GPIO15: ESP_ERR_TIMEOUT: ADC2 is in use by Wi-Fi.
  
  // Read to average out ..
  if (tick < num_reads) {
    check_moisture();
    check_nlight();

    // tick++;
  }

  // Finish Read, start send
  if (tick == num_reads) {
    // connect_wifi_aws(soilmoisturevalue);
    // -- END --
  }
  
  Serial.println("loop complete.");
  delay(1000);
}