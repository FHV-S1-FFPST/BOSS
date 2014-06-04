/*
 * ipc.c
 *
 *  Created on: 05.03.2014
 *      Author: Thaler
 */

#include "ipc.h"

#include "channel.h"
#include "../task/tasktable.h"
#include "../scheduler/scheduler.h"

int32_t
channelOpen( uint32_t channelId )
{
	return channel_open( channelId );
}

int32_t
channelClose( uint32_t channelId )
{
	return channel_close( channelId );
}

int32_t
channelSubscribe( uint32_t channelId )
{
	Task* t = getTask( getCurrentPid() );

	return channel_subscribe( channelId, t );
}

int32_t
send( uint32_t channelId, MESSAGE* msg )
{
	return channel_receivesMessage( channelId, msg );
}

int32_t
receive( uint32_t channelId, MESSAGE* msg, int32_t timeout )
{
	Task* t = getTask( getCurrentPid() );

	return channel_waitForMessage( channelId, t, timeout );
}

int32_t
sendrcv( uint32_t channelId, MESSAGE* msg, uint32_t timeout )
{
	Task* t = getTask( getCurrentPid() );

	if ( channel_receivesMessage( channelId, msg ) )
	{
		return 1;
	}

	return channel_waitForMessage( channelId, t, timeout );
}
