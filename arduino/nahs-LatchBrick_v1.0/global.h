#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

class configData;
class runtimeData;

#define SETUP_PIN D5
#define BAT_CHRG_PIN D6
#define BAT_STDBY_PIN D7
#define BAT_ADC_PIN A0
#define SDA_PIN D2
#define SCL_PIN D1

#define DEFAULT_DEEP_SLEEP_DELAY 60
#define NUMBER_OF_FEATURES 3

#define BRICK_TYPE 2
#define NUMBER_OF_VERSIONS 5

#define OS_YEAR 2020

extern const String version_names[NUMBER_OF_VERSIONS];
extern const float version_numbers[NUMBER_OF_VERSIONS];
extern const String features[NUMBER_OF_FEATURES];
extern configData *cfgdat;
extern runtimeData *rundat;

float readBatVoltage();

#endif // GLOBAL_H
