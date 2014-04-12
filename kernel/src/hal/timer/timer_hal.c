/*
 * timer.c
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#include "timer_hal.h"

// include bitset functions
#include "../../common/common.h"

// REGISTERS //////////////////////////////////////////////////////////////////////////////
// NOTE: this information can be found in chapter 16 in OMAP35x.pdf page 2609
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
// the following registers are avilable in GPTIMER 1,2 & 10 only
#define TPIR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x048 ) ) )	// RW
#define TNIR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x04C ) ) )	// RW
#define TCVR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x050 ) ) )	// RW
#define TOCR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x054 ) ) )	// RW
#define TOWR_GPT( addr )		( *( ( volatile uint32_t* ) ( addr + 0x058 ) ) )	// RW
//////////////////////////////////////////////////////////////////////////////////////////

// SPECIAL REGISTER BITS ////////////////////////////////////////////////////////////////
#define TCLR_ONOFF_BIT			0x01
#define TCLR_COMPARE_BIT		0x40
#define TCLR_AUTORELOAD_BIT		0x02

#define TISR_ALLIT_BITS			0x07
//////////////////////////////////////////////////////////////////////////////////////////

// TODO: remove from timer_hal
#define INTC_MIRCLEAR_GTP (*((volatile uint32_t*) 0x482000A8))     //0x4820 0088 + (0x20 * 1)
#define UNMASK_GPT_INTERRUPT(number)    0x10 << number

void
halTimerInit( uint32_t addr, uint32_t intervalMs )
{
	// TODO: remove from timer_hal
	INTC_MIRCLEAR_GTP |= UNMASK_GPT_INTERRUPT( 2 ); 			// unmask interrupt
}

void
halTimerStart( uint32_t addr )
{
	// NOTE: start the timer by writing 1 into the bit 0 of TCLR
	BIT_SET( TCLR_GPT( addr ), TCLR_ONOFF_BIT );
	AWAIT_BITS_SET( TCLR_GPT( addr ), TCLR_ONOFF_BIT );
}

void
halTimerStop( uint32_t addr )
{
	// NOTE: stop the timer by writing 0 into the bit 0 of TCLR
	BIT_CLEAR( TCLR_GPT( addr ), TCLR_ONOFF_BIT );
	AWAIT_BITS_CLEARED( TCLR_GPT( addr ), TCLR_ONOFF_BIT );
}

void
halTimerReset( uint32_t addr )
{
	// NOTE: reset timer by writing any value to TTGR
	BIT_CLEAR_ALL( TTGR_GPT( addr ) );
}

void
halTimerClearAllInterrupts( uint32_t addr )
{
	// NOTE: clear potentiall pending interrupts by writing 1 to bit 0-2 to TISR
	//		 which is equal of setting the value of TISR to 7
	BIT_SET( TISR_GPT( addr ), TISR_ALLIT_BITS );
}

void
halTimerClearInterrupt( uint32_t addr, uint32_t itBit )
{
	// NOTE: writing the according bit clears a potentially pending interrupt
	BIT_SET( TISR_GPT( addr ), itBit );
}

void
halTimerEnableInterrupt( uint32_t addr, uint32_t itBit )
{
	// NOTE: enable specific interrupt by writing the according bit to TIER
	BIT_SET( TIER_GPT( addr ), itBit );
}

void
halTimerSetCompareValue( uint32_t addr, uint32_t value )
{
	// NOTE: set compare-value to be compared to counter-value in match-interrupt mode
	TMAR_GPT( addr ) = value;
}

void
halTimerEnableCompare( uint32_t addr )
{
	// NOTE: enable compare-mode by setting bit 6 of TCLR
	BIT_SET( TCLR_GPT( addr ), TCLR_COMPARE_BIT );
}

void
halTimerSetLoadValue( uint32_t addr, uint32_t v )
{
	// NOTE: set load-value by writing to TLDR register
	//		 this value will be written to counter value when overflow
	TLDR_GPT( addr ) = v;
}

void
halTimerEnableAutoReload( uint32_t addr )
{
	// NOTE: enable auto-reload by setting bit 1 of TCLR
	BIT_SET( TCLR_GPT( addr ), TCLR_AUTORELOAD_BIT );
}
