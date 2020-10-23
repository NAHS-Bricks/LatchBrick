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
#include "i2c.h"
#include "pinfun.h"
#include "globals.h"
/*
 * 
 */
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
    pin_set_input_pullup(LATCH1_PIN);
    pin_set_input_pullup(LATCH2_PIN);
    pin_int_enable_bothedges(LATCH1_PIN);
    pin_int_enable_bothedges(LATCH2_PIN);
    // TODO: interrupts
    
    i2c_init();
    
    //Release MainIC to let ist Startup
    pin_set_input_hiz(MAINIC_RST_PIN);
    
    // ----------------------------
    // Normal Operation Loop
    // ----------------------------
    while(1) {
        _delay_ms(100);
    }

    return(0);
}

