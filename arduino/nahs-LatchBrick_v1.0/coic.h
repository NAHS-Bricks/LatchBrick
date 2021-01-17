#ifndef COIC_H
#define COIC_H

#include <Arduino.h>

class coic {
  private:
    static const uint8_t ADDR = 45;
    static const uint8_t MAX_LATCHES = 8;
    const struct {
      uint8_t FALLING_EDGE_TRIGGER  =   0;
      uint8_t RISING_EDGE_TRIGGER   =   1;
      uint8_t FALLING_BUMP_TRIGGER  =   2;
      uint8_t RISING_BUMP_TRIGGER   =   3;
      uint8_t CONVERSION_STATE      =   4;
      uint8_t STATE_QUEUE_LENGTH    =   5;
      uint8_t LATCH_COUNT           =   6;
      uint8_t OLDEST_STATE          =   7;
    } CMD;
  private:
    uint8_t get_data(uint8_t cmd);
    void set_data(uint8_t cmd, uint8_t date);

  public:
    uint8_t latch_count;
    uint8_t latch_state[MAX_LATCHES] = {0};
    uint8_t conversion_state();
  public:
    coic();
    uint8_t get_latch_count();
    void get_states();
    bool ready_to_send_states();
    void start_conversion();
    void stop_conversion();
    uint8_t queue_length();
    void set_trigger(uint8_t latch, uint8_t trigger_id);
    void set_all_triggers(uint8_t latch);
    void clear_trigger(uint8_t latch, uint8_t trigger_id);
    void clear_all_triggers(uint8_t latch);
    void clear_all_triggers();
    void testing();
};

#endif // COIC_H 
