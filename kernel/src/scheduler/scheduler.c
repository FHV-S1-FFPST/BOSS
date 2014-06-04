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
#include "../mmu/mmu.h"

/* NOTES ABOUT SCHEDULING AND PC HANDLING //////////////////////////////////////////////////////////////////////////////////////////////////
fact1: during an IRQ the current OP is canceled and the PC incremented
fact2: thus IRQ handler returns doing SUBS on the PC so to repeat the canceled OP
fact3: during a software interrupt no OP is canceled and return is done by a simple MOVS
fact4: a software interrupt can lead to a scheduling of another task

conclusion1: when a task which has never run is scheduled by IRQ the PC must be incremented because SUBS will otherwise point BEFORE taskentry
conclusion2: when a task which has never run is scheduled by SWI the PC must stay the same because PC was not changed and wont be upon return
conclusion3: when a task which was interrupted by IRQ is scheduled by IRQ the PC must stay the same because PC was incremented but SUBS will handle decrementing to repeat instruction
				enter by CPU: PC++
				leave by ASM: PC--
conclusion4: when a task which was interrupted by IRQ is scheduled by SWI the PC must be subtracted because PC was incremented but SWI will not repeat instruction thus decrement PC to repeat
				enter by CPU: PC++
				leave by software: PC = PC
conclusion5: when a task which was interrupted by SWI is scheduled by SWI the PC must stay the same because PC was not changed and wont be upon return
				enter by CPU: PC = PC
				leave by software: PC = PC
conclusion6: when a task which was interrupted by SWI is scheduled by IRQ the PC must be incremented because PC will be changed by return through SUBS thus prevent to repeat last SWI instruction
				enter by software: PC = PC
				leave by ASM: PC--

problem: we don't know how the task was interrupted (IRQ or SWI) and we don't want to store it
solution: transform all to the case of IRQ schedules IRQ
				enter: PC++
				leave: PC--

IRQ -> IRQ
	enter by CPU: PC++
	leave by ASM: PC--

IRQ -> SWI
	enter by CPU: PC++
	leave by software: PC--

SWI -> SWI
	enter by software: PC++
	leave by software: PC--

SWI -> IRQ
	enter by software: PC++
	leave by ASM: PC--
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

#define SCHEDULE_INTERVAL_MS		10
#define USERMODE_WITHIRQ_CPSR		0x60000110

// module-local data //////////////////////////////////////////////
static uint8_t runningPID = 0;
// module-local functions /////////////////////////////////////////
static uint32_t scheduleNextReady( UserContext* ctx );
static Task* getNextReady();
static void initializeTask( Task* task, uint32_t* entryPoint );
static int32_t idleTaskFunc( void );
static void saveCtxToTask( UserContext* ctx, Task* task );
static void restoreCtxFromTask( UserContext* ctx, Task* task );
static uint32_t saveCurrentRunning( UserContext* ctx );
static void allocateStackPointer( Task* task );
///////////////////////////////////////////////////////////////////

uint32_t
schedInit()
{
	// create idle-task first, it MUST BE at pid 0
	createTask( ( uint32_t* ) idleTaskFunc );

	irqRegisterClbk( schedule, GPT2_IRQ );
	irqTimerInit( SCHEDULE_INTERVAL_MS );

	return 0;
}

void
schedStart()
{
	irqTimerStart();
}

uint8_t
getCurrentPid( void )
{
	return runningPID;
}

Task*
createTask( uint32_t* entryPoint )
{
	Task newTask;

	initializeTask( &newTask, entryPoint );

	allocateStackPointer( &newTask );

	newTask.pageTable = mmu_allocate_task( newTask.pid );

	addTask( &newTask );

	return getTask( newTask.pid );
}

/*
int32_t
fork()
{
	// TODO: pc is other space, need to calculate it according to the image

	Task* newTask = createTask( currentUserCtx->pc );

	// the child process will receive the content of the registers of the parent process
	memcpy( newTask->reg, currentUserCtx->regs, sizeof( newTask->reg ) );
	// set register 0 of child-process to 0 to notify that the process is the child process
	newTask->reg[ 0 ] = 0;

	// fork will return 1 for caller which is the parent process
	currentUserCtx->regs[ 0 ] = 1;

	return 0;
}
*/

