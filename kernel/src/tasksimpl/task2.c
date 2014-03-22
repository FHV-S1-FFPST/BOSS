/*
 * task2.c
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */

#include "task2.h"

int32_t
task2( void* args )
{
	volatile uint32_t counter = 0;

	while( 1 )
	{
		uint32_t x1 = 32;
		uint32_t x2 = 33;
		uint32_t x3 = 34;
		uint32_t x4 = 35;

		volatile uint32_t z = x1 + x2 + x3 + x4;
		counter += z;
	}

	// TODO: handle exit of processes: through manipulating LR?
	return 0;
}
