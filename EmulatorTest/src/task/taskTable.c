/*
 * taskTable.c
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#include "taskTable.h"

static Task taskTable[ MAX_TASKS ];

static uint8_t numTasks = 0;
static uint8_t nextFreePID = 0;

Task* getTask( uint8_t pid )
{
	if( pid > MAX_TASKS ) {
		return ( void* ) 0;
	}

	return &taskTable[ pid ];
}

int32_t addTask( Task* task )
{
	if( numTasks >= MAX_TASKS )
	{
		return 1;
	}

	taskTable[ task->pid ] = *( task );
	numTasks++;

	int i;
	int nextPID = task->pid + 1 % MAX_TASKS;
	for(i = 0; i < MAX_TASKS; i++) {
		if(getTask(nextPID)->state == INVALID) {
			nextFreePID = nextPID;
			break;
		} else {
			nextPID = (++nextPID) % MAX_TASKS;
		}
	}

	return 0;
}

uint8_t getNextFreePID() {
	return nextFreePID;
}

uint8_t getNumOfTasks() {
	return numTasks;
}
