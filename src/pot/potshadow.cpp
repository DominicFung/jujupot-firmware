#include <Arduino.h>
#include <ArduinoJson.h>

StaticJsonDocument<500> shadow;
JsonObject state = shadow.createNestedObject("state");
JsonObject reported = state.createNestedObject("reported");


void init_shadow(const char product_id[37], const int fw_version) {
  reported["product-id"] = product_id;
  reported["local-fw-version"] = fw_version;
}

void add_signoff(const char verify_id[37]) {
  reported["verification-id"] = verify_id;
}

void add_pot_info(const char type_id[50], const char color[50], const char plant[50]) {
  JsonObject s = reported.createNestedObject("pot");
  s["type-id"] = type_id;
  s["color"] = color;
  s["plant"] = plant;
}

void add_sensor_value(const char key[50], const int high, const int low, const int samples, const int value) {
  JsonObject s = reported.createNestedObject(key);
  s["config-high"] = high;
  s["config-low"] = low;
  s["config-samples"] = samples;
  s["value"] = value;
}

std::string get_new_shadow(unsigned long time) {
  reported["comm-time"] = time;

  std::string output;
  serializeJson(shadow, output);
  return output;
}