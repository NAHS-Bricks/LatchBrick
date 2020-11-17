/* 
 * File:   main.c
 * Author: nijo
 *
 * Created on 23. Oktober 2020, 11:43
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
#include "latch.h"
#include "globals.h"

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
        cli();
        if(latch_interrupt) {
            sei();
            _delay_ms(20);
            latch_interrupt_handler(false);
        }
        if(conversion_started) {
            sei();
            if(queue_write_pos == 0) {
                latch_interrupt_handler(true);
            }
            conversion_started = false;
            mainic_finished = false;
        }
        if(mainic_finished && queue_write_pos != 0) {
            sei();
            //Reset MainIC
            pin_set_output(MAINIC_RST_PIN);
            pin_set_output_low(MAINIC_RST_PIN);
            _delay_ms(10);
            pin_set_input_hiz(MAINIC_RST_PIN);
        }
        // go to powerdown
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sei();
        sleep_cpu();
    }

    return(0);
}