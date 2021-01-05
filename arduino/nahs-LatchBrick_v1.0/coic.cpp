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
  uint8_t queue_length = get_data(CMD.STATE_QUEUE_LENGTH);
  Serial.print("State Queue Length: ");
  Serial.println(queue_length);
  Serial.println();

  if (queue_length > 0) {
    Serial.println("Getting rid of data...");
    Serial.println("Start conversion...");
    start_conversion();
    Serial.print("Waiting for readyness");
    while(!ready_to_send_states()) {
      delay(100);
      Serial.print(".");
    }
    Serial.println("received");
    Serial.println("Fetching states...");
    get_states();
    Serial.println("Received:");
    for (uint8_t latch = 0; latch < latch_count; ++latch) Serial.print(latch_state[latch]);
    Serial.println("Stop conversion...");
    stop_conversion();
    return;
  }

  Serial.println("Start conversion...");
  start_conversion();
  Serial.println();
  
  Serial.print("Waiting for readyness");
  while(!ready_to_send_states()) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("received");
  Serial.println();

  queue_length = get_data(CMD.STATE_QUEUE_LENGTH);
  Serial.print("State Queue Length: ");
  Serial.println(queue_length);
  Serial.println();

  Serial.println("Setting Up Triggers...");
  set_all_triggers(0);
  set_all_triggers(1);

  Serial.print("Waiting for User to trigger latch");
  while(true) {
    delay(1000);
    Serial.print(".");
    if(get_data(CMD.STATE_QUEUE_LENGTH) > queue_length) break;
  }
  Serial.println("received");
  Serial.println();

  queue_length = get_data(CMD.STATE_QUEUE_LENGTH);
  Serial.print("State Queue Length: ");
  Serial.println(queue_length);
  Serial.println();

  Serial.println("Fetching states...");
  get_states();
  Serial.println("Received:");
  for (uint8_t latch = 0; latch < latch_count; ++latch) Serial.println(latch_state[latch]);
  Serial.println();
  
  Serial.println("Stop conversion...");
  stop_conversion();
}
