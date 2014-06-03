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

typedef struct
{
	uint32_t channel;

	uint32_t id;

	uint8_t sender;

	uint8_t data[ MESSAGE_MAX_DATA_SIZE ];
	uint8_t dataSize;
} IPC_MESSAGE;

typedef struct {
	// process identification
	uint8_t pid;

	uint32_t* initPC;

	// processor state information
	uint32_t* pc;
	uint32_t reg[15];
	uint32_t cpsr;

	// process control information
	TaskState state;

	// time to wait for message ...
	uint32_t waitUntil;
	// ... with given id ...
	uint32_t waitMsgId;
	// ... from channel with id
	uint32_t waitChannel;

	// address of page-table of this task
	uint32_t* pageTable;

	IPC_MESSAGE* msgQueue[ MSG_QUEUE_SIZE ];
} Task;

#endif /* TASK_H_ */
