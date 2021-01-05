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
    pin_set_output(MAINIC_RST_PIN);
    pin_set_output_low(MAINIC_RST_PIN);
    
    // Set F_CPU to 1MHz
    CPU_CCP = CCP_IOREG_gc;  // unlock clk configuration
    CLKCTRL.MCLKCTRLB = (0x03<<1) | 0x01;  // set prescaler to 16 and keep it enabled
    _delay_ms(10);  // wait a bit to let clock settle down
    
    // Setup Latch-Pins
    for(uint8_t i = 0; i < LATCH_COUNT; i++) {
        pin_set_input_pullup(latch_pins[i]);
        pin_int_enable_bothedges(latch_pins[i]);
    }
    
    i2c_init();
    
    // Enable Interrupts
    sei();
    
    //Release MainIC to let it Startup
    pin_set_input_hiz(MAINIC_RST_PIN);
    
    // ----------------------------
    // Normal Operation Loop
    // ----------------------------
    while(1) {
        if (latches_interrupt) {
            _delay_ms(20);
            latches_interrupt_reaction(false);
            for (uint8_t latch = 0; latch < LATCH_COUNT; ++latch) pin_int_enable_bothedges(latch_pins[latch]);
        }
        switch(g_data[CONVERSION_STATE]) {
            case CONVERSION_END:
                state_queue_decrease();
                g_data[CONVERSION_STATE] = CONVERSION_FINISHED;
                break;
            case CONVERSION_START:
                if (g_data[STATE_QUEUE_WRITE_POS] == 0) latches_interrupt_reaction(true);
                g_data[CONVERSION_STATE] = CONVERSION_COMPLETED;
                break;
            case CONVERSION_FINISHED:
                if (g_data[STATE_QUEUE_WRITE_POS] != 0) {
                    //Reset MainIC
                    pin_set_output(MAINIC_RST_PIN);
                    pin_set_output_low(MAINIC_RST_PIN);
                    _delay_ms(10);
                    pin_set_input_hiz(MAINIC_RST_PIN);
                    g_data[CONVERSION_STATE] = CONVERSION_RESET_TRIGGERED;
                }
                break;
        }
                
        // wait for i2c interaction to finish.
        bool go_sleep = true;
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