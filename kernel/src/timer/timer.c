/*
 * timer.c
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#include "timer.h"

#include "../common/common.h"

#define HAL_TIMER_TICKS_PER_MS 750

void
timerInit( uint32_t timerId, uint32_t intervalMs )
{
	INTC_MIRCLEAR_GTP |= UNMASK_GPT_INTERRUPT( timerId ); 		//unmask interrupt

	timerStop( timerId );
	timerReset( timerId );

	TIER_GPT( timerId ) |= 0x01; 								//enable interrupts for given gpt
	TMAR_GPT( timerId ) = intervalMs * HAL_TIMER_TICKS_PER_MS; 	//set compare value
	TCLR_GPT( timerId ) |= HAL_TIMER_CE_GPT; 					//Enable Compare mode
	TLDR_GPT( timerId ) = 0x0; 									//set resetvalue to 0

	TCLR_GPT( timerId ) |= HAL_TIMER_AR_GPT; 					//autoreload
}

void
timerStart( uint32_t timerId )
{
	TCLR_GPT( timerId ) |= HAL_TIMER_ON_GPT;
}

void
timerStop( uint32_t timerId )
{
	TCLR_GPT( timerId ) &= ~HAL_TIMER_ON_GPT;
	volatile uint32_t* ptr = TCLR_GPT( timerId );

	*ptr &= ~HAL_TIMER_ON_GPT;

	AWAIT_BITS_CLEARED( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
}

void
timerReset( uint32_t timerId )
{
	TTGR_GPT( timerId ) = 0x00; //ResetTimer Value
	TISR_GPT( timerId ) |= HAL_TIMER_CLEAR_INTERRUPT; //Clear Interrupt bits
}

void
timerSetInterruptMode( uint32_t timerId )
{
	INTC_ILR_GTP( timerId ) |= 0x1;
}

