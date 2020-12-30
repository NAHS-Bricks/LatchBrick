#include "coic.h"
#include <Wire.h>

coic::coic() {
  Wire.begin();
  Wire.setClock(100000);
  latch_count = get_latch_count();
}

uint8_t coic::get_latch_count() {
  return get_data(CMD.GET_LATCH_COUNT);
}

void coic::get_states() {
  Wire.beginTransmission(ADDR);
  Wire.write(CMD.GET_STATES);
  Wire.endTransmission();

  Wire.requestFrom(ADDR, latch_count);
  uint8_t latch = 0;
  while(Wire.available()) {
    latch_state[latch] = Wire.read();
    latch++;
  }
}

bool coic::ready_to_send_states() {
  uint8_t result = get_data(CMD.GET_QUEUE_LENGTH);
  if(result == 0) return false;
  else return true;
}

void coic::start_conversion() {
  Wire.beginTransmission(ADDR);
  Wire.write(CMD.START_CONVERSION);
  Wire.endTransmission();
}

void coic::stop_conversion() {
  Wire.beginTransmission(ADDR);
  Wire.write(CMD.FINISHED);
  Wire.endTransmission();
}

void coic::set_trigger(uint8_t latch, uint8_t trigger_id) {
  if(latch < latch_count && trigger_id < 4) {
    uint8_t trigger = get_data(CMD.GET_FALLING_EDGE_TRIGGERS + trigger_id);
    trigger |= (1<<latch);
    set_data(CMD.SET_FALLING_EDGE_TRIGGERS + trigger_id, trigger);
  }
}

void coic::set_all_triggers(uint8_t latch) {
  for(uint8_t trigger_id = 0; trigger_id < 4; trigger_id++) set_trigger(latch, trigger_id);
}

void coic::clear_trigger(uint8_t latch, uint8_t trigger_id) {
  if(latch < latch_count && trigger_id < 4) {
    uint8_t trigger = get_data(CMD.GET_FALLING_EDGE_TRIGGERS + trigger_id);
    trigger &= ~(1<<latch);
    set_data(CMD.SET_FALLING_EDGE_TRIGGERS + trigger_id, trigger);
  }
}

void coic::clear_all_triggers(uint8_t latch) {
  for(uint8_t trigger_id = 0; trigger_id < 4; trigger_id++) clear_trigger(latch, trigger_id);
}

void coic::clear_all_triggers() {
  for(uint8_t trigger_id = 0; trigger_id < 4; trigger_id++) set_data(CMD.SET_FALLING_EDGE_TRIGGERS + trigger_id, 0x00);
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
  return result;
}

void coic::set_data(uint8_t cmd, uint8_t date) {
  Wire.beginTransmission(ADDR);
  Wire.write(cmd);
  Wire.endTransmission();

  Wire.beginTransmission(ADDR);
  Wire.write(date);
  Wire.endTransmission();
}

void coic::findAddr() {
  for(uint8_t i = 44; i < 92; ++i) {
    Serial.print("start ");
    Serial.print(i);
    Serial.print(" ");

    Wire.requestFrom(i, (uint8_t) 1);
    uint8_t result = 0;
    result = Wire.read();
    Serial.print(result);
    Serial.println(" end");
  }
}
