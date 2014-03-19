/*
 * core.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

// first include: header-file of the .c-file
#include "core.h"

// second includes: local includes
#include "../scheduler/scheduler.h"
#include "../timer/gptimer.h"

// third includes: project-includes
#include <boss.h>

// fourth includes: clib and system-includes
#include <stdarg.h>

#include "../common/common.h"

#pragma INTERRUPT ( undefInstrHandler, UDEF );
#pragma INTERRUPT ( prefetchAbortHandler, PABT );
#pragma INTERRUPT ( dataAbortHandler, DABT );
#pragma INTERRUPT ( fiqHandler, FIQ );

int32_t
initCore( void )
{
	// TODO: setup global kernel data-structure

	return 0;
}

interrupt
void undefInstrHandler()
{
	// TODO: implement
}

// NOTE: not marked with interrupt, applied different technique to handle SWI
int32_t
swiHandler( uint32_t swiId, uint32_t* regs )
{
	int32_t ret = 0;

	if ( SYSC_SEND == swiId || SYSC_RECEIVE == swiId || SYSC_SENDRCV == swiId )
	{
		uint8_t* data = ( uint8_t* ) regs[ 1 ];
		uint8_t dataSize = ( uint8_t ) regs[ 2 ];

		if ( SYSC_SEND == swiId )
		{
			ret = send( regs[ 0 ], data, dataSize );
		}
		else if ( SYSC_RECEIVE == swiId )
		{
			ret = receive( regs[ 0 ], data, dataSize );
		}
		else if ( SYSC_SENDRCV == swiId )
		{
			ret = sendrcv( regs[ 0 ], data, dataSize );
		}
	}
	else if ( SYSC_CREATETASK == swiId )
	{
		task_func entryPoint = ( task_func ) regs[ 0 ];

		ret = createTask( entryPoint );
	}
	else if ( SYSC_FORK == swiId )
	{
		ret = fork();
	}
	else if ( SYSC_SLEEP == swiId )
	{
		ret = sleep( regs[ 0 ] );
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

uint32_t
irqHandler( uint32_t irqNr, uint32_t* userCpsr, uint32_t* userPC, uint32_t* userRegs )
{
	uint32_t ret = 0;

	// clear interrupt flags
	reg32w( GPTIMER2_BASE, GP_TIMER_IT_FLAG, 0x7 );

	// TODO: make it more nice
	if ( 38 == ( irqNr & 127 ) )
	{
		ret = schedule( userCpsr, userPC, userRegs );
	}

	return ret;
}

interrupt
void fiqHandler()
{
	// TODO: implement
}
