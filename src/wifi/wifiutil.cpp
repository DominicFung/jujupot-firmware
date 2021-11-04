#include <WiFi.h>
#include "time.h"

#include "secret_wifi.h"
#include <Preferences.h>
#include "../storage.h"

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int status = WL_IDLE_STATUS;
const int _max_retry_wifi = 10;
char TEMP_WIFI[50];
char TEMP_PASS[50];

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

/* If you call connect_wifi AND no wifi was previously saved, 
    we use default WIFI: stored in secrete_wifi.h
    
    NOTE: we only do this IF this is test firmware
*/
void connect_wifi() {
  int tick = 0;
  if (strcmp(TEMP_WIFI, "") != 0 && strcmp(TEMP_PASS, "") != 0) {
    while (status != WL_CONNECTED && tick < _max_retry_wifi) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(TEMP_WIFI);
      status = WiFi.begin(TEMP_WIFI, TEMP_PASS);
      delay(4000);
      tick++;
    }
  } else {
    while (status != WL_CONNECTED && tick < _max_retry_wifi) {
      Serial.print("Attempting to connect to (TEST) SSID: ");
      Serial.println(WIFI_SSID);
      status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      delay(4000);
      tick++;
    }
  }

  if (status == WL_CONNECTED) {
    delay(3000);
    Serial.println("Connected to wifi");

    delay(500);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("Time configured.");

    delay(2000);
  } else {
    Serial.print("After ");
    Serial.print(_max_retry_wifi);
    Serial.println(" retries, we still can't connect. Sleeping indefinitely.");
    
    esp_deep_sleep_start();
  }
}

void disconnect_wifi() {
  status = WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  Serial.print("Wifi Status: ");
  Serial.print(status);
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