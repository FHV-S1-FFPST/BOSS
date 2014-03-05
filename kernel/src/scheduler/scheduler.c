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
	// TODO: create new task-entry in structure
	// TODO: PC = entryPoint

	return entryPoint( 0 );
}

int32_t
fork()
{
	// TODO: clone the task-structure of the current running process
	// TODO: child needs to return 0 (r0 = 0) and parent > 0 (r0 = pid)

	return 0;
}

int32_t
sleep( uint32_t millis )
{
	// TODO: tell scheduler to keep it not ready for the next millis

	return 0;
}
