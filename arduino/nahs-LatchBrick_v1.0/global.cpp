#include "global.h"
#include "configData.h"
#include "runtimeData.h"
#include "coic.h"

const String version_names[NUMBER_OF_VERSIONS] = {"os", "all", "latch", "bat", "sleep"};
const float version_numbers[NUMBER_OF_VERSIONS] = {1.0, 1.0, 1.0, 1.0, 1.0};
const String features[NUMBER_OF_FEATURES] = {"latch", "bat", "sleep"};
configData *cfgdat;
runtimeData *rundat;
coic *latches;

float readBatVoltage() {
    uint32_t adcReading = 0;
    for(uint8_t i = 0; i < 3; i++) {
      adcReading += analogRead(BAT_ADC_PIN);
      delay(20);
    }
  adcReading /= 3;
  return (((uint16_t) adcReading) * 5.0) / rundat->vars.adc5V;
}
