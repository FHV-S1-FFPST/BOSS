/*
 * channel.c
 *
 *  Created on: 03.06.2014
 *      Author: Jonathan Thaler
 */


#include "channel.h"

#include <stdlib.h>
#include <string.h>

// module-local defines
#define MAX_CHANNELS 				16
#define MESSAGE_QUEUE_LENGTH 		32
#define MAX_SUBSCRIBERS		 		32
////////////////////////////////////////////////////////

// module-local data-structures
typedef struct
{
	uint32_t id;

	Task* subscribers[ MAX_SUBSCRIBERS ];

	MESSAGE* msgQueue[ MESSAGE_QUEUE_LENGTH ];
} IPC_CHANNEL;
////////////////////////////////////////////////////////

// module-local functions
static IPC_CHANNEL* getChannelById( uint32_t channelId );
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
	channel->id = channelId;
	memset( channel->msgQueue, 0, sizeof( channel->msgQueue ) );

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
channel_attach( uint32_t channelId, Task* task )
{
	uint32_t i = 0;

	IPC_CHANNEL* channel = getChannelById( channelId );
	if ( 0 == channel )
	{
		return 1;
	}

	for ( i = 0; i < MAX_SUBSCRIBERS; ++i )
	{
		if ( 0 == channel->subscribers[ i ] )
		{
			channel->subscribers[ i ] = task;
			return 0;
		}
	}

	return 1;
}

uint32_t
channel_receivesMessage( uint32_t channelId, MESSAGE* msg )
{
	uint32_t i = 0;

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

				// TODO: copy message to tasks space
				// TODO: set return registers of sys-call
			}
		}
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

	t->state = WAITING;
	t->waitUntil = getSysMillis() + timeout;

	// TODO: schedule next running

	return 0;
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
