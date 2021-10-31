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

void connect_wifi() {
  preferences.begin(preference_name, false);
  const char * TEMP_WIFI = preferences.getString(wifi_ssid_key, "").c_str();
  Serial.print("Saved Wifi: ");
  Serial.print(TEMP_WIFI);

  const char * TEMP_PASS = preferences.getString(wifi_password, "").c_str();
  Serial.print("Saved Pass: ");
  Serial.print(TEMP_PASS);
  preferences.end();

  if (strcmp(TEMP_WIFI, "") == 0 && strcmp(TEMP_PASS, "") == 0) {
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(TEMP_WIFI);
      status = WiFi.begin(TEMP_WIFI, TEMP_PASS);
    }
  } else {
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to (DEFAULT) SSID: ");
      Serial.println(WIFI_SSID);
      status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }
  delay(5000);
  Serial.println("Connected to wifi");

  delay(500);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time configured.");

  delay(2000);
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