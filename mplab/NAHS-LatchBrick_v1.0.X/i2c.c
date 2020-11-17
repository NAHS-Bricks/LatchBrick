#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "pinfun.h"
#include "globals.h"

volatile uint8_t i2c_data;				// data byte sent by slave or received from master
												// defined in main program
//uint8_t timeout_cnt;							// 1ms timeout tick counter
bool cmd_received = false;
uint8_t cmd = 0;
uint8_t next_latch_to_send = 0;
volatile bool mainic_finished = false;
volatile bool conversion_started = false;


// timeout is handled by 1 millisecond RTC PIT interrupt as follows:

/*
extern uint8_t timeout_cnt;

ISR(RTC_PIT_vect) {						// PIT interrupt handling code, 1 ms interrupt
	timeout_cnt++;;						// increment timeout counter
	RTC.PITINTFLAGS = RTC_PI_bm;		// clear interrupt flag
}
*/

// Slave Status Register
//
// DIF APIF CLKHOLD RXACK COLL BUSERR DIR AP
//
// DIF		Data Interrupt Flag		slave byte transmit or receive operation successfully completed
// APIF		Addr./Stop Int. Flag	valid address has been received or stop condition
// CLKHOLD	Clock Hold				slave is currently stretching the TWI clock period
// RXACK	Received Acknowledge	the most recent acknowledge bit from the master. ‘0’ ACK, ‘1’ NACK
// COLL		Collision				slave has not been able to transmit a high data or NACK bit
// BUSERR	Bus Error				illegal bus condition
// DIR		Read/Write Direction	direction bit from last address packet received from master, ‘1’ master read
// AP		Address or Stop			when APIF is set: 0 STOP, 1 ADR

// Initialize I2C interface
// on exit I2C is enabled and interrupts active

void i2c_init(void)											// initialize slave
{
    pin_set_output(SCL_PIN);
    // TODO: disable interrupt on SCL_PIN
    pin_set_output(SDA_PIN);
    // TODO: disable interrupt on SDA_PIN
		
	TWI0.CTRLA = 0 << TWI_FMPEN_bp								// FM Plus Enable: disabled
	             | TWI_SDAHOLD_50NS_gc							// Typical 50ns hold time
	             | TWI_SDASETUP_8CYC_gc;						// SDA setup time is 8 clock cycles
		// TWI0.DBGCTRL = 0 << TWI_DBGRUN_bp; Debug Run: disabled

	TWI0.SADDR = SLAVE_ADDRESS									// Slave Address (8 bit address, i. e. bit 0 = 0, will be substituted by R/W bit)
	             | 0 << TWI_ADDREN_bp;							// General Call Recognition Enable: disabled
				 
		// TWI0.SADDRMASK = 0 << TWI_ADDREN_bp; Address Mask Enable: disabled
		//	| 0x0 << TWI_ADDRMASK_gp; Address Mask: 0x0

	TWI0.SCTRLA = 1 << TWI_APIEN_bp								// Address/Stop Interrupt Enable: enabled
	              | 1 << TWI_DIEN_bp							// Data Interrupt Enable: enabled
	              | 1 << TWI_ENABLE_bp							// Enable TWI Slave: enabled
	              | 1 << TWI_PIEN_bp							// Stop Interrupt Enable: enabled
	              | 0 << TWI_PMEN_bp							// Promiscuous Mode Enable: disabled
	              | 0 << TWI_SMEN_bp;							// Smart Mode Enable: disabled
}

void i2c_error_handler() {  									// error handler, should reset I2C slave internal state	
	TWI0.SSTATUS |= (TWI_APIF_bm | TWI_DIF_bm);					// clear interrupt flags
    pin_set_input_hiz(SCL_PIN);
    pin_set_input_hiz(SDA_PIN);
	TWI0.SCTRLA = 0;											// disable slave
	_delay_us(10);												// SCL, SDA tristated high
    pin_set_output(SCL_PIN);
    pin_set_output(SDA_PIN);
    
																// re-enable slave
	TWI0.SCTRLA = 1 << TWI_APIEN_bp								// Address/Stop Interrupt Enable: enabled
				| 1 << TWI_DIEN_bp								// Data Interrupt Enable: enabled
				| 1 << TWI_ENABLE_bp							// Enable TWI Slave: enabled
				| 1 << TWI_PIEN_bp								// Stop Interrupt Enable: enabled
				| 0 << TWI_PMEN_bp								// Promiscuous Mode Enable: disabled
				| 0 << TWI_SMEN_bp;								// Smart Mode Enable: disabled
	TWI0.SCTRLB = TWI_ACKACT_NACK_gc | TWI_SCMD_NOACT_gc;		// set NACK, no action (just in case)
}

