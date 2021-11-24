#include <Arduino.h>

//  ADC2 GPIO pins: 6, 7, 8, 10 through to 17  
// #define soilpin 15   // Capacitive Soil  Moisture Sensor v1.2
#define soilpin 32

#define nldrpin 12  // North LDR 
#define eldrpin 25  // East LDR
#define sldrpin 26  // South LDR
#define wldrpin 27  // West LDR

#define temppin 13  // LM35 Temp sensor

// Moisture Sensor
int soilmoisturevalue = 0;

// Light Level
//const int lightvaluehigh = 1621;
//const int lightvaluelow = 413;
int lightvalue = 0;

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