/*
int32_t
sleep( uint32_t millis )
{
	saveCurrentRunning( currentUserCtx );
	uint32_t systemMillis = getSysMillis();

	Task* runningTask = getTask( runningPID );
	runningTask->state = SLEEPING;
	runningTask->waitUntil = systemMillis + millis;

	// TODO: problem: when new to schedule task has never run, the PC will point one instruction too far because createtask incremented it
	// 		 it is ok when it already ran because the current instruction is interrupted by the IRQ and thus not executed and needs to be executed again

	scheduleNextReady( currentUserCtx );

	return 0;
}
*/

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
scheduleNextReady( UserContext* ctx )
{
	Task* task = getNextReady();
	task->state = RUNNING;

	SystemState sysState = querySystemState();

	// task has not run so far, need to adjust the PC according to the systemstate
	if ( 0 == task->pc )
	{
		// NOTE: when a task which has never run is scheduled by IRQ the PC must be incremented because SUBS will otherwise point BEFORE taskentry
		if ( IRQ == sysState )
		{
			task->pc = task->initPC + 1;
		}
		// NOTE: when a task which has never run is scheduled by SWI the PC must stay the same because PC was not changed and wont be upon return
		else
		{
			task->pc = task->initPC;
		}
	}
	else
	{
		// NOTE: when a task which was interrupted by IRQ is scheduled by IRQ the PC must stay the same because PC was incremented by CPU but SUBS will handle decrementing to repeat instruction
		//		 -> incrementation was done by CPU and SUBS will be done by ASM
		// NOTE: when a task which was interrupted by SWI is scheduled by IRQ the PC must be incremented because PC will be changed by return through SUBS thus prevent to repeat last SWI instruction
		//		 -> incrementation was done already during SWI interrupt in software and SUBS will be done by ASM
		if ( IRQ == sysState )
		{
			task->pc = task->pc;
		}
		// NOTE: when a task which was interrupted by IRQ is scheduled by SWI the PC must be subtracted because PC was incremented by CPU but SWI will not repeat instruction thus decrement PC to repeat
		//		 -> incrementation was done by CPU and decrement will be done in software
		// NOTE: when a task which was interrupted by SWI is scheduled by SWI the PC must stay the same because PC was not changed and wont be upon return
		//		 -> incrementation was done by software in SWI mode and is decremented in software. It is imporant that in both SWI and IRQ interrupt the PC needs to be incremented for this to work
		else
		{
			task->pc--;
		}
	}

	restoreCtxFromTask( ctx, task );

	mmu_ttbSet( ( int32_t ) task->pageTable );
	mmu_setProcessID( task->pid );

	runningPID = task->pid;

	return 0;
}

uint32_t
saveCurrentRunning( UserContext* ctx )
{
	Task* task = getTask( runningPID );
	// TODO: when no task has ever run in the system there is no current running task, so
	// this if will hit one time only, thus try to remove it in future
	if ( RUNNING == task->state )
	{
		SystemState sysState = querySystemState();

		// NOTE: when a task which was interrupted by SWI is scheduled by IRQ the PC must be incremented because PC will be changed by return through SUBS thus prevent to repeat last SWI instruction
		// NOTE: when a task which was interrupted by IRQ is scheduled by SWI the PC must be subtracted because PC was incremented but SWI will not repeat instruction thus decrement PC to repeat
		// CONCLUSION: increment for SWI here and decrement according to systemstate in scheduleNextReady
		if ( SUPERVISOR == sysState )
		{
			ctx->pc++;
		}

		saveCtxToTask( ctx, task );
	}

	return 0;
}

Task*
getNextReady()
{
	uint32_t i = 1;
	uint32_t currentMillis = getSysMillis();

	while ( 1 )
	{
		uint32_t pid = ( runningPID + i ) % MAX_TASKS;
		Task* task = getTask( pid );

		// iterated through all tasks, no runnable found
		if ( pid == runningPID )
		{
			// current running task is still runnable, continue with it
			if ( RUNNING == task->state )
			{
				return task;
			}
			// current running task it NOT runnable, use idle-task because no other task found
			else
			{
				break;
			}
		}

		// task is waiting for a message
		if ( WAITING == task->state )
		{
			// a timeout is specified, check if it is hit
			if ( ( task->waitUntil ) && ( task->waitUntil <= currentMillis ) )
			{
				task->state = READY;
				// receive returns -1 when timeout occurs
				task->regs[ 0 ] = -1;

				return task;
			}
		}
		// task is ready to run and its not the idle-task because we won't schedule it
		else if( READY == task->state && 0 != pid )
		{
			return task;
		}

		i++;
	}

	// NOTE: no task to run found, return the idle-task which MUST BE always at index 0
	return getTask( 0 );
}

void
saveCtxToTask( UserContext* ctx, Task* task)
{
	task->state = READY;
	task->pc = ctx->pc;
	task->cpsr = ctx->cpsr;
	memcpy( task->regs, ctx->regs, sizeof( task->regs ) );
}

void
restoreCtxFromTask( UserContext* ctx, Task* task )
{
	ctx->pc = task->pc;
	ctx->cpsr = task->cpsr;
	memcpy( ctx->regs, task->regs, sizeof( task->regs ) );
}

void
allocateStackPointer( Task* task )
{
	// TODO: replace malloc by other facilitys when virtual memory and process creation is implemented
	void* stackPtr = ( void*) malloc( 1024 );
	memset( stackPtr, 'a', 1024 );

	task->regs[ 13 ] = ( uint32_t ) stackPtr;
}

void
initializeTask( Task* task, uint32_t* entryPoint )
{
	memset( task, 0, sizeof( Task ) );

	task->state = READY;
	task->pid = getNextFreePID();
	task->initPC = entryPoint;
	task->cpsr = USERMODE_WITHIRQ_CPSR;
}

int32_t
idleTaskFunc( void )
{
	volatile uint32_t counter = 0;

	while( 1 )
	{
		counter++;
	}
}
