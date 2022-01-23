#include <Arduino.h>
#include <ArduinoJson.h>

StaticJsonDocument<512> shadow;

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

bool load_shadow(char json[]) {
  auto error = deserializeJson(shadow, json);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  } else { return true; }
}

bool load_init_sensor(char json[]) {
  StaticJsonDocument<512> doc;
  auto error = deserializeJson(doc, json);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  } else { 
    JsonObject c = doc.as<JsonObject>();
    sensors.set(c);
    return true;
  }
}

bool load_init_controlable(char json[]) {
  StaticJsonDocument<200> doc;
  Serial.println(json);
  auto error = deserializeJson(doc, json);
  Serial.println(json);

  const char* world = doc["hello"];
  Serial.println(world);
  
  std::string testoutput;
  serializeJson(doc, testoutput);
  for(int i=0; i<testoutput.length(); i++) {
    Serial.print(testoutput.at(i));
  }
  Serial.println();

  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return false;
  } else { 
    JsonObject c = doc.as<JsonObject>();
    bool isCSet = controllables.set(c);
    if (!isCSet) { Serial.println("Unable to set Controllable."); }
    return isCSet;
  }
}

/**
 * @brief Get the controllable object
 * @return JsonObject 
 * 
 * {"sensors":{"lsv1-3":{"h":20,"l":0,"v":1}},"controllables":{"ledv1-1":{"v":"0000FF"}},"product-id":"dabae003-b3c6-491a-82e3-47b56e16eecd","local-fw-version":1,"device-id":"hommieo_9e2c0d3b-88bd-46fc-a18d-5291a3737e83","verification-id":"e99e3303-dcc5-4071-9569-94068bac24cc","comm-time":1642951071}
 */

JsonObject get_controllable() {
  if (shadow.containsKey("controllables")) {
    return shadow["controllables"].as<JsonObject>();
  } else { 
    Serial.println("GETCONTROLLABLE ERROR: No controllable in reported");
  }
}