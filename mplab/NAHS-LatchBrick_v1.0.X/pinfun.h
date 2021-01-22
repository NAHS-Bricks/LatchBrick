/* 
 * File:   pinfun.h
 * Author: nijo
 *
 * Created on 2. Januar 2021, 21:15
 */

#ifndef PINFUN_H
#define	PINFUN_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <avr/io.h>

#define pin_set_input(pin_bp) PORTA.DIRCLR = (1<<pin_bp)
#define pin_set_input_pullup(pin_bp) \
    PORTA.DIRCLR = (1<<pin_bp); \
    _SFR_MEM8(0x0410 + pin_bp) |= PORT_PULLUPEN_bm;
#define pin_set_input_hiz(pin_bp) \
    PORTA.DIRCLR = (1<<pin_bp); \
    _SFR_MEM8(0x0410 + pin_bp) &= ~PORT_PULLUPEN_bm;
#define pin_set_output(pin_pb) PORTA.DIRSET = (1<<pin_pb)
#define pin_set_output_high(pin_pb) PORTA.OUTSET = (1<<pin_pb)
#define pin_set_output_low(pin_pb) PORTA.OUTCLR = (1<<pin_pb)
#define pin_set_output_toggle(pin_pb) PORTA.OUTTGL = (1<<pin_pb)
#define pin_pullup_enable(pin_bp) _SFR_MEM8(0x0410 + pin_bp) |= PORT_PULLUPEN_bm
#define pin_pullup_disable(pin_bp) _SFR_MEM8(0x0410 + pin_bp) &= ~PORT_PULLUPEN_bm
#define pin_inverted_enable(pin_bp) _SFR_MEM8(0x0410 + pin_bp) |= PORT_INVEN_bm
#define pin_inverted_disable(pin_bp) _SFR_MEM8(0x0410 + pin_bp) &= ~PORT_INVEN_bm
#define pin_read(pin_pb) (PORTA.IN & (1<<pin_pb))
#define pin_int_disable(pin_bp) _SFR_MEM8(0x0410 + pin_bp) &= ~0b111
#define pin_int_enable_bothedges(pin_bp) _SFR_MEM8(0x0410 + pin_bp) |= PORT_ISC_BOTHEDGES_gc
    
#ifdef	__cplusplus
}
#endif

#endif	/* PINFUN_H */

