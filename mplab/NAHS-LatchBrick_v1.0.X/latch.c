#include "latch.h"
#include "globals.h"
#include "pinfun.h"
#include <util/delay.h>
#include <avr/interrupt.h>

volatile bool latch_interrupt = false;
uint8_t latch_last_state[LATCH_COUNT] = {0};

void latch_interrupt_handler(bool default_increase_write_pos) {
    uint8_t latch_pre_bump[LATCH_COUNT] = {0};
    for(uint8_t i = 0; i < LATCH_COUNT; i++) {
        latch_pre_bump[i] = pin_read(latch_pins[i]);
    }

    // if all latches returned to their last state there is no need for further waiting
    // this is checked in the following block (or its delayed for 1s)
    for(uint8_t delay_count = 0; delay_count < 100; delay_count++) {
        _delay_ms(20);
        bool diff = false;
        for(uint8_t i = 0; i < LATCH_COUNT; i++) {
            if(pin_read(latch_pins[i]) != latch_last_state[i]) {
                diff = true;
                break;
            }
        }
        if (!diff) break;
    }

    uint8_t latch_post_bump[LATCH_COUNT] = {0};
    for(uint8_t i = 0; i < LATCH_COUNT; i++) {
        latch_post_bump[i] = pin_read(latch_pins[i]);
    }

    for(uint8_t latch = 0; latch < LATCH_COUNT; latch++) {
        if(latch_pre_bump[latch] != latch_post_bump[latch]) {
            // bump detected
            if(latch_last_state[latch] == 0 && (rising_bump_trigger & (1<<latch)) != 0) {
                // allowed rising bump detected
                state_queue[queue_write_pos][latch] = RISING_BUMP;
                continue;
            } else if(latch_last_state[latch] != 0 && (falling_bump_trigger & (1<<latch)) != 0) {
                // allowed falling bump detected
                state_queue[queue_write_pos][latch] = FALLING_BUMP;
                continue;
            }
        }
        if(latch_last_state[latch] != latch_pre_bump[latch]) {
            // edge detected
            if(latch_last_state[latch] == 0 && (rising_edge_trigger & (1<<latch)) != 0) {
                // allowed rising edge detected
                state_queue[queue_write_pos][latch] = RISING_EDGE;
                continue;
            } else if(latch_last_state[latch] != 0 && (falling_edge_trigger & (1<<latch)) != 0) {
                // allowed falling edge detected
                state_queue[queue_write_pos][latch] = FALLING_EDGE;
                continue;
            }
        }
        if(latch_post_bump[latch] == 0) {
            //low-level detected
            state_queue[queue_write_pos][latch] = LOW_LEVEL;
        } else {
            // high-level detected
            state_queue[queue_write_pos][latch] = HIGH_LEVEL;
        }
    }

    latch_interrupt = false;
    for(uint8_t latch = 0; latch < LATCH_COUNT; latch++) {
        latch_last_state[latch] = latch_post_bump[latch];
        if(state_queue[queue_write_pos][latch] > 1) default_increase_write_pos = true;
        pin_int_enable_bothedges(latch_pins[latch]);
    }

    if(default_increase_write_pos) {
        increase_queue_write_pos();
    }
}

ISR (PORTA_PORT_vect) {
    for(uint8_t i = 0; i < LATCH_COUNT; i++) {
        pin_int_disable(latch_pins[i]);
    }
    latch_interrupt = true;
}