
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "latches.h"
#include "pinfun.h"

volatile bool latches_interrupt = false;
uint8_t previous_state[LATCH_COUNT] = {0};

void latches_interrupt_reaction(bool move_queue) {
    latches_interrupt = false;
    
    // store current state of latches
    uint8_t initial_state[LATCH_COUNT];
    for (uint8_t latch = 0; latch < LATCH_COUNT; ++latch) {
        if (pin_read(latch_pins[latch]) == 0) initial_state[latch] = 0;
        else initial_state[latch] = 1;
    }
    
    // wait for bump to complete or max wait time is exeeded
    uint8_t final_state[LATCH_COUNT];
    uint8_t bump_wait = 0;
    while (bump_wait < 10) {
        _delay_ms(50);
        ++bump_wait;
        for (uint8_t latch = 0; latch < LATCH_COUNT; ++latch) {
            if (pin_read(latch_pins[latch]) == 0) final_state[latch] = 0;
            else final_state[latch] = 1;
            if (final_state[latch] != initial_state[latch]) bump_wait = 200;
        }
        if (move_queue) bump_wait = 200;
    }
    
    // evaluate result
    for (uint8_t latch = 0; latch < LATCH_COUNT; ++latch) {
        uint8_t w_pos = g_data[STATE_QUEUE_WRITE_POS];  // Just load this var once, as it consumes some time
        
        // write level to state_queue as this is allways done
        if (final_state[latch] == 0) state_queue[w_pos][latch] = STATE_LOW_LEVEL;
        else state_queue[w_pos][latch] = STATE_HIGH_LEVEL;
        
        // evaluate falling trigger
        if (previous_state[latch] > initial_state[latch]) {
            if ((g_data[FALLING_EDGE_TRIGGER] & 1<<latch) != 0) state_queue[w_pos][latch] = STATE_FALLING_EDGE;
            if (((g_data[FALLING_BUMP_TRIGGER] & 1<<latch) != 0) && (final_state[latch] != 0)) state_queue[w_pos][latch] = STATE_FALLING_BUMP;
        }
        
        // evaluate rising trigger
        else if ((previous_state[latch] < initial_state[latch])) {
            if ((g_data[RISING_EDGE_TRIGGER] & 1<<latch) != 0) state_queue[w_pos][latch] = STATE_RISING_EDGE;
            if (((g_data[RISING_BUMP_TRIGGER] & 1<<latch) != 0) && (final_state[latch] == 0)) state_queue[w_pos][latch] = STATE_RISING_BUMP;
        }
        
        if (state_queue[w_pos][latch] > STATE_HIGH_LEVEL) move_queue = true;
        
        // store state for next run, as it is not needed further
        previous_state[latch] = final_state[latch];
    }
    
    if (move_queue) state_queue_increase();
}

ISR (PORTA_PORT_vect) {
    for (uint8_t latch = 0; latch < LATCH_COUNT; ++latch) pin_int_disable(latch_pins[latch]);
    latches_interrupt = true;
}