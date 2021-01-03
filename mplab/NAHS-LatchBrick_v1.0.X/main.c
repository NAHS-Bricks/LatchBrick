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

int main() {
    // ----------------------------
    // Initialization
    // ----------------------------
    
    // Set F_CPU to 1MHz
    CPU_CCP = CCP_IOREG_gc;  // unlock clk configuration
    CLKCTRL.MCLKCTRLB = (0x03<<1) | 0x01;  // set prescaler to 16 and keep it enabled
    _delay_ms(10);  // wait a bit to let clock settle down
    
    i2c_init();
    
    // Enable Interrupts
    sei();
    
    //Release MainIC to let it Startup
    pin_set_input_hiz(MAINIC_RST_PIN);
    
    // ----------------------------
    // Normal Operation Loop
    // ----------------------------
    while(1) {
        // wait for i2c to finish completely. TODO: If space left this cloud be done a be more less timeconsuming by watching the state of the i2c conversion
        _delay_ms(10);
        // go to powerdown
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sei();
        sleep_cpu();
    }

    return(0);
}