/*
 * boss.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef BOSS_H_
#define BOSS_H_

#include <inttypes.h>

/**
 * This is the header-file to be used by applications
 * need to access the API of BOSS and OS-Specific stuff
 */

// agreed channel-ids /////////////////////////////////////////////////////////
#define KEY_IN_CHANNEL			0
#define TIMER_CHANNEL			1
///////////////////////////////////////////////////////////////////////////////

// Sys-Call IDs ///////////////////////////////////////////////////////////////
// IPC CALLS
#define SYSC_SEND				8
#define SYSC_RECEIVE			9
#define SYSC_SENDRCV			10

// TASK-MANAGEMENT CALLS
#define SYSC_CREATETASK			11
#define SYSC_FORK				12
#define SYSC_SLEEP				13

// MISC CALLS
#define SYSC_SYSMILLIS			14
///////////////////////////////////////////////////////////////////////////////

// defined SWI-aliases for the given sys-calls for user-apps //////////////////
#ifndef KERNEL
	// IPC CALLS
	#pragma SWI_ALIAS( send, SYSC_SEND );
	#pragma SWI_ALIAS( receive, SYSC_RECEIVE );
	#pragma SWI_ALIAS( sendrcv, SYSC_SENDRCV );

	// TASK-MANAGEMENT CALLS
	#pragma SWI_ALIAS( createTask, SYSC_CREATETASK );
	#pragma SWI_ALIAS( fork, SYSC_FORK );
	#pragma SWI_ALIAS( sleep, SYSC_SLEEP );

	// MISC CALLS
	#pragma SWI_ALIAS( getSysMillis, SYSC_SYSMILLIS );
#endif
///////////////////////////////////////////////////////////////////////////////

typedef int32_t (*task_func) ( void* args );

// SYSTEM-CALLS ///////////////////////////////////////////////////////////////

// IPC calls
/**
 * Blocking send of dataSize bytes stored in buffer data to channel found in channelId
 * There must be a receiver to consume the data
 */
int32_t send( uint32_t channelId, uint8_t* data, uint8_t dataSize );

/**
 * Blocking receive of max dataSize bytes to buffer data from channel found in channelId
 * 	Returns the amount of bytes stored in dataSize
 */
int32_t receive( uint32_t channelId, uint8_t* data, uint8_t dataSize );

/**
 * TODO: describe and specify
 */
int32_t sendrcv( uint32_t channelId, uint8_t* data, uint8_t dataSize );

// Task-management calls
/**
 * Starts a process from the given process-function.
 */
int32_t createTask( task_func entryPoint );

/**
 * Forks the task. Will return 0 for the child-task and > 0 for the parent-task.
 */
int32_t fork();

/**
 * Suspends the task for at least the given milliseconds.
 */
int32_t sleep( uint32_t millis );

/**
 * Returns the milliseconds since the system was started.
 */
uint64_t getSysMillis();
///////////////////////////////////////////////////////////////////////////////

#endif /* BOSS_H_ */
