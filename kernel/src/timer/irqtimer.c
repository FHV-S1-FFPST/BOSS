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
	halTimerStop( IRQTIMER_ADDR );
	halTimerClearAllInterrupts( IRQTIMER_ADDR );
	halTimerEnableInterrupt( IRQTIMER_ADDR, MATCH_IT_BIT );
	halTimerSetCompareValue( IRQTIMER_ADDR, irqIntervalMs * HAL_TIMER_TICKS_PER_MS );
	halTimerEnableCompare( IRQTIMER_ADDR );
	halTimerSetLoadValue( IRQTIMER_ADDR, 0x0 );
	halTimerEnableAutoReload( IRQTIMER_ADDR );
	halTimerReset( IRQTIMER_ADDR );
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
