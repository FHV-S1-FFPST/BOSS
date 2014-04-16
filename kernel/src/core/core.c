/*
 * core.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

// first include: header-file of the .c-file
#include "core.h"

// second includes: local includes
#include "../timer/systimer.h"
#include "../common/common.h"
#include "../irq/irq.h"

// third includes: project-includes
#include <boss.h>

#pragma INTERRUPT ( undefInstrHandler, UDEF );
#pragma INTERRUPT ( prefetchAbortHandler, PABT );
#pragma INTERRUPT ( dataAbortHandler, DABT );

#define SYSTIMER_OVERFLOW_INTERVAL_MS 0xFFFFFF

// module-local data //////////////////////////////////////////////
static uint64_t systemMillis;
// module-local functions /////////////////////////////////////////
static uint32_t handleSystemTimerOverflow( UserContext* ctx );
static uint32_t getSysMillisSysCall( void );
///////////////////////////////////////////////////////////////////

int32_t
initCore( void )
{
	sysTimerInit( SYSTIMER_OVERFLOW_INTERVAL_MS );
	irqRegisterClbk( handleSystemTimerOverflow, GPT10_IRQ );

	// TODO: this shouldnt be necessary anymore because this is handled inside timer
	// NOTE: need to waste some time, otherwise IRQ won't hit
	volatile uint32_t i = 100000;
	while ( i > 0 )
		--i;

	return 0;
}

uint64_t
getSysMillis( void )
{
	return systemMillis + sysTimerValue();
}

SystemState
querySystemState( void )
{
	uint32_t cpsr = _get_CPSR();
	// TODO: use reg32* macros
	return BIT_CLEAR( cpsr, ~0x1F );
}

// NOTE: not marked with interrupt, applied different technique to handle SWI
int32_t
swiHandler( uint32_t swiId, UserContext* ctx )
{
	int32_t ret = 0;

	currentUserCtx = ctx;

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
	else if ( SYSC_SYSMILLIS == swiId )
	{
		ret = getSysMillisSysCall();
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

uint32_t
getSysMillisSysCall( void )
{
	uint64_t sysMillis = getSysMillis();

	// NOTE: because sysMillis is a 64bit wide we need to distribute it over R0 and R1
	currentUserCtx->regs[ 0 ] = sysMillis & 0xFFFFFFFF;
	currentUserCtx->regs[ 1 ] = ( sysMillis >> 31 ) & 0xFFFFFFFF;

	return 0;
}


uint32_t
handleSystemTimerOverflow( UserContext* ctx )
{
	// NOTE: every SYSTIMER_OVERFLOW_INTERVAL_MS milliseconds the system-timer will
	// overflow. The system-timer is driven by the TOCR register which is a 24-bit
	// register so SYSTIMER_OVERFLOW_INTERVAL_MS will be most likely set to 0x00FFFFFF
	// thus after 0x00FFFFFF ms an overflow will be generated which allows the
	// global system-milliseconds to be incremented by SYSTIMER_OVERFLOW_INTERVAL_MS
	// so it is possible to maintain a uint64_t systemmilliseconds value
	systemMillis += SYSTIMER_OVERFLOW_INTERVAL_MS;

	sysTimerResetInterrupt();

	return 0;
}
