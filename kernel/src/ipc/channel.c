/*
 * channel.c
 *
 *  Created on: 03.06.2014
 *      Author: Jonathan Thaler
 */


#include "channel.h"

#include "../common/common.h"
#include "../scheduler/scheduler.h"
#include "../mmu/mmu.h"

#include <stdlib.h>
#include <string.h>

// module-local defines
#define MAX_CHANNELS 					16
#define MESSAGE_QUEUE_LENGTH 			32	// MUST BE a power of 2
#define MAX_SUBSCRIBERS		 			32	// MUST BE a power of 2

#define NEXT_SUBSCRIBER_INSERTPOS( channel ) ( ( channel->subscribersInsertPos + 1 ) & ( MAX_SUBSCRIBERS - 1 ) )
#define IS_SUBSCRIBERS_EMPTY( channel ) ( channel->subscribersInsertPos == channel->subscribersStartPos  )
#define IS_SUBSCRIBERS_FULL( channel ) ( ( ! IS_SUBSCRIBERS_EMPTY( channel ) ) && ( NEXT_SUBSCRIBER_INSERTPOS( channel ) == channel->subscribersStartPos ) )

#define NEXT_MSG_QUEUE_INSERTPOS( channel ) ( ( channel->msgQueueInsertPos + 1 ) & ( MESSAGE_QUEUE_LENGTH - 1 ) )
#define NEXT_MSG_QUEUE_STARTPOS( channel ) ( ( channel->msgQueueStartPos + 1 ) & ( MESSAGE_QUEUE_LENGTH - 1 ) )
#define IS_MSG_QUEUE_EMPTY( channel ) ( channel->msgQueueInsertPos == channel->msgQueueStartPos  )
#define IS_MSG_QUEUE_FULL( channel ) ( ( ! IS_MSG_QUEUE_EMPTY( channel ) ) && ( NEXT_MSG_QUEUE_INSERTPOS( channel ) == channel->msgQueueStartPos ) )
////////////////////////////////////////////////////////

// module-local data-structures
typedef struct
{
	int32_t id;

	int32_t subscribersStartPos;
	int32_t subscribersInsertPos;
	Task* subscribers[ MAX_SUBSCRIBERS ];

	int32_t msgQueueStartPos;
	int32_t msgQueueInsertPos;
	MESSAGE msgQueue[ MESSAGE_QUEUE_LENGTH ];
} IPC_CHANNEL;
////////////////////////////////////////////////////////

// module-local functions
static IPC_CHANNEL* getChannelById( uint32_t channelId );
static void copyMessageToTaskSpace( MESSAGE* msg, Task* targetTask, Task* currentRunning );
////////////////////////////////////////////////////////

// module-local data
static IPC_CHANNEL _channels[ MAX_CHANNELS ];
static MESSAGE cpyMsg;
////////////////////////////////////////////////////////

uint32_t
channel_open( uint32_t channelId )
{
	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 != channel )
	{
		return 1;
	}

	_channels[ channelId ].id = channelId;

	return 0;
}

uint32_t
channel_close( uint32_t channelId )
{
	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 == channel )
	{
		return 1;
	}

	channel->id = NULL_CHANNEL;

	return 0;
}

uint32_t
channel_subscribe( uint32_t channelId, Task* task )
{
	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 == channel )
	{
		return 1;
	}

	if ( IS_SUBSCRIBERS_FULL( channel ) )
	{
		return 1;
	}

	channel->subscribers[ channel->subscribersInsertPos ] = task;
	channel->subscribersInsertPos = NEXT_SUBSCRIBER_INSERTPOS( channel );

	return 0;
}

