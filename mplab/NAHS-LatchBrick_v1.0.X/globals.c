#include "globals.h"

uint8_t queue_write_pos = 0;

void shift_queue() {
    if(queue_write_pos == 0) return;
    for(uint8_t q_pos = 0; q_pos < queue_write_pos; q_pos++) {
        for(uint8_t latch = 0; latch < LATCH_COUNT; latch++) {
            state_queue[q_pos][latch] = state_queue[q_pos+1][latch];
        }
    }
    queue_write_pos--;
}