#include <Arduino.h>

extern const int airvalue = 4095;
extern const int watervalue = 2023;
int check_moisture();

extern const int lightvaluehigh = 1621;
extern const int lightvaluelow = 413;
int check_nlight();