#include <WiFi.h>
#include "time.h"

//void connect_wifi(char wifi_ssid[], char wifi_password[]);
//void connect_wifi(const char * wifi_ssid, const char * wifi_password);

bool load_stored_wifi();
void keepWiFiAlive(void * parameter);
void connect_wifi();
void disconnect_wifi();

unsigned long get_time();