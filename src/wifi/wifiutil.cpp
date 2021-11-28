#include <WiFi.h>
#include "time.h"

#include "secret_wifi.h"
#include <Preferences.h>
#include "../storage.h"

#define WIFI_TIMEOUT_MS 20000 // 20 second WiFi connection timeout
#define WIFI_RECOVER_TIME_MS 30000

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int status = WL_IDLE_STATUS;
const int _max_retry_wifi = 10;
char TEMP_WIFI[50] = "\0";
char TEMP_PASS[50] = "\0";

void print_connected_with_mac() {
  byte mac[6]; 

  if ( WiFi.status() != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } else {
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[5],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.println(mac[0],HEX);
  }
}

bool load_stored_wifi() {
  Serial.print("Loading preference from ");
  Serial.println(preference_name);

  preferences.begin(preference_name, true);
  strcpy(TEMP_WIFI, preferences.getString(wifi_ssid_key, "").c_str());
  Serial.print("Loaded Wifi: ");
  Serial.println(TEMP_WIFI);

  strcpy(TEMP_PASS, preferences.getString(wifi_password, "").c_str());
  Serial.print("Loaded Pass: ");
  Serial.println(TEMP_PASS);
  preferences.end();

  return strcmp(TEMP_WIFI, "") != 0 && strcmp(TEMP_PASS, "") != 0;
}

void keepWiFiAlive(void * parameter){
  for(;;){
      if(WiFi.status() == WL_CONNECTED){
          vTaskDelay(10000 / portTICK_PERIOD_MS);
          continue;
      }

      Serial.println("[WIFI] Connecting .. ");
      WiFi.mode(WIFI_STA);

      if (TEMP_WIFI[0] == '\0' || TEMP_PASS[0] == '\0') {
        Serial.print("Wifi: ");
        Serial.print(WIFI_SSID);
        Serial.print(" ");
        Serial.println(TEMP_WIFI);

        Serial.print("Pass: ");
        Serial.print(WIFI_PASSWORD);
        Serial.print(" ");
        Serial.println(TEMP_PASS);

        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      } else {
        if (strcmp(TEMP_WIFI, "") != 0 && strcmp(TEMP_PASS, "") != 0) {
          WiFi.begin(TEMP_WIFI, TEMP_PASS);
        } else {
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
      }
      
      unsigned long startAttemptTime = millis();

      // Keep looping while we're not connected and haven't reached the timeout
      while (WiFi.status() != WL_CONNECTED && 
              millis() - startAttemptTime < WIFI_TIMEOUT_MS){
                delay(200);
              }

      // When we couldn't make a WiFi connection (or the timeout expired)
      // sleep for a while and then retry.
      if(WiFi.status() != WL_CONNECTED){
        Serial.println("[WIFI] FAILED");
        vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
        continue;
      }

      delay(200);
      //Serial.println("[WIFI] Connected: " + String(WiFi.localIP()));
      print_connected_with_mac();

      delay(500);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      Serial.println("Time configured.");
  }
}

unsigned long get_time() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}