uint32_t
channel_receivesMessage( uint32_t channelId, MESSAGE* msg, Task* currentRunning )
{
	uint32_t i = 0;
	bool consumed = FALSE;

	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 == channel )
	{
		return 1;
	}

	// check if any subscriber to this channel is waiting for a message
	for ( i = 0; i < MAX_SUBSCRIBERS; ++i )
	{
		Task* subscriber = channel->subscribers[ i ];
		if ( subscriber )
		{
			// the subscriber is WAITING for a message from this channel => notify
			if ( ( WAITING == subscriber->state ) && ( channel->id == subscriber->waitChannel ) )
			{
				// prevent a potential timeout because it could take a few milliseconds until the task will be scheduled
				subscriber->waitUntil = 0;
				subscriber->state = READY;

				copyMessageToTaskSpace( msg, subscriber, currentRunning );
				// receive will return 0 if message was received
				subscriber->regs[ 0 ] = 0;

				consumed = TRUE;
			}
		}
	}

	if ( FALSE == consumed )
	{
		if ( IS_MSG_QUEUE_FULL( channel ) )
		{
			return 1;
		}

		MESSAGE* copyMsg = &channel->msgQueue[ channel->msgQueueInsertPos ];
		memcpy( copyMsg, msg, sizeof( MESSAGE ) );

		channel->msgQueueInsertPos = NEXT_MSG_QUEUE_INSERTPOS( channel );
	}

	return 0;
}

int32_t
channel_waitForMessage( uint32_t channelId, Task* t, int32_t timeout )
{
	// NOTE: sleep is done using a receive with timeout on NULL-CHANNEL
	// NULL-Channel is the invalid channel, it is not opened/closed/subscribed
	// no send to it is possible but only a receive with a timeout
	if ( NULL_CHANNEL == channelId )
	{
		// no timeout specified, error, no wait, return immediately
		if ( timeout <= 0 )
		{
			return 1;
		}
	}
	else
	{
		bool subscriberFound = FALSE;

		IPC_CHANNEL* channel = getChannelById( channelId );
		if ( 0 == channel )
		{
			return 1;
		}

		// if a message is present, consume it
		if ( ! IS_MSG_QUEUE_EMPTY( channel ) )
		{
			MESSAGE* msg = &channel->msgQueue[ channel->msgQueueStartPos ];

			// receive has a pointer to the message to store as the 2nd parameter
			MESSAGE* targetMsgPtr = ( MESSAGE* ) t->regs[ 1 ];
			memcpy( targetMsgPtr, msg, sizeof( MESSAGE ) );

			// clean-up message
			channel->msgQueueStartPos = NEXT_MSG_QUEUE_STARTPOS( channel );

			// receive returns 0 when message received
			// NOTE: will be written to currentUserCtx because the call receive is from a currently running task
			return 0;
		}

		// no message present at this point

		// negative timeout specified: return immediately
		if ( 0 > timeout )
		{
			// receive returns -1 when no message received
			// NOTE: will be written to currentUserCtx because the call receive is from a currently running task
			return -1;
		}

		// timeout is >= 0: wait until message arrives or timeout hits

		// need to check if this Task is in subscribers if blocking forever, if not it won't get notified ever if a message arrives
		if ( 0 == timeout )
		{
			uint32_t i = 0;

			// NOTE: this is a security check only, can be removed after debugging
			for ( i = 0; i < MAX_SUBSCRIBERS; ++i )
			{
				Task* subscriber = channel->subscribers[ i ];
				if ( subscriber == t )
				{
					subscriberFound = TRUE;
					break;
				}
			}
		}
		else
		{
			subscriberFound = TRUE;
		}

		if ( FALSE == subscriberFound )
		{
			return 1;
		}
	}

	saveCurrentRunning( currentUserCtx );

	t->state = WAITING;
	t->waitUntil = ( 0 == timeout ) ? 0 : ( getSysMillis() + timeout );
	t->waitChannel = channelId;

	scheduleNextReady( currentUserCtx );

	return 0;
}

void
copyMessageToTaskSpace( MESSAGE* msg, Task* targetTask, Task* currentRunning )
{
	cpyMsg = *msg;

	mmu_ttbSet( ( int32_t ) targetTask->pageTable );
	mmu_setProcessID( targetTask->pid );

	// receive has a pointer to the message to store as the 2nd parameter
	MESSAGE* targetMsgPtr = ( MESSAGE* ) targetTask->regs[ 1 ];
	memcpy( targetMsgPtr, &cpyMsg, sizeof( MESSAGE ) );

	mmu_ttbSet( ( int32_t ) currentRunning->pageTable );
	mmu_setProcessID( currentRunning->pid );
}

IPC_CHANNEL*
getChannelById( uint32_t channelId )
{
	if ( channelId >= MAX_CHANNELS )
	{
		return 0;
	}

	if ( _channels[ channelId ].id == NULL_CHANNEL )
	{
		return 0;
	}

	return &_channels[ channelId ];
}
