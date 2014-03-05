/*
 * scheduler.c
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */


#include "scheduler.h"

int32_t
createTask( task_func entryPoint )
{
	return entryPoint( 0 );
}

int32_t
fork()
{
	return 0;
}

int32_t
sleep( uint32_t millis )
{
	return 0;
}
