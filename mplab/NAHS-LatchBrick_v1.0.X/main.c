/*
 * File:   main.c
 * Author: nijo
 *
 * Created on 2. Januar 2021, 21:16
 */


//HINT: F_CPU is defined in gcc-compiler symbols (xc8-compiler macro)

#include <avr/io.h>

FUSES = {
	.WDTCFG = 0x00, // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x00, // BODCFG {SLEEP=DIS, ACTIVE=DIS, SAMPFREQ=1KHz, LVL=BODLEVEL0}
	.OSCCFG = 0x01, // OSCCFG {FREQSEL=16MHZ, OSCLOCK=CLEAR}
	.TCD0CFG = 0x00, // TCD0CFG {CMPA=CLEAR, CMPB=CLEAR, CMPC=CLEAR, CMPD=CLEAR, CMPAEN=CLEAR, CMPBEN=CLEAR, CMPCEN=CLEAR, CMPDEN=CLEAR}
	.SYSCFG0 = 0xF6, // SYSCFG0 {EESAVE=CLEAR, RSTPINCFG=UPDI, CRCSRC=NOCRC}
	.SYSCFG1 = 0x07, // SYSCFG1 {SUT=64MS}
	.APPEND = 0x00, // APPEND
	.BOOTEND = 0x00, // BOOTEND
};

LOCKBITS = {
	0xC5, // LOCKBIT {LB=NOLOCK}
};


#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include "i2c.h"
#include "pinfun.h"
#include "globals.h"
#include "latches.h"

int main() {
    // ----------------------------
    // Initialization
    // ----------------------------
    
    // Pull MainIC to reset-state, till this CoIC is fully set-up
    pin_set_output(INTERRUPT_CTL_PIN);
    pin_set_output_low(INTERRUPT_CTL_PIN);
    
    // Set F_CPU to 1MHz
    CPU_CCP = CCP_IOREG_gc;  // unlock clk configuration
    CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm;  // set prescaler to 16 and enable it
    _delay_ms(1);  // wait a bit to let clock settle down
    
    // Setup Latch-Pins
    for(uint8_t i = 0; i < LATCH_COUNT; i++) {
        pin_set_input_pullup(latch_pins[i]);
        pin_int_enable_bothedges(latch_pins[i]);
    }
    
    i2c_init();
    
    // Enable Interrupts
    CPUINT.CTRLA &= ~CPUINT_LVL0RR_bm;  // Disable Interrupt Round-Robin
    CPUINT.LVL0PRI = 19;  // Setting Highest Interrupt Priority to TWI interrupts
    sei();
    
    // ----------------------------
    // Normal Operation Loop
    // ----------------------------
    while(1) {
        bool go_sleep = false;
        if (latches_interrupt) {
            _delay_ms(20);
            latches_interrupt_reaction(false);
        }
        else if (interrupt_ctl_changed) {
            uint8_t int_ctl = g_data[INTERRUPT_CTL];  // caching volatile variable
            if (int_ctl > INT_CTL_HIGH_SIG) {
                pin_set_output(INTERRUPT_CTL_PIN);
                if (int_ctl == INT_CTL_LOW_PULSE) pin_set_output_high(INTERRUPT_CTL_PIN);
                else pin_set_output_low(INTERRUPT_CTL_PIN);
            }
            else {
                pin_set_input_hiz(INTERRUPT_CTL_PIN);
            }
            interrupt_ctl_changed = false;
            if (g_data[CONVERSION_STATE] == CONVERSION_RESET_TRIGGERED) g_data[CONVERSION_STATE] = CONVERSION_FINISHED;
        }
        else if (g_data[CONVERSION_STATE] == CONVERSION_END) {
            state_queue_decrease();
            g_data[CONVERSION_STATE] = CONVERSION_FINISHED;
        }
        else if (g_data[CONVERSION_STATE] == CONVERSION_START) {
            if (g_data[STATE_QUEUE_WRITE_POS] == 0) latches_interrupt_reaction(true);
            g_data[CONVERSION_STATE] = CONVERSION_COMPLETED;
        }
        else if (g_data[CONVERSION_STATE] == CONVERSION_FINISHED && g_data[STATE_QUEUE_WRITE_POS] != 0) {
            uint8_t int_ctl = g_data[INTERRUPT_CTL];  // caching volatile variable
            if (int_ctl != INT_CTL_DISABLE) {
                if (int_ctl > INT_CTL_HIGH_SIG) pin_set_output_toggle(INTERRUPT_CTL_PIN);
                else {
                    pin_set_output(INTERRUPT_CTL_PIN);
                    if (int_ctl == INT_CTL_LOW_SIG) pin_set_output_low(INTERRUPT_CTL_PIN);
                    else pin_set_output_high(INTERRUPT_CTL_PIN);
                }
                _delay_ms(10);
                if (int_ctl > INT_CTL_HIGH_SIG) pin_set_output_toggle(INTERRUPT_CTL_PIN);
                else {
                    pin_set_input_hiz(INTERRUPT_CTL_PIN);
                } 
            }
            g_data[CONVERSION_STATE] = CONVERSION_RESET_TRIGGERED;
        }
        else go_sleep = true;
                
        // wait for i2c interaction to finish.
        while(i2c_interaction_running) {
            go_sleep = false;
            _delay_ms(1);
        }
        
        if(go_sleep  && !latches_interrupt) {
            // go to powerdown
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);
            sleep_enable();
            sei();
            sleep_cpu();
        }
    }

    return(0);
}