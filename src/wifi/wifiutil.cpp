#include <WiFi.h>
#include "time.h"

#include "secret_wifi.h"

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int status = WL_IDLE_STATUS;

void connect_wifi(char wifi_ssid[], char wifi_password[]) {
  while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(wifi_ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(wifi_ssid, wifi_password);
      delay(5000);
  }
  Serial.println("Connected to wifi");

  delay(500);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time configured.");

  delay(2000);
}

void connect_wifi(const char * wifi_ssid, const char * wifi_password) {
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifi_ssid);

    status = WiFi.begin(wifi_ssid, wifi_password);
    delay(5000);
  }
  Serial.println("Connected to wifi");

  delay(500);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time configured.");

  delay(2000);
} 

void connect_wifi() {
  while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(WIFI_SSID);
      status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      delay(5000);
  }
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