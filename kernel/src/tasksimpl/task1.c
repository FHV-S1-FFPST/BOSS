/*
 * task1.c
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */
#undef KERNEL	// NOTE: if not used sleep, fork and getSysMillis will call the sys-calls directly instead of causing a trap through SWI

#include "task1.h"

int32_t
task1( void* args )
{
	if ( fork() )
	{
		// NOTE: at this point we are in the parent of fork

		while( 1 )
		{
			uint64_t startMillis = getSysMillis();

			sleep( 5000 );

			uint64_t stopMillis = getSysMillis();
			uint64_t deltaMillis = stopMillis - startMillis;

			deltaMillis++;
		}
	}
	else
	{
		// NOTE: at this point we are in the child of fork

		while( 1 )
		{
			uint64_t startMillis = getSysMillis();

			sleep( 10000 );

			uint64_t stopMillis = getSysMillis();
			uint64_t deltaMillis = stopMillis - startMillis;

			deltaMillis++;
		}
	}

	// TODO: handle exit of processes: through manipulating LR?
}
