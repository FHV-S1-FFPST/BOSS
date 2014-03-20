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

#define AWAIT_BITS_SET( addr, bits ) while ( ! ( *addr & bits ) ) {}
#define AWAIT_BITS_CLEARED( addr, bits ) while ( *addr & bits ) {}

#define INTCPS_MIR_CLEAR1 0x482000A8
#define INTCPS_SIR_IRQ 0x48200040

#endif /* COMMON_H_ */
