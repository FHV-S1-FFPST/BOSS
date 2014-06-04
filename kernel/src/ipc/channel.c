/*
 * channel.c
 *
 *  Created on: 03.06.2014
 *      Author: Jonathan Thaler
 */


#include "channel.h"

#include "../common/common.h"
#include "../scheduler/scheduler.h"

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
	uint32_t id;

	int32_t subscribersStartPos;
	int32_t subscribersInsertPos;
	Task* subscribers[ MAX_SUBSCRIBERS ];

	int32_t msgQueueStartPos;
	int32_t msgQueueInsertPos;
	MESSAGE* msgQueue[ MESSAGE_QUEUE_LENGTH ];
} IPC_CHANNEL;
////////////////////////////////////////////////////////

// module-local functions
static IPC_CHANNEL* getChannelById( uint32_t channelId );
static void copyMessageToTaskSpace( MESSAGE* msg, Task* t );
////////////////////////////////////////////////////////

// module-local data
static IPC_CHANNEL* _channels[ MAX_CHANNELS ];
////////////////////////////////////////////////////////

uint32_t
channel_open( uint32_t channelId )
{
	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 != channel )
	{
		return 1;
	}

	channel = malloc( sizeof( IPC_CHANNEL ) );
	memset( channel, 0, sizeof( IPC_CHANNEL ) );
	channel->id = channelId;

	_channels[ channel->id ] = channel;

	return 0;
}

uint32_t
channel_close( uint32_t channelId )
{
	uint32_t i = 0;

	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 == channel )
	{
		return 1;
	}

	for ( i = 0; i < MESSAGE_QUEUE_LENGTH; ++i )
	{
		MESSAGE* msg = channel->msgQueue[ i ];
		if ( msg )
		{
			free( msg );
		}
	}

	free( channel );

	_channels[ channelId ] = 0;

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

	return 1;
}

uint32_t
channel_receivesMessage( uint32_t channelId, MESSAGE* msg )
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

				copyMessageToTaskSpace( msg, subscriber );
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

		MESSAGE* copyMsg = malloc( sizeof( MESSAGE ) );
		memcpy( copyMsg, msg, sizeof( MESSAGE ) );

		channel->msgQueue[ channel->msgQueueInsertPos ] = copyMsg;
		channel->msgQueueInsertPos = NEXT_MSG_QUEUE_INSERTPOS( channel );
	}

	return 0;
}

uint32_t
channel_waitForMessage( uint32_t channelId, Task* t, int32_t timeout )
{
	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 == channel )
	{
		return 1;
	}

	// negative timeout specified: only check for messages and if none present, return immediately
	if ( 0 > timeout )
	{
		if ( IS_MSG_QUEUE_EMPTY( channel ) )
		{
			// receive returns -1 when no message received
			// NOTE: use currentUserCtx because the call receive is from a currently running task
			currentUserCtx->regs[ 0 ] = -1;
		}
		else
		{
			MESSAGE* msg = channel->msgQueue[ channel->msgQueueStartPos ];

			copyMessageToTaskSpace( msg, t );
			// receive returns 0 when message received
			// NOTE: use currentUserCtx because the call receive is from a currently running task
			currentUserCtx->regs[ 0 ] = 0;

			// clean-up message
			free( msg );
			channel->msgQueue[ channel->msgQueueStartPos ] = 0;
			channel->msgQueueStartPos = NEXT_MSG_QUEUE_STARTPOS( channel );
		}

		return 0;
	}

	// at this state receive is blocking: either with timeout or blocking forever

	// TODO: need to check if this Task is in subscribers, if not it wont get notified ever if a message arrives

	t->state = WAITING;
	t->waitUntil = ( 0 == timeout ) ? 0 : ( getSysMillis() + timeout );
	t->waitChannel = channel->id;

	schedule( currentUserCtx );

	return 0;
}

void
copyMessageToTaskSpace( MESSAGE* msg, Task* t )
{
	// receive has a pointer to the message to store as the 2nd parameter
	MESSAGE* targetMsgPtr = currentUserCtx->regs[ 1 ];
	targetMsgPtr->id = targetMsgPtr->id;

	// TODO: implement, using currentUserCtx
}

IPC_CHANNEL*
getChannelById( uint32_t channelId )
{
	if ( channelId >= MAX_CHANNELS )
	{
		return 0;
	}

	return _channels[ channelId ];
}
