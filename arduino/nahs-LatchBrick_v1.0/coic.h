#ifndef COIC_H
#define COIC_H

#include <Arduino.h>

class coic {
  private:
    static const uint8_t ADDR = 16;
    static const uint8_t MAX_LATCHES = 8;
    const struct {
      uint8_t GET_LATCH_COUNT           =   0;
      uint8_t GET_STATES                =   1;
      uint8_t FINISHED                  =   2;
      uint8_t GET_FALLING_EDGE_TRIGGERS =   3;
      uint8_t GET_RISING_EDGE_TRIGGERS  =   4;
      uint8_t GET_FALLING_BUMP_TRIGGERS =   5;
      uint8_t GET_RISING_BUMP_TRIGGERS  =   6;
      uint8_t SET_FALLING_EDGE_TRIGGERS =   7;
      uint8_t SET_RISING_EDGE_TRIGGERS  =   8;
      uint8_t SET_FALLING_BUMP_TRIGGERS =   9;
      uint8_t SET_RISING_BUMP_TRIGGERS  =  10;
      uint8_t GET_QUEUE_LENGTH          =  11;
      uint8_t START_CONVERSION          =  12;
    } CMD;
  private:
    uint8_t get_data(uint8_t cmd);
    void set_data(uint8_t cmd, uint8_t date);

  public:
    uint8_t latch_count;
    uint8_t latch_state[MAX_LATCHES] = {0};
  public:
    coic();
    uint8_t get_latch_count();
    void get_states();
    bool ready_to_send_states();
    void start_conversion();
    void stop_conversion();
    void set_trigger(uint8_t latch, uint8_t trigger_id);
    void set_all_triggers(uint8_t latch);
    void clear_trigger(uint8_t latch, uint8_t trigger_id);
    void clear_all_triggers(uint8_t latch);
    void clear_all_triggers();
    void findAddr();
};

#endif // COIC_H 
