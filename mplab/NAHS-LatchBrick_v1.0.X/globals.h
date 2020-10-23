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
    
#define SDA_PIN PIN6_bp
#define SCL_PIN PIN6_bp
#define MAINIC_RST_PIN PIN6_bp
#define LATCH1_PIN PIN6_bp
#define LATCH2_PIN PIN6_bp

#define LATCH_COUNT 2
#define QUEUE_LENGTH 6
    
volatile uint8_t state_queue[QUEUE_LENGTH][LATCH_COUNT];
extern uint8_t queue_write_pos;
extern volatile uint8_t i2c_data;

void shift_queue();

#ifdef	__cplusplus
}
#endif

#endif	/* GLOBALS_H */

