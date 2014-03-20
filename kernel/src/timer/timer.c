/*
 * timer.c
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#include "timer.h"

#include "../common/common.h"

#define HAL_TIMER_TICKS_PER_MS 100000

void
timerInit( uint32_t timerId, uint32_t intervalMs )
{
	INTC_MIRCLEAR_GTP |= UNMASK_GPT_INTERRUPT( timerId ); 		//unmask interrupt

	timerStop( timerId );
	timerReset( timerId );

	BIT_SET( TIER_GPT( timerId ), 0x1 );						//enable interrupts for given gpt
	TMAR_GPT( timerId ) = intervalMs * HAL_TIMER_TICKS_PER_MS; 	//set compare value
	BIT_SET( TCLR_GPT( timerId ), HAL_TIMER_CE_GPT );			//Enable Compare mode
	BIT_CLEAR_ALL( TLDR_GPT( timerId ) );						//set resetvalue to 0
	BIT_SET( TCLR_GPT( timerId ), 0x1 );						//autoreload
}

void
timerStart( uint32_t timerId )
{
	BIT_SET( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
	AWAIT_BITS_SET( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
}

void
timerStop( uint32_t timerId )
{
	BIT_CLEAR( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
	AWAIT_BITS_CLEARED( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
}

void
timerReset( uint32_t timerId )
{
	//ResetTimer Value, just write once to it, no need to do AWAIT_*
	BIT_CLEAR_ALL( TTGR_GPT( timerId ) );

	// set Interrupt bits, reads return 0, so no need to do AWAIT_*
	BIT_SET( TISR_GPT( timerId ), HAL_TIMER_CLEAR_INTERRUPT );
}

void
timerSetInterruptMode( uint32_t timerId )
{
	BIT_SET( INTC_ILR_GTP( timerId ), 0x1 );
	AWAIT_BITS_SET( INTC_ILR_GTP( timerId ), 0x1 );
}
