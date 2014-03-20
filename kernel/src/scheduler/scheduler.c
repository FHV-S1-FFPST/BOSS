/*
 * scheduler.c
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#include <stdio.h>
#include <string.h>

#include "scheduler.h"
#include "../common/common.h"
#include "../timer/timer.h"
#include "../task/task.h"
#include "../task/taskTable.h"

// prototypes of this module
uint32_t getNextReady();
int32_t createTask( task_func entryPoint );

static uint32_t runningPID = 0;

uint32_t
initScheduler()
{
	// want IRQ from timer every 100ms
	timerInit( TIMER2_ID, 100 );

	// NOTE: need to waste some time, otherwise IRQ won't hit
	volatile uint32_t i = 100000;
	while ( i > 0 )
		--i;

	return 0;
}

uint32_t
schedule( uint32_t* userCpsr, uint32_t* userPc, uint32_t* userRegs )
{
	uint32_t ret = 0;

	Task* runningTask = getTask( runningPID );
	if ( runningTask->state == RUNNING )
	{
		runningTask->state = READY;
		runningTask->pc = *userPc;
		runningTask->cpsr = *userCpsr;
		memcpy( runningTask->reg, userRegs, sizeof( runningTask->reg ) );
	}

	uint32_t nextPID = getNextReady();
	Task* task = getTask( nextPID );
	if ( task->state == READY )
	{
		task->state = RUNNING;
		*userPc = task->pc;
		*userCpsr = task->cpsr;
		memcpy( userRegs, task->reg, sizeof( task->reg ) );

		runningPID = nextPID;

		// signal: a task was scheduled
		ret = 1;
	}

	return ret;
}

uint32_t
getNextReady()
{
	uint32_t i = 0;
	uint32_t pid = 0;

	for ( i = 1; i <= MAX_TASKS; i++ )
	{
		pid = ( runningPID + i ) % MAX_TASKS ;

		if( READY == getTask( pid )->state )
		{
			return pid;
		}
	}

	return runningPID;
}

int32_t
createTask( task_func entryPoint )
{
	Task newTask;
	newTask.state = READY;
	newTask.pid = getNextFreePID();
	newTask.pc = ( ( uint32_t ) entryPoint ) + 4; // need to increment by 4 bytes because return is done by SUBS ... #4
	newTask.cpsr = 0x60000110; // user-mode and IRQs enabled

	// TODO: need a valid stack-pointer
	// TODO: check to what we need to set the LR (newTask.reg[ 14 ]
	void* stackPtr = ( void*) malloc( 1024 );
	memset( stackPtr, 'a', 1024 );

	newTask.reg[ 13 ] = ( uint32_t ) stackPtr;

	addTask( &newTask );

	return 0;
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
