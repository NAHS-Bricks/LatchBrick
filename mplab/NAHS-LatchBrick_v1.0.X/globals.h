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
    #include <stdbool.h>

    #define LATCH_COUNT                 2
    #define STATE_QUEUE_MAX_LENGTH      6
    
    #define SDA_PIN                     PIN1_bp
    #define SCL_PIN                     PIN2_bp
    #define INTERRUPT_CTL_PIN           PIN3_bp
    static const uint8_t latch_pins[LATCH_COUNT] = {PIN6_bp, PIN7_bp};

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
    #define CMD_INTERRUPT_CTL           5
    #define CMD_STATE_QUEUE_LENGTH      6
    #define CMD_LATCH_COUNT             7
    #define CMD_OLDEST_STATE            8

    #define CONVERSION_END              0
    #define CONVERSION_START            1
    #define CONVERSION_COMPLETED        2
    #define CONVERSION_FINISHED         3
    #define CONVERSION_RESET_TRIGGERED  4

    #define INT_CTL_DISABLE             0
    #define INT_CTL_LOW_SIG             1
    #define INT_CTL_HIGH_SIG            2
    #define INT_CTL_LOW_PULSE           3
    #define INT_CTL_HIGH_PULSE          4

    extern volatile uint8_t state_queue[STATE_QUEUE_MAX_LENGTH][LATCH_COUNT];
    extern volatile uint8_t g_data[7];  // Contains the following data:
    #define FALLING_EDGE_TRIGGER        0
    #define RISING_EDGE_TRIGGER         1
    #define FALLING_BUMP_TRIGGER        2
    #define RISING_BUMP_TRIGGER         3
    #define CONVERSION_STATE            4
    #define INTERRUPT_CTL               5
    #define STATE_QUEUE_WRITE_POS       6

    extern volatile bool interrupt_ctl_changed;

    void state_queue_increase();
    void state_queue_decrease();


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBALS_H */

