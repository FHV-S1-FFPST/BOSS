/*
 * task1.c
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */
#undef KERNEL

#include "task1.h"

int32_t
task1( void* args )
{
	volatile uint32_t counter = 0;

	if ( fork() )
	{
		while( 1 )
		{
			counter++;

			uint64_t sysMillis = getSysMillis();

			sleep( 10000 );

			sysMillis = getSysMillis();
		}
	}
	else
	{
		while( 1 )
		{
			counter++;

			uint64_t sysMillis = getSysMillis();

			sleep( 10000 );

			sysMillis = getSysMillis();
		}
	}

	// TODO: handle exit of processes: through manipulating LR?
}
