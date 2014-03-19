/*
 * idle.c
 *
 *  Created on: 18.03.2014
 *      Author: Thaler
 */

#include "idle.h"

int32_t
idleTask( void* args )
{
	volatile uint32_t counter = 0;

	uint32_t x1 = 42;
	uint32_t x2 = 43;
	uint32_t x3 = 44;
	uint32_t x4 = 45;

	while( 1 )
	{
		volatile uint32_t z = x1 + x2 + x3 + x4;
		counter += z;
	}
}
