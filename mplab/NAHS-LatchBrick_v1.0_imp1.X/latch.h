/* 
 * File:   latch.h
 * Author: nijo
 *
 * Created on 2. November 2020, 20:01
 */

#ifndef LATCH_H
#define	LATCH_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdbool.h>
    
extern volatile bool latch_interrupt;
void latch_interrupt_handler(bool default_increase_write_pos);


#ifdef	__cplusplus
}
#endif

#endif	/* LATCH_H */

