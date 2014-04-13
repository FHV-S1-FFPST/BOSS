/*
 * systimer.c
 *
 *  Created on: 12.04.2014
 *      Author: Jonathan Thaler
 */

#include "systimer.h"

#include "../hal/timer/timer_hal.h"

#define SYSTIMER_ADDR GPTIMER10_ADDR

#define POSINC_1MSTICK_VALUE 232000
#define NEGINC_1MSTICK_VALUE -768000

void
sysTimerInit( void )
{
	halTimerStop( SYSTIMER_ADDR );

	// TODO: maybe it is necessary to set the timer-input clock to 32khz instead of system-clock, see OMAP35x for it

	// NOTE: config for 1ms tick (see OMAP35x.pdf page 2625 )
	// overflow will lead to autoreload and incrementing of TOCR register => every 1ms this will be incremented by one so it counts the MS
	// TODO: need to handle the overflow of the TOCR because its only 24bit
	halTimerSetPosInc( SYSTIMER_ADDR, POSINC_1MSTICK_VALUE );
	halTimerSetNegInc( SYSTIMER_ADDR, NEGINC_1MSTICK_VALUE );
	halTimerSetLoadValue( SYSTIMER_ADDR, 0xFFFFFFE0 );

	halTimerEnableAutoReload( SYSTIMER_ADDR );
	halTimerReset( SYSTIMER_ADDR );

	halTimerStart( SYSTIMER_ADDR );
}

uint32_t
sysTimerValue( void )
{
	// NOTE:
	return halTimerOvfMaskValue( SYSTIMER_ADDR );
}
