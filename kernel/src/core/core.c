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
swiHandler( uint32_t swiId, uint32_t* swiFuncParams )
{
	// TODO: need process-control block struct: (userCpsr, userPc and userRegs)

	int32_t ret = 0;

	if ( SYSC_SEND == swiId || SYSC_RECEIVE == swiId || SYSC_SENDRCV == swiId )
	{
		uint8_t* data = ( uint8_t* ) swiFuncParams[ 1 ];
		uint8_t dataSize = ( uint8_t ) swiFuncParams[ 2 ];

		if ( SYSC_SEND == swiId )
		{
			ret = send( swiFuncParams[ 0 ], data, dataSize );
		}
		else if ( SYSC_RECEIVE == swiId )
		{
			ret = receive( swiFuncParams[ 0 ], data, dataSize );
		}
		else if ( SYSC_SENDRCV == swiId )
		{
			ret = sendrcv( swiFuncParams[ 0 ], data, dataSize );
		}
	}
	else if ( SYSC_CREATETASK == swiId )
	{
		task_func entryPoint = ( task_func ) swiFuncParams[ 0 ];

		ret = createTask( entryPoint );
	}
	else if ( SYSC_FORK == swiId )
	{
		ret = fork();
	}
	else if ( SYSC_SLEEP == swiId )
	{
		ret = sleep( swiFuncParams[ 0 ] );
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
	// TODO: fetch irqNr in C-code
	// TODO: move process-control block to a struct

	uint32_t ret = 0;

	// TODO: make it more nice
	if ( 38 == ( irqNr & 127 ) )
	{
		ret = schedule( userCpsr, userPC, userRegs );
	}

	// TODO: set IRQ-interrupt flag

	// clear interrupt flags
	reg32w( GPTIMER2_BASE, GP_TIMER_IT_FLAG, 0x7 );

	return ret;
}

interrupt
void fiqHandler()
{
	// TODO: implement
}
