#include "global.h"
#include "runtimeData.h"
#include "configData.h"

runtimeData::runtimeData() {
  // Read data from rctMem
  initialized = false;
  ESP.rtcUserMemoryRead(0, (uint32_t*) &vars, sizeof(vars));
  if (vars.memWritten1 != vars.memWritten2) init();
}

void runtimeData::init() {
  // Copy data from configData or set initial values
  initialized = true;
  vars.brickTypeRequested = false;
  vars.featuresRequested = false;
  vars.versionRequested = false;
  vars.batVoltageRequested = false;
  vars.deepSleepDelay = DEFAULT_DEEP_SLEEP_DELAY;
  vars.adc5V = cfgdat->adc5V;
}

void runtimeData::write() {
  vars.memWritten1 = vars.memWritten2;
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &vars, sizeof(vars));
}
