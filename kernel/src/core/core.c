/*
 * core.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

// first include: header-file of the .c-file
#include "core.h"

// second includes: local includes

// third includes: project-includes
#include <boss.h>

// fourth includes: clib and system-includes
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
int32_t
swiHandler( uint32_t swiId, ... )
{
	int32_t ret = 0;

	if ( SYSC_SEND == swiId || SYSC_RECEIVE == swiId || SYSC_SENDRCV == swiId )
	{
		va_list ap;
		va_start( ap, swiId );

		uint32_t sendId = va_arg( ap, uint32_t );
		uint8_t* data = va_arg( ap, uint8_t* );
		uint8_t dataSize = va_arg( ap, uint8_t );

		if ( SYSC_SEND == swiId )
		{
			ret = send( sendId, data, dataSize );
		}
		else if ( SYSC_RECEIVE == swiId )
		{
			ret = receive( sendId, data, dataSize );
		}
		else if ( SYSC_SENDRCV == swiId )
		{
			ret = sendrcv( sendId, data, dataSize );
		}

		va_end( ap );
	}
	else if ( SYSC_CREATEPROC == swiId )
	{
		ret = createProcess();
	}
	else if ( SYSC_FORK == swiId )
	{
		ret = fork();
	}
	else
	{
		ret = -1;
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
