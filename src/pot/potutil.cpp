#include <Arduino.h>

//  ADC2 GPIO pins: 6, 7, 8, 10 through to 17  
// #define soilpin 15   // Capacitive Soil  Moisture Sensor v1.2
#define soilpin 32

#define nldrpin 12  // North LDR 
#define eldrpin 25  // East LDR
#define sldrpin 26  // South LDR
#define wldrpin 27  // West LDR

#define temppin 13  // LM35 Temp sensor

#define PIN_RED    23 // GIOP23
#define PIN_GREEN  22 // GIOP22
#define PIN_BLUE   21 // GIOP21

// Moisture Sensor
int soilmoisturevalue = 0;

// Light Level
//const int lightvaluehigh = 1621;
//const int lightvaluelow = 413;
int lightvalue = 0;

int r = 255;
int g = 255;
int b = 255;

int check_moisture() {
  int tick = 0;
  int MULTI_READ = 6;
  
  while (tick < MULTI_READ) {
    soilmoisturevalue = analogRead(soilpin);
    Serial.print("Soil Mosture Reading: ");
    Serial.println(soilmoisturevalue);
    tick++;

    delay(1000);
  }

  Serial.print("FINAL Soil Mosture Reading: ");
  Serial.println(soilmoisturevalue);

  return soilmoisturevalue;
}

int check_nlight() {
  int tick = 0;
  int MULTI_READ = 6;
  
  while (tick < MULTI_READ) {
    lightvalue = analogRead(nldrpin);
    Serial.print("North LDR Reading: ");
    Serial.println(lightvalue);
    tick++;

    delay(1000);
  }

  Serial.print("FINAL North LDR Reading: ");
  Serial.println(lightvalue);

  return lightvalue;
}

// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/AnalogOut/ledcWrite_RGB/ledcWrite_RGB.ino
void setup_light() {
  // pinMode(PIN_RED,   OUTPUT);
  // pinMode(PIN_GREEN, OUTPUT);
  // pinMode(PIN_BLUE,  OUTPUT);

  ledcAttachPin(PIN_RED,   1); // assign RGB led pins to channels
  ledcAttachPin(PIN_GREEN, 2);
  ledcAttachPin(PIN_BLUE,  3);

  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);
}

void turn_off_light() {
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
}

void turn_on_light(char *hexString) {
  sscanf(hexString, "%02x%02x%02x", &r, &g, &b);

  ledcWrite(1, r);
  ledcWrite(2, g);
  ledcWrite(3, b);
}