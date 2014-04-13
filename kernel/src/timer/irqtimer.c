/*
 * irqtimer.c
 *
 *  Created on: 12.04.2014
 *      Author: Jonathan Thaler
 */

#include "irqtimer.h"

#include "../hal/timer/timer_hal.h"

#define IRQTIMER_ADDR GPTIMER2_ADDR

void
irqTimerInit( uint32_t irqIntervalMs )
{
	// stop timer, maybe was started bevore reset
	irqTimerStop();

	// TODO: disable post-mode
	// TODO: disable prescaler
	// clear all pending interrupts before reset
	halTimerClearAllInterrupts( IRQTIMER_ADDR );
	// enable match-interrupt
	halTimerEnableInterrupt( IRQTIMER_ADDR, MATCH_IT_BIT );
	// compare value which resembles the milliseconds after which the match-interrupt should be signaled
	halTimerSetCompareValue( IRQTIMER_ADDR, irqIntervalMs * HAL_TIMER_TICKS_PER_MS );
	// enable compare
	halTimerEnableCompare( IRQTIMER_ADDR );
	// always reset to 0x0 after match
	halTimerSetLoadValue( IRQTIMER_ADDR, 0x0 );
	// reload
	halTimerEnableAutoReload( IRQTIMER_ADDR );

	// reset
	irqTimerReset();
}

void
irqTimerReset( void )
{
	halTimerReset( IRQTIMER_ADDR );
}

void
irqTimerStart( void )
{
	halTimerStart( IRQTIMER_ADDR );
}

void
irqTimerStop( void )
{
	halTimerStop( IRQTIMER_ADDR );
}
