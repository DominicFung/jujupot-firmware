#include <Arduino.h>
#include <ArduinoJson.h>

extern StaticJsonDocument<200> shadow;

void init_shadow(const char product_id[37], const int fw_version);
void load_shadow(char json[]);

void add_signoff(const char verify_id[37]);
//void add_pot_info(const char type_id[50], const char color[50], const char plant[50]);
void add_device_id(const char device_id[60]); //45 for jujupot-*
void add_device_color(const char color[50]);

void add_sensor_value(const char key[50], const int high, const int low, const int samples, const int value);
std::string get_new_shadow(unsigned long time); // adds comm-date