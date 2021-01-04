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
    
    
    #include <stdbool.h>

    #define SLAVE_ADDRESS	90		// 8 bit Slave Address, i.e. bit 0 = 0, will be substituted by R/W bit

    extern volatile bool i2c_interaction_running;

    void i2c_init(void);	// initialize I2C slave device

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

