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
#include "../timer/gptimer.h"
#include "../task/task.h"
#include "../task/taskTable.h"

// prototypes of this module
uint32_t getNextReady();
int32_t createTask( task_func entryPoint );

static uint32_t runningPID = 0;

uint32_t
initScheduler()
{
	reg32w(INTCPS_MIR_CLEAR1, 0, (1 << 6));
	reg32w(GPTIMER2_BASE, GPTIMER_TCRR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TIER, GPTIMER_MATCH);
	reg32w(GPTIMER2_BASE, GPTIMER_TMAR, (1 << 31));
	reg32w(GPTIMER2_BASE, GPTIMER_TLDR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TWER, 0x01);
	reg32w(GPTIMER2_BASE, GPTIMER_TISR, 0x03);
	reg32w(GPTIMER2_BASE, GPTIMER_TTGR, 0x00);
	//reg32w(GPTIMER2_BASE, GPTIMER_TCLR, 0x6B);
	reg32w(GPTIMER2_BASE, GPTIMER_TCLR, (1 << 6) | 0x03);

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
	if ( runningTask->state == READY )
	{
		task->state = RUNNING;
		//_schedule_asm( task->pc, task->cpsr, task->reg );
		*userPc = runningTask->pc;
		*userCpsr = runningTask->cpsr;
		memcpy( userRegs, runningTask->reg, sizeof( runningTask->reg ) );

		// signal: a task was scheduled
		ret = 1;
	}

	return ret;
}

static uint32_t
getNextReady()
{
	uint32_t i = 0;
	uint32_t pid = 0;

	for ( i = 1; i <= MAX_TASKS; i++ )
	{
		pid = ( runningPID + i % MAX_TASKS );

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
	newTask.pc = ( uint32_t* ) entryPoint;
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
