/*
 * taskTable.c
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#include "taskTable.h"

static Task taskTable[MAX_TASKS];
static uint32_t nextFreePID = 0;

Task* getTask(uint32_t pid) {
	if(pid > MAX_TASKS) {
		return (void *)0;
	}
	return &taskTable[pid];
}


int32_t addTask(Task task) {

	if(nextFreePID < MAX_TASKS) {
		return 1;
	}

	taskTable[nextFreePID] = task;
	nextFreePID++;
	return 0;
}



