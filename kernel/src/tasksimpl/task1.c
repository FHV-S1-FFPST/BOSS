/*
 * task1.c
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */

#include "task1.h"

int32_t
task1( void* args )
{
	volatile uint32_t counter = 0;

	while( 1 )
	{
		uint32_t x1 = 22;
		uint32_t x2 = 23;
		uint32_t x3 = 24;
		uint32_t x4 = 25;

		volatile uint32_t z = x1 + x2 + x3 + x4;
		counter += z;
	}

	// TODO: handle exit of processes: through manipulating LR?
	return 0;
}
