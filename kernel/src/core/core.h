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

int32_t initCore( void );

#endif /* CORE_H_ */
