/* 
 * File:   globals.h
 * Author: nijo
 *
 * Created on 23. Oktober 2020, 11:56
 */

#ifndef GLOBALS_H
#define	GLOBALS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <avr/io.h>
    
#define SDA_PIN PIN1_bp
#define SCL_PIN PIN2_bp
#define MAINIC_RST_PIN PIN3_bp

#define LATCH_COUNT 2
#define QUEUE_LENGTH 6
    
#define LOW_LEVEL 0
#define HIGH_LEVEL 1
#define FALLING_EDGE 2
#define RISING_EDGE 3
#define FALLING_BUMP 4
#define RISING_BUMP 5
    
extern uint8_t latch_pins[LATCH_COUNT];
extern volatile uint8_t state_queue[QUEUE_LENGTH][LATCH_COUNT];
extern uint8_t queue_write_pos;
extern volatile uint8_t i2c_data;
extern volatile uint8_t falling_edge_trigger, rising_edge_trigger, falling_bump_trigger, rising_bump_trigger;

#define increase_queue_write_pos() if(queue_write_pos < QUEUE_LENGTH-1) queue_write_pos++

void shift_queue();

#ifdef	__cplusplus
}
#endif

#endif	/* GLOBALS_H */

