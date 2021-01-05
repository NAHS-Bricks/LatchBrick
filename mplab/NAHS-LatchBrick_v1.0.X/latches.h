/* 
 * File:   latches.h
 * Author: nijo
 *
 * Created on 5. Januar 2021, 09:52
 */

#ifndef LATCHES_H
#define	LATCHES_H

#ifdef	__cplusplus
extern "C" {
#endif


    #include "globals.h"

    extern volatile bool latches_interrupt;
    void latches_interrupt_reaction(bool move_queue);
    void latches_catch_state();


#ifdef	__cplusplus
}
#endif

#endif	/* LATCHES_H */

