/*
 * scheduler.c
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */


#include <stdio.h>

#include "scheduler.h"
#include "../common/common.h"
#include "../timer/gptimer.h"
#include "../task/task.h"
#include "../task/taskTable.h"

extern void _schedule_asm(uint32_t* pc, uint32_t cpsr, uint32_t userReg);

uint32_t initScheduler();
void schedule();
uint32_t getNextReady();
int32_t createTask( task_func entryPoint );

static uint32_t runningPID = MAX_TASKS;

uint32_t
initScheduler() {
	reg32w(INTCPS_MIR_CLEAR1, 0, (1 << 6));
	reg32w(GPTIMER2_BASE, GPTIMER_TCRR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TIER, GPTIMER_MATCH);
	reg32w(GPTIMER2_BASE, GPTIMER_TMAR, (1 << 31));
	reg32w(GPTIMER2_BASE, GPTIMER_TLDR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TWER, 0x01);
	reg32w(GPTIMER2_BASE, GPTIMER_TISR, 0x03);
	reg32w(GPTIMER2_BASE, GPTIMER_TTGR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TCLR, (1 << 6) | 0x03);

	return 0;
}

void
schedule( uint32_t* pc, uint32_t userCpsr, uint32_t* userRegs ) {

	reg32w(GPTIMER2_BASE, GPTIMER_TCLR, reg32r(GPTIMER2_BASE, GPTIMER_TCLR) & ~0x01);
	Task *runningTask = getTask(runningPID);

	if(runningTask->state == RUNNING) {
		if(getNumOfTasks() == 1) {
			return;
		}

		uint32_t i;
		for(i = 0; i < 15; i++) {
			runningTask->pc = pc;
			runningTask->reg[i] = *(userRegs + i);
			runningTask->cpsr = userCpsr;
		}

		uint32_t nextPID = getNextReady();
		Task nextTask = getTask(nextPID);

		_schedule_asm(nextTask->pc, nextTask->cpsr, nextTask->reg);
		// reset context from next

	}

	reg32w(GPTIMER2_BASE, GPTIMER_TTGR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TCLR, reg32r(GPTIMER2_BASE, GPTIMER_TCLR) | 0x01);

}

uint32_t
getNextReady() {

	uint32_t i;
	uint32_t pid = runningPID % MAX_TASKS;

	for(i = 0; i < MAX_TASKS; i++) {
		if(getTask(pid)->state == READY) {
			return pid;
		} else {
			pid = (++pid) % MAX_TASKS;
		}
	}

	return MAX_TASKS;

}

int32_t
createTask( task_func entryPoint )
{
	Task newTask;
	newTask.state = READY;
	newTask.pid = getNextFreePID();
	newTask.pc = (uint32_t*)entryPoint;

	addTask(newTask);
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
