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

        Serial.println("[WIFI] Connecting");
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
          Serial.println("Wifi begin -- passed ..");
        } else {
          Serial.println("First Char is not null ?? ");

          if (strcmp(TEMP_WIFI, "") != 0 && strcmp(TEMP_PASS, "") != 0) {
            Serial.println("Made it this far, how come .. ");
            WiFi.begin(TEMP_WIFI, TEMP_PASS);
          } else {
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          }
        }
        
        unsigned long startAttemptTime = millis();
        Serial.println("After milli call ..");

        // Keep looping while we're not connected and haven't reached the timeout
        while (WiFi.status() != WL_CONNECTED && 
                millis() - startAttemptTime < WIFI_TIMEOUT_MS){
                  delay(500);
                }

        Serial.println("After while unconnected loop call ..");

        // When we couldn't make a WiFi connection (or the timeout expired)
		    // sleep for a while and then retry.
        if(WiFi.status() != WL_CONNECTED){
          Serial.println("[WIFI] FAILED");
          vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
			    continue;
        }

        Serial.println("[WIFI] Connected: " + WiFi.localIP());
    }
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