#ifndef RUNTIME_DATA_H
#define RUNTIME_DATA_H

#include <Arduino.h>
#include "global.h"

class runtimeData {
  public:
    struct {
      uint16_t memWritten1;
      uint16_t memWritten2;
      bool brickTypeRequested;
      bool featuresRequested;
      bool versionRequested;
      bool batVoltageRequested;
      uint16_t deepSleepDelay;
      uint16_t adc5V;  // same as in configData but this one can overwrite the one from configData
      
    } vars;
    bool initialized;

  public:
    runtimeData();
    void write();
  private:
    void init();
};

#endif // RUNTIME_DATA_H
