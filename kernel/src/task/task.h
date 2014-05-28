/*
 * task.h
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#ifndef TASK_H_
#define TASK_H_

#include <boss.h>

typedef enum {
	INVALID = 0,
	READY,
	RUNNING,
	BLOCKED,
	SLEEPING,
	WAITING_MSG
} TaskState;

typedef struct {
	// process identification
	uint32_t pid;

	uint32_t* initPC;

	// processor state information
	uint32_t* pc;
	uint32_t reg[15];
	uint32_t cpsr;

	// process control information
	TaskState state;

	// time to wait for message
	uint32_t waitUntil;
} Task;

#endif /* TASK_H_ */
