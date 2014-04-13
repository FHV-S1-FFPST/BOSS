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
#define LOADVALUE_1MSTICK_VALUE 0xFFFFFFE0

void
sysTimerInit( uint32_t ovfAfterMs )
{
	// stop timer, maybe was started bevore reset
	halTimerStop( SYSTIMER_ADDR );
	// TODO: disable post-mode
	// TODO: disable prescaler
	// clear all pending interrupts before reset
	halTimerClearAllInterrupts( SYSTIMER_ADDR );

	// NOTE: config for 1ms tick (see OMAP35x.pdf page 2625 )
	halTimerSetPosInc( SYSTIMER_ADDR, POSINC_1MSTICK_VALUE );
	halTimerSetNegInc( SYSTIMER_ADDR, NEGINC_1MSTICK_VALUE );
	halTimerSetLoadValue( SYSTIMER_ADDR, LOADVALUE_1MSTICK_VALUE );

	// NOTE: the timer will now generate overflows every 1ms thus we
	// get an accurate 1ms timer by counting the overflows. this is done
	// by enabling the overflow interrupt and specifying a value for the
	// TOWR-register (see page 2625 in OMAP35x.pdf).
	halTimerEnableInterrupt( SYSTIMER_ADDR, OVERFLOW_IT_BIT );
	// enable overflow
	halTimerEnableOverflow( SYSTIMER_ADDR );
	// NOTE: after ovfAfterMs milliseconds an interrupt will be generated
	// which needs to be handled.
	halTimerSetOvfWrapping( SYSTIMER_ADDR, ovfAfterMs );
	// reset the overflow-counter register to start with 0
	halTimerSetOvfMaskValue( SYSTIMER_ADDR, 0x0 );

	// reload value after overflow
	halTimerEnableAutoReload( SYSTIMER_ADDR );
	// reset timer-value to reload value
	halTimerReset( SYSTIMER_ADDR );
	// start it immediately
	halTimerStart( SYSTIMER_ADDR );
}

uint32_t
sysTimerValue( void )
{
	// NOTE:
	return halTimerOvfMaskValue( SYSTIMER_ADDR );
}
