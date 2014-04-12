/*
 * timer.c
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#include "timer.h"

#include "../common/common.h"


#define TIDR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x000 ) ) )	// R
#define TIOCP_CFG_GPT( addr )	( *( ( volatile uint32_t* ) ( addr + 0x010 ) ) )	// RW
#define TISTAT_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x014 ) ) )	// R
#define TISR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x018 ) ) )	// RW
#define TIER_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x01C ) ) )	// RW
#define TWER_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x020 ) ) )	// RW
#define TCLR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x024 ) ) )	// RW
#define TCRR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x028 ) ) )	// RW
#define TLDR_GPT( addr ) 		( *( ( volatile uint32_t* ) ( addr + 0x02C ) ) )	// RW
#define TTGR_GPT( addr ) 		( *( ( volatile uint32_t* ) ( addr + 0x030 ) ) )	// RW
#define TWPS_GPT( addr ) 		( *( ( volatile uint32_t* ) ( addr + 0x034 ) ) )	// R
#define TMAR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x038 ) ) )	// RW
#define TCAR1_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x03C ) ) )	// R
#define TSICR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x040 ) ) )	// RW
#define TCAR2_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x044 ) ) )	// R
#define TPIR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x048 ) ) )	// RW
#define TNIR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x04C ) ) )	// RW
#define TCVR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x050 ) ) )	// RW
#define TOCR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x054 ) ) )	// RW
#define TOWR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x058 ) ) )	// RW

#define INTC_ILR_GTP(number)      ( *((volatile uint32_t*) ((long)0x48200100 + 0x4 * (36 + number))))        // 0x4820 01000 + 0x98 (0x04*38)

#define INTC_MIRCLEAR_GTP (*((volatile uint32_t*) 0x482000A8))     //0x4820 0088 + (0x20 * 1)
#define INTCPS_CONTROL (*((volatile uint32_t*) 0x48200048))

#define HAL_TIMER_CE_GPT                              0x40
#define HAL_TIMER_ON_GPT                              0x01
#define HAL_TIMER_AR_GPT                              0x02
#define HAL_TIMER_CLEAR_INTERRUPT                     0x07

#define UNMASK_GPT_INTERRUPT(number)    0x10 << number

#define HAL_TIMER_TICKS_PER_MS 1000

void
timerInit( uint32_t timerId, uint32_t intervalMs )
{
	// TODO: read chapter 16.2.6.1 about writing to registers in OMAP35x.pdf page 2630

	INTC_MIRCLEAR_GTP |= UNMASK_GPT_INTERRUPT( timerId ); 		//unmask interrupt

	timerStop( timerId );
	timerReset( timerId );

	// TODO: use reg32* macros
	BIT_SET( TIER_GPT( timerId ), 0x1 );						//enable interrupts for given gpt
	TMAR_GPT( timerId ) = intervalMs * HAL_TIMER_TICKS_PER_MS; 	//set compare value
	BIT_SET( TCLR_GPT( timerId ), HAL_TIMER_CE_GPT );			//Enable Compare mode
	BIT_CLEAR_ALL( TLDR_GPT( timerId ) );						//set resetvalue to 0
	BIT_SET( TCLR_GPT( timerId ), 0x1 );						//autoreload
}

void
timerStart( uint32_t timerId )
{
	// TODO: use reg32* macros
	BIT_SET( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
	AWAIT_BITS_SET( TCLR_GPT( timerId ), HAL_TIMER_ON_GPT );
}

void
timerStop( uint32_t timerId )
{
	// TODO: use reg32* macros
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
	// TODO: use reg32* macros
	BIT_SET( INTC_ILR_GTP( timerId ), 0x1 );
	AWAIT_BITS_SET( INTC_ILR_GTP( timerId ), 0x1 );
}
