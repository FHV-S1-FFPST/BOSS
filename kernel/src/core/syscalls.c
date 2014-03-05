/*
 * syscalls.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include <boss.h>

int32_t
send( uint32_t channelId, uint8_t* data, uint8_t dataSize )
{
	// TODO: implement

	return 31;
}

int32_t
receive( uint32_t channelId, uint8_t* data, uint8_t dataSize )
{
	// TODO: implement

	return 32;
}

int32_t
sendrcv( uint32_t channelId, uint8_t* data, uint8_t dataSize )
{
	// TODO: implement

	return 33;
}

int32_t
createProcess( proc_func entryPoint )
{
	return 41;
}

int32_t
fork()
{
	return 42;
}

int32_t
sleep( uint32_t millis )
{
	return 43;
}
