/*
 * core.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include "core.h"

#include "../public/boss.h"

#include <stdarg.h>

#pragma INTERRUPT ( resetHandler, RESET );
#pragma INTERRUPT ( undefInstrHandler, UDEF );
#pragma INTERRUPT ( prefetchAbortHandler, PABT );
#pragma INTERRUPT ( dataAbortHandler, DABT );
#pragma INTERRUPT ( irqHandler, IRQ );
#pragma INTERRUPT ( fiqHandler, FIQ );

int32_t
initCore( void )
{
	// TODO: setup global kernel data-structure


	return 0;
}

interrupt
void resetHandler()
{
	// TODO: implement
}

interrupt
void undefInstrHandler()
{
	// TODO: implement
}

// NOTE: not marked with interrupt, applied different technique to handle SWI
uint32_t
swiHandler( uint32_t swiId, ... )
{
	uint32_t ret = 0;

	if ( SYSCALL_SEND_ID == swiId || SYSCALL_RECEIVE_ID == swiId || SYSCALL_SENDRCV_ID == swiId )
	{
		va_list ap;
		va_start( ap, swiId );

		uint32_t sendId = va_arg( ap, uint32_t );
		uint8_t* data = va_arg( ap, uint8_t* );
		uint8_t dataSize = va_arg( ap, uint8_t );

		if ( SYSCALL_SEND_ID == swiId )
		{
			ret = send( sendId, data, dataSize );
		}
		else if ( SYSCALL_RECEIVE_ID == swiId )
		{
			ret = receive( sendId, data, dataSize );
		}
		else if ( SYSCALL_SENDRCV_ID == swiId )
		{
			ret = sendrcv( sendId, data, dataSize );
		}

		va_end( ap );
	}

	return ret;
}

interrupt
void prefetchAbortHandler()
{
	// TODO: implement
}

interrupt
void dataAbortHandler()
{
	// TODO: implement
}

interrupt
void irqHandler()
{
	// TODO: implement
}

interrupt
void fiqHandler()
{
	// TODO: implement
}
