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
#include "../timer/timer.h"
#include "../common/common.h"

// third includes: project-includes
#include <boss.h>

// fourth includes: clib and system-includes
#include <stdarg.h>

// TODO: move to another include
#define INTCPS_SIR_IRQ_ADDR 	0x48200040

#define INTC_CONTROL 			0x48
#define INTC_CONTROL_NEWIRQAGR	0x00000001
#define SOC_AINTC_REGS			0x48200000

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

SystemState
querySystemState( void )
{
	uint32_t cpsr = _get_CPSR();
	return BIT_CLEAR( cpsr, ~0x1F );
}

// NOTE: not marked with interrupt, applied different technique to handle SWI
int32_t
swiHandler( uint32_t swiId, UserContext* ctx )
{
	int32_t ret = 0;

	if ( SYSC_SEND == swiId || SYSC_RECEIVE == swiId || SYSC_SENDRCV == swiId )
	{
		uint8_t* data = ( uint8_t* ) ctx->regs[ 1 ];
		uint8_t dataSize = ( uint8_t ) ctx->regs[ 2 ];

		if ( SYSC_SEND == swiId )
		{
			ret = send( ctx->regs[ 0 ], data, dataSize );
		}
		else if ( SYSC_RECEIVE == swiId )
		{
			ret = receive( ctx->regs[ 0 ], data, dataSize );
		}
		else if ( SYSC_SENDRCV == swiId )
		{
			ret = sendrcv( ctx->regs[ 0 ], data, dataSize );
		}
	}
	else if ( SYSC_CREATETASK == swiId )
	{
		task_func entryPoint = ( task_func ) ctx->regs[ 0 ];

		ret = createTask( entryPoint );
	}
	else if ( SYSC_FORK == swiId )
	{
		ret = fork();
	}
	else if ( SYSC_SLEEP == swiId )
	{
		ret = sleep( ctx->regs[ 0 ] );
	}
	else
	{
		ret = -1;
	}

	return ret;
}

uint32_t
irqHandler( UserContext* ctx )
{
	uint32_t ret = 0;
	// fetch irqNr
	uint32_t irqNr = *( ( uint32_t* ) INTCPS_SIR_IRQ_ADDR );

	// TODO: use func-pointers to jump to irqNr instead of branching

	// TODO: make it more nice
	if ( 38 == ( irqNr & 127 ) )
	{
		ret = schedule( ctx );
	}

	// TODO: use func-pointers to jump to reset-handlers, to decouple reset-functionality from irq functionality

	// reset and clear timer interrupt flags
	timerReset( TIMER2_ID );

	// reset IRQ-interrupt flag
	reg32m( SOC_AINTC_REGS, INTC_CONTROL, INTC_CONTROL_NEWIRQAGR );

	return ret;
}

interrupt
void prefetchAbortHandler()
{
	// implement when necessary
}

interrupt
void dataAbortHandler()
{
	// implement when necessary
}

interrupt
void undefInstrHandler()
{
	// implement when necessary
}

interrupt
void fiqHandler()
{
	// implement when necessary
}
