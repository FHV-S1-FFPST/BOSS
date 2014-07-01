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
#include "../scheduler/scheduler.h"
#include "../ipc/channel.h"
#include "../task/taskTable.h"
#include "../fs/fat32/fat32.h"

// third includes: project-includes
#include <boss.h>

#include <stdlib.h>

#pragma INTERRUPT ( undefInstrHandler, UDEF );

#define SYSTIMER_OVERFLOW_INTERVAL_MS 0xFFFFFF

// module-local data //////////////////////////////////////////////
static uint64_t systemMillis;
///////////////////////////////////////////////////////////////////

// module-local functions /////////////////////////////////////////
static uint32_t handleSystemTimerOverflow( UserContext* ctx );
static int32_t getSysMillisSysCall( void );
///////////////////////////////////////////////////////////////////

int32_t
initCore( void )
{
	sysTimerInit( SYSTIMER_OVERFLOW_INTERVAL_MS );
	irqRegisterClbk( handleSystemTimerOverflow, GPT10_IRQ );

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
	return ( SystemState ) BIT_CLEAR( cpsr, ~0x1F );
}

// NOTE: not marked with interrupt, applied different technique to handle SWI
void
swiHandler( uint32_t swiId, UserContext* ctx )
{
	currentUserCtx = ctx;

	if ( SYSC_SEND == swiId || SYSC_RECEIVE == swiId || SYSC_SENDRCV == swiId )
	{
		MESSAGE* msg = ( MESSAGE* ) ctx->regs[ 1 ];

		if ( SYSC_SEND == swiId )
		{
			ctx->regs[ 0 ] = channel_receivesMessage( ctx->regs[ 0 ], msg, getTask( getCurrentPid() ) );
		}
		else if ( SYSC_RECEIVE == swiId )
		{
			ctx->regs[ 0 ] = channel_waitForMessage( ctx->regs[ 0 ], getTask( getCurrentPid() ), ctx->regs[ 2 ] );
		}
		else if ( SYSC_SENDRCV == swiId )
		{
			if ( channel_receivesMessage( ctx->regs[ 0 ], msg, getTask( getCurrentPid() ) ) )
			{
				ctx->regs[ 0 ] = 1;
			}
			else
			{
				ctx->regs[ 0 ] = channel_waitForMessage( ctx->regs[ 0 ], getTask( getCurrentPid() ), ctx->regs[ 2 ] );
			}
		}
	}
	else if ( SYSC_CH_OPEN == swiId )
	{
		uint32_t ret = channel_open( ctx->regs[ 0 ] );
		if ( 0 == ret )
		{
			// subscribe owner of the new channel to its channel, will always succeed
			channel_subscribe( ctx->regs[ 0 ], getTask( getCurrentPid() ) );
		}

		ctx->regs[ 0 ] = ret;
	}
	else if ( SYSC_CH_CLOSE == swiId )
	{
		ctx->regs[ 0 ] = channel_close( ctx->regs[ 0 ] );
	}
	else if ( SYSC_CH_SUBSCRIBE == swiId )
	{
		ctx->regs[ 0 ] = channel_subscribe( ctx->regs[ 0 ], getTask( getCurrentPid() ) );
	}
	else if ( SYSC_SYSMILLIS == swiId )
	{
		getSysMillisSysCall();
	}
	else if ( SYSC_PID == swiId )
	{
		getPid();
	}
	else if ( SYSC_EXIT_TASK == swiId )
	{
		exitTask( ctx->regs[ 0 ] );
	}
	else if ( SYSC_READ_REG == swiId )
	{
		uint32_t value = READ_REGISTER( ctx->regs[ 0 ] );
		ctx->regs[ 0 ] = value;
	}
	else if ( SYSC_WRITE_REG == swiId )
	{
		READ_REGISTER( ctx->regs[ 0 ] ) = ctx->regs[ 1 ];
	}
	else if ( SYSC_FOPEN == swiId )
	{
		file_id fileId;

		ctx->regs[ 0 ] = fat32Open( ( const char* ) ctx->regs[ 0 ], &fileId );
		ctx->regs[ 1 ] = fileId;
	}
	else if ( SYSC_FCLOSE == swiId )
	{
		ctx->regs[ 0 ] = fat32Close( ( file_id ) ctx->regs[ 0 ] );
	}
	else if ( SYSC_FSIZE == swiId )
	{
		ctx->regs[ 0 ] = fat32Size( ( file_id ) ctx->regs[ 0 ], ( uint32_t* ) &ctx->regs[ 1 ] );
	}
	else if ( SYSC_FREAD == swiId )
	{
		ctx->regs[ 0 ] = fat32Read( ( file_id ) ctx->regs[ 0 ], ctx->regs[ 1 ], ( uint8_t* ) ctx->regs[ 2 ] );
	}
	else if ( SYSC_ALLOCSHAREDMEM == swiId )
	{
		// NOTE: OS memory is shared by virtual memory, so this is a fake shared memory implementation
		ctx->regs[ 0 ] = ( uint32_t ) malloc( ctx->regs[ 0 ] );
	}
	else if ( SYSC_FREESHAREDMEM == swiId )
	{
		free( ( void* ) ctx->regs[ 0 ] );
	}
}

uint32_t
dataAbortHandler( uint32_t faultStatusReg, uint32_t faultAddress )
{
	//uint32_t faultStatus = faultStatusReg & 0xF;
	//uint32_t domain = ( faultStatusReg >> 4 ) & 0xF;

	// TODO: ABORT-STACK seems to be not present in linkage, fix it

	// NOTE: return 0 when no repeating of last instruction is needed,  otherwise return 1
	return 0;
}

void
prefetchAbortHandler()
{
	// implement if necessary
}

interrupt
void undefInstrHandler()
{
	// implement if necessary
}

int32_t
getSysMillisSysCall( void )
{
	uint64_t sysMillis = getSysMillis();

	// NOTE: because sysMillis is a 64bit wide we need to distribute it over R0 and R1
	currentUserCtx->regs[ 0 ] = sysMillis & 0xFFFFFFFF;
	currentUserCtx->regs[ 1 ] = ( sysMillis >> 31 ) & 0xFFFFFFFF;

	return 0;
}

int32_t
getPid( void )
{
	currentUserCtx->regs[ 0 ] = getCurrentPid();

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
