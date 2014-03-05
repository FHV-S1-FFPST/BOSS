/*
 * task.h
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#ifndef TASK_H_
#define TASK_H_

#include <boss.h>
#include <setjmp.h>

typedef enum {
	INVALID = 0,
	READY,
	RUNNING,
	BLOCKED
} TaskState;

typedef struct {
	// process identification
	uint32_t pid;

	// processor state information
	uint32_t pc;
	uint32_t sp;
	jmp_buf reg;
	uint32_t cpsr;

	// process control information
	TaskState state;
} Task;



#endif /* TASK_H_ */