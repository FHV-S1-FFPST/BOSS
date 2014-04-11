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

			sleep( 5000 );
		}
	}
	else
	{
		while( 1 )
		{
			counter++;

			sleep( 2000 );
		}
	}

	// TODO: handle exit of processes: through manipulating LR?
}
