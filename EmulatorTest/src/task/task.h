/*
 * task.h
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#ifndef TASK_H_
#define TASK_H_

#include <boss.h>

#define MSG_QUEUE_SIZE 16

typedef enum {
	INVALID = 0,
	READY,
	RUNNING,
	BLOCKED,
	WAITING
} TaskState;

typedef struct {
	// process identification
	uint8_t pid;

	uint32_t* initPC;

	// processor state information
	uint32_t* pc;
	int32_t regs[15];
	uint32_t cpsr;

	// process control information
	TaskState state;

	// time to wait for message ...
	uint32_t waitUntil;
	// ... from channel
	uint32_t waitChannel;

	// address of page-table of this task
	uint32_t* pageTable;
} Task;

#endif /* TASK_H_ */
