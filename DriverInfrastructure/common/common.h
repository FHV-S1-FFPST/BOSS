/*
 * common.h
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <boss.h>

#define reg32r(b, r) (*(volatile uint32_t *)((b)+(r)))
#define reg32w(b, r, v) (*((volatile uint32_t *)((b)+(r))) = (v))

#define reg32m(b, r, v) ( *( ( volatile uint32_t* ) ( b + r ) ) |= ( v ) )

#define BIT_SET( val, bits ) val |= bits
#define BIT_CLEAR( val, bits ) val &= ~bits
#define BIT_CLEAR_ALL( val ) val = 0x0

#define AWAIT_BITS_SET( val, bits ) while ( ! ( val & bits ) ) {}
#define AWAIT_BITS_CLEARED( val, bits ) while ( val & bits ) {}
#define AWAIT_BITS_ALL_CLEARED( val ) while ( val ) {}

#define INTCPS_MIR_CLEAR1 0x482000A8
#define INTCPS_SIR_IRQ 0x48200040

#endif /* COMMON_H_ */
