/*
 * systimer.c
 *
 *  Created on: 12.04.2014
 *      Author: Jonathan Thaler
 */

#include "systimer.h"

#include "../hal/timer/timer_hal.h"

#define IRQTIMER_ADDR GPTIMER10_ADDR

void
sysTimerInit( void )
{
	// TODO: initialize system timer
}

uint32_t
sysTimerValue( void )
{
	// TODO: return timer-value which resembles the milliseconds
	return 0;
}
