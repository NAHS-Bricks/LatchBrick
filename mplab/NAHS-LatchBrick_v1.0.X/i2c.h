/* 
 * File:   i2c.h
 * Author: nijo
 *
 * Created on 23. Oktober 2020, 11:44
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

// I2C address space:
// 0100 XXXr is reserved for video encoders, GPIO expanders, should be applicable fro most cases
// 1010 XXXr memory, real-time clock

#define SLAVE_ADDRESS	8		// 8 bit address, i.e. bit 0 = 0, will be substituted by R/W bit
#define MAX_TRANSACTION 9		// maximum number of bytes sent/received in single transaction

void i2c_init(void);	// initialize I2C slave

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