// I2C IRQ handler
// WARNING - do NOT clear interrupt flags during transaction, this will lead to false data when master reads

ISR(TWI0_TWIS_vect)
{
	if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_DIF_bm)) { // APIF && DIF, invalid state (should never occur
		i2c_error_handler();									// but happened during debugging
		return;
	}

	if (TWI0.SSTATUS & TWI_COLL_bm) {							// Collision - slave has not been able to transmit a high data or NACK bit
		i2c_error_handler();	
		return;
	}

	if (TWI0.SSTATUS & TWI_BUSERR_bm) {							// Bus Error - illegal bus condition
		i2c_error_handler();				
		return;
	}

	if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_AP_bm)) { // APIF && AP - valid address has been received
		TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;	// send ACK 

		if (TWI0.SSTATUS & TWI_DIR_bm) {						// Master wishes to read from slave (else return and wait for data from master)
//			timeout_cnt = 0;									// reset timeout counter, will be incremented by ms tick interrupt
			while (!(TWI0.SSTATUS & TWI_CLKHOLD_bm)) {			// wait until Clock Hold flag set
//				if (timeout_cnt > 2) return;					// return if timeout error
			}
            if(cmd_received) {
                if(cmd == 1) {
                    TWI0.SDATA = state_queue[0][next_latch_to_send];
                    next_latch_to_send++;
                    if(next_latch_to_send == LATCH_COUNT) cmd_received = false;
                } else {
                    TWI0.SDATA = i2c_data;								// Master read operation
                    cmd_received = false;
                }
            } else TWI0.SDATA = 0;
			TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;										
		}
		return;
	}
	if (TWI0.SSTATUS & TWI_DIF_bm) {							// DIF - Data Interrupt Flag - slave byte transmit or receive completed

		if (TWI0.SSTATUS & TWI_DIR_bm) {						// Master wishes to read from slave
			if (!(TWI0.SSTATUS & TWI_RXACK_bm)) {				// Received ACK from master
//				timeout_cnt = 0;								// reset timeout counter, will be incremented by ms tick interrupt
				while (!(TWI0.SSTATUS & TWI_CLKHOLD_bm)) {		// wait until Clock Hold flag set
//					if (timeout_cnt > 2) return;				// return if timeout error
				}
				TWI0.SDATA = i2c_data;							// Master read operation
				TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;							
			} else {											// Received NACK from master
				TWI0.SSTATUS |= (TWI_DIF_bm | TWI_APIF_bm);		// Reset module
				TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;				
			}
			
		} else {												// Master wishes to write to slave
            if(!cmd_received) {
                cmd_received = true;
                cmd = TWI0.SDATA;
                switch(cmd) {  // prepare outgoing data
                    case 0:
                        i2c_data = LATCH_COUNT;
                        break;
                    case 1:
                        next_latch_to_send = 0;
                        break;
                    case 2:
                        mainic_finished = true;
                        cmd_received = false;
                        shift_queue();
                        break;
                    case 3:
                        i2c_data = falling_edge_trigger;
                        break;
                    case 4:
                        i2c_data = rising_edge_trigger;
                        break;
                    case 5:
                        i2c_data = falling_bump_trigger;
                        break;
                    case 6:
                        i2c_data = rising_bump_trigger;
                        break;
                    case 11:
                        i2c_data = queue_write_pos;
                        break;
                    case 12:
                        conversion_started = true;
                        cmd_received = false;
                        break;
                }
                TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc; // send ACK
            } else {
                switch(cmd) {
                    case 7:
                        falling_edge_trigger = TWI0.SDATA;
                        break;
                    case 8:
                        rising_edge_trigger = TWI0.SDATA;
                        break;
                    case 9:
                        falling_bump_trigger = TWI0.SDATA;
                        break;
                    case 10:
                        rising_bump_trigger = TWI0.SDATA;
                        break;
                    default:
                        TWI0.SCTRLB = TWI_ACKACT_NACK_gc | TWI_SCMD_RESPONSE_gc; // send NACK
                        cmd_received = false;
                        return;
                }
                cmd_received = false;
                TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc; // send ACK
            }
        }
        return;
	}
	
	if ((TWI0.SSTATUS & TWI_APIF_bm) && (!(TWI0.SSTATUS & TWI_AP_bm))) { // APIF && !AP - Stop has been received
		TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
		return;
	}
}

