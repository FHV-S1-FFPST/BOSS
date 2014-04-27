/*
 * common.h
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <inttypes.h>

typedef uint8_t bool;

#define TRUE 	1
#define FALSE 	0

#define READ_REGISTER( addr ) (*(volatile uint32_t *)( addr ))
#define READ_REGISTER_OFFSET( base, offset ) (*(volatile uint32_t *)((base)+(offset)))

#define BIT_SET( val, bits ) val |= bits
#define BIT_CLEAR( val, bits ) val &= ~(bits)
#define BIT_KEEP( val, bits ) val &= bits
#define BIT_CLEAR_ALL( val ) val = 0x0
#define BIT_CHECK( val, bits ) val & bits

// TODO: add max retries to prevent endless loops!
#define AWAIT_BITS_SET( val, bits ) while ( ( val & bits ) != bits ) {}
#define AWAIT_BITS_CLEARED( val, bits ) while ( val & bits ) {}
#define AWAIT_BITS_ALL_CLEARED( val ) while ( val ) {}

#endif /* COMMON_H_ */
