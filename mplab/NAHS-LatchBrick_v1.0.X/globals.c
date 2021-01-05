
#include "globals.h"

//static const uint8_t latch_pins[LATCH_COUNT] = {PIN6_bp, PIN7_bp};
volatile uint8_t state_queue[STATE_QUEUE_MAX_LENGTH][LATCH_COUNT] = {0};
volatile uint8_t g_data[6] = {0};

void state_queue_increase() {
    if(g_data[STATE_QUEUE_WRITE_POS] < STATE_QUEUE_MAX_LENGTH - 1) g_data[STATE_QUEUE_WRITE_POS]++;
}

void state_queue_decrease() {
    if(g_data[STATE_QUEUE_WRITE_POS] == 0) return;
    for(uint8_t q_pos = 0; q_pos < g_data[STATE_QUEUE_WRITE_POS]; ++q_pos) {
        for(uint8_t latch = 0; latch < LATCH_COUNT; ++latch) {
            state_queue[q_pos][latch] = state_queue[q_pos+1][latch];
        }
    }
    g_data[STATE_QUEUE_WRITE_POS]--;
}