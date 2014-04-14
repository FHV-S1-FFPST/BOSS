/*
 * scheduler.c
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#include <stdio.h>
#include <string.h>

#include "scheduler.h"

#include "../core/core.h"
#include "../common/common.h"
#include "../task/task.h"
#include "../task/taskTable.h"
#include "../timer/irqtimer.h"
#include "../irq/irq.h"

static uint32_t runningPID = 0;

#define SCHEDULE_INTERVAL_MS			1000

// prototypes of this module
uint32_t getNextReady();
int32_t createTask( task_func entryPoint );
uint32_t schedule( UserContext* ctx );			// IRQ callback prototype

// TODO: idle-process MUST NOT be in normal task-list otherwise it will be scheduled round robin

uint32_t
schedInit()
{
	irqRegisterClbk( schedule, GPT2_IRQ );
	irqTimerInit( SCHEDULE_INTERVAL_MS );

	// TODO: this shouldnt be necessary anymore because this is handled inside timer
	// NOTE: need to waste some time, otherwise IRQ won't hit
	volatile uint32_t i = 100000;
	while ( i > 0 )
		--i;

	return 0;
}

void
schedStart()
{
	irqTimerStart();
}

void
saveCtxToTask( UserContext* ctx, Task* task)
{
	task->state = READY;
	task->pc = ctx->pc;
	task->cpsr = ctx->cpsr;
	memcpy( task->reg, ctx->regs, sizeof( task->reg ) );
}

void
restoreCtxFromTask( UserContext* ctx, Task* task )
{
	ctx->pc = task->pc;
	ctx->cpsr = task->cpsr;
	memcpy( ctx->regs, task->reg, sizeof( task->reg ) );
}

uint32_t
scheduleNextReady( UserContext* ctx )
{
	uint32_t nextPID = getNextReady();
	Task* task = getTask( nextPID );
	if ( task->state == READY )
	{
		task->state = RUNNING;
		restoreCtxFromTask( ctx, task );

		runningPID = nextPID;

		// signal: a task was scheduled
		return 1;
	}

	return 0;
}

uint32_t
saveCurrentRunning( UserContext* ctx )
{
	Task* runningTask = getTask( runningPID );
	if ( runningTask->state == RUNNING )
	{
		saveCtxToTask( ctx, runningTask );
	}

	return 0;
}

// NOTE: this is a callback called by irq
uint32_t
schedule( UserContext* ctx )
{
	uint32_t ret = 0;

	saveCurrentRunning( ctx );

	ret = scheduleNextReady( ctx );

	irqTimerResetCounterAndInterrupt();

	return ret;
}

uint32_t
getNextReady()
{
	uint32_t i = 0;
	uint32_t currentMillis = getSysMillis();

	for ( i = 1; i <= MAX_TASKS; i++ )
	{
		uint32_t pid = ( runningPID + i ) % MAX_TASKS;
		Task* task = getTask( pid );

		if ( SLEEPING == task->state )
		{
			if ( task->sleepUntil <= currentMillis )
			{
				task->state = READY;
				return pid;
			}
		}
		else if( READY == task->state )
		{
			return pid;
		}
	}

	return runningPID;
}

void allocateStackPointer( Task* task )
{
	// TODO: replace malloc by other facilitys when virtual memory and process creation is implemented
	void* stackPtr = ( void*) malloc( 1024 );
	memset( stackPtr, 'a', 1024 );

	task->reg[ 13 ] = ( uint32_t ) stackPtr;
}

void initializeTask( Task* task, task_func entryPoint )
{
	task->state = READY;
	task->pid = getNextFreePID();
	task->pc = ( uint32_t* ) entryPoint;
	task->pc++; 				// NOTE: increment program-counter by 4bytes because scheduling is done through IRQ-handler which will return by SUBS
	task->cpsr = 0x60000110; // user-mode and IRQs enabled
}

int32_t
createTask( task_func entryPoint )
{
	Task newTask;

	initializeTask( &newTask, entryPoint );

	allocateStackPointer( &newTask );

	addTask( &newTask );

	return 0;
}

int32_t
fork()
{
	Task newTask;

	initializeTask( &newTask, ( task_func ) currentUserCtx->pc );

	allocateStackPointer( &newTask );

	// the child process will receive the content of the registers of the parent process
	memcpy( newTask.reg, currentUserCtx->regs, sizeof( newTask.reg ) );
	// set register 0 of child-process to 0 to notify that the process is the child process
	newTask.reg[ 0 ] = 0;

	addTask( &newTask );

	// fork will return 1 for caller which is the parent process
	currentUserCtx->regs[ 0 ] = 1;

	return 0;
}

int32_t
sleep( uint32_t millis )
{
	Task* runningTask = getTask( runningPID );
	// NOTE: runningTask MUST be in state Running
	saveCtxToTask( currentUserCtx, runningTask );

	uint32_t systemMillis = getSysMillis();

	runningTask->state = SLEEPING;
	runningTask->sleepUntil = systemMillis + millis;

	// TODO: problem: when new to schedule task has never run, the PC will point one instruction too far because createtask incremented it
	// 		 it is ok when it already ran because the current instruction is interrupted by the IRQ and thus not executed and needs to be executed again

	scheduleNextReady( currentUserCtx );

	return 0;
}
