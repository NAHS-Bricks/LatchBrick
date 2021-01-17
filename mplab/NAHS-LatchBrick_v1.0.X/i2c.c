#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "pinfun.h"
#include "globals.h"
#include "i2c.h"

volatile bool i2c_interaction_running = false;
int8_t received_cmd = -1;  // Only to be used in interrupt
uint8_t next_latch_to_send = 0;  // Only to be used in interrupt

// Slave is organized as Byte Array (Register Array) with Read/Write access from Master (similar to EEPROM)
// maximum number of data bytes per transaction is limited to MAX_TRANSACTION.
// Limitation is necessary to ensure error recovery, otherwise the master might clock out
// an unlimited number of bytes when trying to recover from a stuck low SDA line since
// slave sends ACK after each recovery cycle and thus blocks the bus again.

// Master Register Write Cycle
// Start	Device Address, W			Slave ACK
//			Data Register Address		Slave ACK
//			Data Write					Slave ACK
//			...
//			Data Write					Slave ACK
// Stop

// Master Register Read Cycle
// Start	Device Address, W			Slave ACK
//			Data Register Address		Slave ACK
// Restart	Device Address, R			Slave ACK
//			Data Read					Master ACK
//			...
//			Data Read					Master NACK
// Stop

// if Master requests illegal address (address overflow), internal R/W register address will be reset zero
// the register address is incremented and limited to valid range after each data read or write operation
// e.g. valid address range 0 .. 7: read from address 13 will return data from address 0
// e.g. valid address range 0 .. 7: writing 4 consecutive bytes starting with address 6, address sequence will be 6 - 7 - 0 - 1

// slave device states:
// 0: idle, waiting for device address
// 1: slave device address received, waiting for register address
// 2: register address received
// 3: ongoing master data read operation
// 4: ongoing master data write operation

// when a Stop has been received, i2c_stop_flag will be set to signal data read/write transfer, may be reset in main program

// Initialize I2C interface
// on exit I2C is enabled and interrupts active

// initialize slave device
void i2c_init(void)	{
    pin_set_output(SCL_PIN);							// SCL = PA2
    pin_set_output_low(SCL_PIN);
    pin_pullup_disable(SCL_PIN);
    pin_inverted_disable(SCL_PIN);
    pin_int_disable(SCL_PIN);
    
	pin_set_output(SDA_PIN);							// SDA = PA1
	pin_set_output_low(SDA_PIN);
	pin_pullup_disable(SDA_PIN);
	pin_inverted_disable(SDA_PIN);
	pin_int_disable(SDA_PIN);
		
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

// error handler, should reset I2C slave internal state
// additional processing may be required, e.g. set flag in register array to enable master to read status
void i2c_error_handler() {
    received_cmd = -1;  // Reset received command if allready been set
    i2c_interaction_running = false;
	TWI0.SSTATUS |= (TWI_APIF_bm | TWI_DIF_bm);					// clear interrupt flags

	TWI0.SCTRLA = 0;											// disable slave
    
    i2c_init();
    
    TWI0.SCTRLB = TWI_ACKACT_NACK_gc | TWI_SCMD_NOACT_gc;		// set NACK, no action (just in case)
}

// I2C IRQ handler
ISR(TWI0_TWIS_vect)
{
    // APIF && DIF, invalid state (should never occur)
	if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_DIF_bm)) {
		i2c_error_handler();									// but happened during debugging
		return;
	}

    // Collision - slave has not been able to transmit a high data or NACK bit
	if (TWI0.SSTATUS & TWI_COLL_bm) {
		i2c_error_handler();	
		return;
	}

    // Bus Error - illegal bus condition
	if (TWI0.SSTATUS & TWI_BUSERR_bm) {
		i2c_error_handler();				
		return;
	}
    
    // APIF && !AP - Stop has been received
    if ((TWI0.SSTATUS & TWI_APIF_bm) && (!(TWI0.SSTATUS & TWI_AP_bm))) {
        i2c_interaction_running = false;
		TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
		return;
	}
    
    // APIF && AP - valid address has been received
    if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_AP_bm)) {
        i2c_interaction_running = true;
        TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;	// send ACK
        if (!(TWI0.SSTATUS & TWI_DIR_bm)) {  // Master wishes to write -- return and wait for payload
            return;
        }
    }
    
    // DIF - Data Interrupt Flag - slave byte transmit or receive completed
    if (TWI0.SSTATUS & TWI_DIF_bm) {
        if (TWI0.SSTATUS & TWI_DIR_bm) {						// Master wishes to read from slave
			if (TWI0.SSTATUS & TWI_RXACK_bm) {              	// Received NACK from master
                TWI0.SSTATUS |= (TWI_DIF_bm | TWI_APIF_bm);		// Reset module
				TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
                return;
            }
        }
    }

    // Master wishes to read from slave
    if (TWI0.SSTATUS & TWI_DIR_bm) {
        while (!(TWI0.SSTATUS & TWI_CLKHOLD_bm)) {}			// wait until Clock Hold flag set

        // No command received yet, send dummy-load
        if (received_cmd == -1) TWI0.SDATA = 0;

        // one-byte long data is requested
        if (received_cmd < CMD_OLDEST_STATE) {
            if (received_cmd == CMD_LATCH_COUNT) {
                TWI0.SDATA = LATCH_COUNT;
            }

            else if (received_cmd == CMD_CONVERSION_STATE) {
                TWI0.SDATA = g_data[received_cmd];
            }

            else {
                TWI0.SDATA = g_data[received_cmd];
            }
            received_cmd = -1;  // Reset received command -- transaction done
        }

        // oldest_state is requested
        else {
            TWI0.SDATA = state_queue[0][next_latch_to_send];
            next_latch_to_send++;
            if(next_latch_to_send >= LATCH_COUNT) {
                received_cmd = -1;
                next_latch_to_send = 0;
            }
        }
    }
    
    // Master wishes to write to slave
    else {
        // No command received yet, so this write should be a command
        if(received_cmd < 0) {
            received_cmd = TWI0.SDATA;
        }

        // Command allready received, so this should be a data-write
        else {
            const int8_t current_cmd = received_cmd;    // Save received_cmd for further use in this sub-tree...
            received_cmd = -1;                          // ...but be able to allready reset it
            const uint8_t current_data = TWI0.SDATA;

            // Command points to a valid storage location (and has valid data in case of CONVERSION_STATE)
            if ((current_cmd < CMD_CONVERSION_STATE) || ((current_cmd == CMD_CONVERSION_STATE) && (current_data < 2))) {
                g_data[current_cmd] = current_data;
            }
        }
    }
    
    TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;	// send ACK
}
