#include "coic.h"
#include <Wire.h>

coic::coic() {
  Wire.begin();
  //Wire.setClock(100000);
  latch_count = get_latch_count();
}

uint8_t coic::get_latch_count() {
  return get_data(CMD.LATCH_COUNT);
}

void coic::get_states() {
  Wire.beginTransmission(ADDR);
  Wire.write(CMD.OLDEST_STATE);
  Wire.endTransmission();

  Wire.requestFrom(ADDR, latch_count);
  uint8_t latch = 0;
  while(Wire.available()) {
    latch_state[latch] = Wire.read();
    latch++;
  }
  Wire.endTransmission();
}

bool coic::ready_to_send_states() {
  uint8_t result = get_data(CMD.CONVERSION_STATE);
  if(result == 2) return false;
  else return true;
}

void coic::start_conversion() {
  set_data(CMD.CONVERSION_STATE, 1);
}

void coic::stop_conversion() {
  set_data(CMD.CONVERSION_STATE, 0);
}

void coic::set_trigger(uint8_t latch, uint8_t trigger_id) {
  if(latch < latch_count && trigger_id < 4) {
    uint8_t trigger = get_data(CMD.FALLING_EDGE_TRIGGER + trigger_id);
    trigger |= (1<<latch);
    set_data(CMD.FALLING_EDGE_TRIGGER + trigger_id, trigger);
  }
}

void coic::set_all_triggers(uint8_t latch) {
  for(uint8_t trigger_id = 0; trigger_id < 4; trigger_id++) set_trigger(latch, trigger_id);
}

void coic::clear_trigger(uint8_t latch, uint8_t trigger_id) {
  if(latch < latch_count && trigger_id < 4) {
    uint8_t trigger = get_data(CMD.FALLING_EDGE_TRIGGER + trigger_id);
    trigger &= ~(1<<latch);
    set_data(CMD.FALLING_EDGE_TRIGGER + trigger_id, trigger);
  }
}

void coic::clear_all_triggers(uint8_t latch) {
  for(uint8_t trigger_id = 0; trigger_id < 4; trigger_id++) clear_trigger(latch, trigger_id);
}

void coic::clear_all_triggers() {
  Wire.beginTransmission(ADDR);
  for(uint8_t trigger_id = 0; trigger_id < 4; trigger_id++) {
    Wire.write(CMD.FALLING_EDGE_TRIGGER + trigger_id);
    Wire.write(0x00);
  }
  Wire.endTransmission();
}

uint8_t coic::get_data(uint8_t cmd) {
  Wire.beginTransmission(ADDR);
  Wire.write(cmd);
  Wire.endTransmission();

  Wire.requestFrom(ADDR, (uint8_t) 1);
  uint8_t result = 0;
  while(Wire.available()) {
    result = Wire.read();
  }
  Wire.endTransmission();
  return result;
}

void coic::set_data(uint8_t cmd, uint8_t date) {
  Wire.beginTransmission(ADDR);
  Wire.write(cmd);
  Wire.write(date);
  Wire.endTransmission();
}

void coic::testing() {
  uint8_t result = 0;

  Serial.println("State Queue Length:");
  Serial.println(get_data(CMD.STATE_QUEUE_LENGTH));
  Serial.println();

  Serial.println("Latch Count:");
  Serial.println(get_data(CMD.LATCH_COUNT));
  Serial.println();

  Serial.println("Conversion State:");
  Serial.println(get_data(CMD.CONVERSION_STATE));
  Serial.println();

  Serial.println("Start Conversion...");
  start_conversion();
  Serial.println("Conversion State:");
  Serial.println(get_data(CMD.CONVERSION_STATE));
  Serial.println();

  Serial.println("Stop Conversion...");
  stop_conversion();
  Serial.println("Conversion State:");
  Serial.println(get_data(CMD.CONVERSION_STATE));
  Serial.println();
  
  Serial.println("Get Rising Triggers:");
  Serial.println(get_data(CMD.RISING_EDGE_TRIGGER), BIN);
  Serial.println();
  
  Serial.println("Set Latch 0");
  set_trigger(0, 1);

  Serial.println("Get Rising Triggers:");
  Serial.println(get_data(CMD.RISING_EDGE_TRIGGER), BIN);
  Serial.println();
  
  Serial.println("Set Latch 1");
  set_trigger(1, 1);

  Serial.println("Get Rising Trigger:");
  Serial.println(get_data(CMD.RISING_EDGE_TRIGGER), BIN);
  Serial.println();

  Serial.println("Get Falling Trigger:");
  Serial.println(get_data(CMD.FALLING_EDGE_TRIGGER), BIN);
  Serial.println();

  Serial.println("Clear Triggers");
  clear_all_triggers();

  Serial.println("Get Rising Triggers:");
  Serial.println(get_data(CMD.RISING_EDGE_TRIGGER), BIN);
  Serial.println();
}
