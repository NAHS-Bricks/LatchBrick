#include "global.h"
#include "runtimeData.h"
#include "configData.h"

runtimeData::runtimeData() {
}

void runtimeData::begin() {
  initialized = false;
  if (!RTCmem.isValid()) init();
}

void runtimeData::init() {
  // Copy data from configData or set initial values
  initialized = true;
  vars->brickTypeRequested = false;
  vars->featuresRequested = false;
  vars->versionRequested = false;
  vars->batVoltageRequested = false;
  vars->deepSleepDelay = DEFAULT_DEEP_SLEEP_DELAY;
  vars->adc5V = cfgdat.adc5V;
}

runtimeData rundat;