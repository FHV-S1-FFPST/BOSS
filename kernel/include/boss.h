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
#define NULL_CHANNEL			0
#define SERIAL_CHANNEL			1
#define HDMI_CHANNEL			2
///////////////////////////////////////////////////////////////////////////////

// Sys-Call IDs ///////////////////////////////////////////////////////////////
// IPC CALLS
#define SYSC_CH_OPEN			8
#define SYSC_CH_CLOSE			9
#define SYSC_CH_ATTACH			10

#define SYSC_SEND				11
#define SYSC_RECEIVE			12
#define SYSC_SENDRCV			13

// TASK-MANAGEMENT CALLS
//#define SYSC_CREATETASK			14
//#define SYSC_FORK				15
//#define SYSC_SLEEP				16

// MISC CALLS
#define SYSC_SYSMILLIS			17
#define SYSC_PID				18
///////////////////////////////////////////////////////////////////////////////

// defined SWI-aliases for the given sys-calls for user-apps //////////////////
#ifndef KERNEL
	// IPC CALLS
	#pragma SWI_ALIAS( channelOpen, SYSC_CH_OPEN );
	#pragma SWI_ALIAS( channelClose, SYSC_CH_CLOSE );
	#pragma SWI_ALIAS( channelAttach, SYSC_CH_ATTACH );

	#pragma SWI_ALIAS( send, SYSC_SEND );
	#pragma SWI_ALIAS( receive, SYSC_RECEIVE );
	#pragma SWI_ALIAS( sendrcv, SYSC_SENDRCV );

	// TASK-MANAGEMENT CALLS
	//#pragma SWI_ALIAS( createTask, SYSC_CREATETASK );
	//#pragma SWI_ALIAS( fork, SYSC_FORK );
	//#pragma SWI_ALIAS( sleep, SYSC_SLEEP );

	// MISC CALLS
	#pragma SWI_ALIAS( getSysMillis, SYSC_SYSMILLIS );
	#pragma SWI_ALIAS( getPid, SYSC_PID );
#endif
///////////////////////////////////////////////////////////////////////////////

//typedef int32_t (*task_func) ( void* args );

// SYSTEM-STRCUTURES

#define MESSAGE_MAX_DATA_SIZE 1024

typedef struct
{
	uint32_t id;

	uint8_t receiver;
	uint8_t sender;

	uint8_t data[ MESSAGE_MAX_DATA_SIZE ];
	uint8_t dataSize;
} MESSAGE;

// SYSTEM-CALLS ///////////////////////////////////////////////////////////////

// IPC calls
/**
 * Opens a channel with the given id.
 * If id is already in use or failed then will return 1.
 * If succeeds return 0.
 */
int32_t channelOpen( uint32_t channelId );
/**
 * closes an opened channel with given id.
 * If channel doesnt exist or failed, return 1.
 * If succeeds return 0.
 */
int32_t channelClose( uint32_t channelId );
/**
 * Attaches to an opened channel with the given id.
 * If channel doesnt exist it returns 1.
 * If succeeds return 0.
 */
int32_t channelAttach( uint32_t channelId );

/**
 * Sends async a message to the given channel.
 */
int32_t send( uint32_t channelId, MESSAGE* msg );

/**
 * Receives a message from a given channel waiting timeout milliseconds for a message to arrive.
 * If timeout is > 0 it will block at least timeout ms until a message arrives.
 * If timeout is 0, it will block forever.
 * If timeout is -1, it will return immediately if no message is present and will have msg set to 0.
 *
 * returns 0 if message a was received
 */
int32_t receive( uint32_t channelId, MESSAGE* msg, int32_t timeout );

/**
 * Sends a message and receives a response waiting for timeout milliseconds.
 * msg is used to hold both the sending message and the receiving message.
 * If timeout is 0 this will block forever.
 */
int32_t sendrcv( uint32_t channelId, MESSAGE* msg, uint32_t timeout );

// Task-management calls
/**
 * Starts a process from the given process-function.
 */
//int32_t createTask( task_func entryPoint );

/**
 * Forks the task. Will return 0 for the child-task and > 0 for the parent-task.
 *
 * int32_t fork();
 */

/**
 * Suspends the task for at least the given milliseconds.
 * TODO: replace by a receive on the null-channel with a given timeout of millis
 *
 * int32_t sleep( uint32_t millis );
 */

/**
 * Returns the milliseconds since the system was started.
 */
uint64_t getSysMillis();

/**
 * Returns the pid of the current process
 */
int32_t getPid();
///////////////////////////////////////////////////////////////////////////////

#endif /* BOSS_H_ */
