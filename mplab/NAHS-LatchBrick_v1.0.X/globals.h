/* 
 * File:   globals.h
 * Author: nijo
 *
 * Created on 2. Januar 2021, 21:15
 */

#ifndef GLOBALS_H
#define	GLOBALS_H

#ifdef	__cplusplus
extern "C" {
#endif

    
    #include <avr/io.h>

    #define SDA_PIN                     PIN1_bp
    #define SCL_PIN                     PIN2_bp
    #define MAINIC_RST_PIN              PIN3_bp
    const static uint8_t latch_pins[LATCH_COUNT];

    #define LATCH_COUNT                 2
    #define STATE_QUEUE_MAX_LENGTH      6

    #define STATE_LOW_LEVEL             0
    #define STATE_HIGH_LEVEL            1
    #define STATE_FALLING_EDGE          2
    #define STATE_RISING_EDGE           3
    #define STATE_FALLING_BUMP          4
    #define STATE_RISING_BUMP           5

    #define CMD_FALLING_EDGE_TRIGGER    0
    #define CMD_RISING_EDGE_TRIGGER     1
    #define CMD_FALLING_BUMP_TRIGGER    2
    #define CMD_RISING_BUMP_TRIGGER     3
    #define CMD_CONVERSION_STATE        4
    #define CMD_STATE_QUEUE_LENGTH      5
    #define CMD_LATCH_COUNT             6
    #define CMD_OLDEST_STATE            7

    #define CONVERSION_ENDED            0
    #define CONVERSION_STARTED          1
    #define CONVERSION_COMPLETED        2

    extern volatile uint8_t state_queue[STATE_QUEUE_MAX_LENGTH][LATCH_COUNT];
    extern volatile uint8_t g_data[6];  // Contains the following data:
    #define FALLING_EDGE_TRIGGER        0
    #define RISING_EDGE_TRIGGER         1
    #define FALLING_BUMP_TRIGGER        2
    #define RISING_BUMP_TRIGGER         3
    #define CONVERSION_STATE            4
    #define STATE_QUEUE_WRITE_POS       5

    void state_queue_increase();
    void state_queue_decrease();


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBALS_H */

