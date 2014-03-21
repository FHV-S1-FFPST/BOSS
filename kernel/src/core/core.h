/*
 * core.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef CORE_H_
#define CORE_H_

#include <inttypes.h>

typedef struct
{
	uint32_t cpsr;
	uint32_t* pc;
	uint32_t regs[15];
} UserContext;

typedef enum
{
	USER = 0x10,
	FIQ = 0x11,
	IRQ = 0x12,
	SUPERVISOR = 0x13,
	ABORT = 0x17,
	SYSTEM = 0x1F,
} SystemState;

int32_t initCore( void );

SystemState querySystemState( void );

#endif /* CORE_H_ */
