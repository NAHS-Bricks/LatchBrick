#ifndef RUNTIME_DATA_H
#define RUNTIME_DATA_H

#include <Arduino.h>
#include "global.h"
#include <nahs-Brick-Lib-RTCmem.h>

class runtimeData {
  private:
    typedef struct {
      bool brickTypeRequested;
      bool featuresRequested;
      bool versionRequested;
      bool batVoltageRequested;
      uint16_t deepSleepDelay;
      uint16_t adc5V;  // same as in configData but this one can overwrite the one from configData
      
    } _vars;
  public:
    _vars* vars = RTCmem.registerData<_vars>();
    bool initialized;

  public:
    runtimeData();
    void begin();

  private:
    void init();
};

extern runtimeData rundat;

#endif // RUNTIME_DATA_H
