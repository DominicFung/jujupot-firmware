#include <Arduino.h>
#include <ArduinoJson.h>

StaticJsonDocument<500> shadow;
JsonObject state = shadow.createNestedObject("state");
JsonObject reported = state.createNestedObject("reported");

JsonObject sensors = reported.createNestedObject("sensors");
JsonObject controllables = reported.createNestedObject("controllables");


void init_shadow(const char product_id[37], const int fw_version) {
  reported["product-id"] = product_id;
  reported["local-fw-version"] = fw_version;
}

void add_device_id(const char device_id[60]) {
  reported["device-id"] = device_id;
}

void add_device_color(const char color[50]) {
  reported["device-color"] = color;
}

void add_signoff(const char verify_id[37]) {
  reported["verification-id"] = verify_id;
}

void add_sensor_value(const char key[50], const int high, const int low, const int samples, const int value) {
  JsonObject s = sensors.createNestedObject(key);
  s["config-high"] = high;
  s["config-low"] = low;
  s["config-samples"] = samples; // Maybe remove this .. we dont have enough space
  s["value"] = value;
}

void add_controlable_value(const char key[50], const int high, const int low, const int samples, const int value) {
  JsonObject c = controllables.createNestedObject(key);
  c["value"] = value;
}


// DO NOT CARRY OTHER DEVICE INFO HERE .. REASON: 512 BYTE LIMIT
// void add_pot_info(const char type_id[50], const char color[50], const char plant[50]) {
//   JsonObject s = reported.createNestedObject("pot");
//   s["type-id"] = type_id;
//   s["color"] = color;
//   s["plant"] = plant;
// }

std::string get_new_shadow(unsigned long time) {
  reported["comm-time"] = time;

  std::string output;
  serializeJson(shadow, output);
  return output;
}

void load_shadow(char json[]) { }