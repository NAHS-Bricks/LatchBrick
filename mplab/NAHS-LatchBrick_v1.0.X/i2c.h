/* 
 * File:   i2c.h
 * Author: nijo
 *
 * Created on 2. Januar 2021, 21:16
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

#define SLAVE_ADDRESS	90		// 8 bit Slave Address, i.e. bit 0 = 0, will be substituted by R/W bit
#define SLAVE_REG_SIZE	8		// byte register array size

#define MAX_TRANSACTION 9		// maximum number of bytes sent/received in single transaction
								// not used for normal transactions, only used for error recovery

extern volatile uint8_t	i2c_reg_array[SLAVE_REG_SIZE];	// register array
extern volatile uint8_t	i2c_stop_flag;				// 1: register address write performed,
													// 2: master read performed, 3: master write performed,
													// 0xff: stop received in invalid state
extern uint8_t i2c_reg_start;						// first register address of current transaction
extern uint8_t i2c_bytes;							// number of bytes sent/received during transaction

void		i2c_init(void);	// initialize I2C slave device

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

