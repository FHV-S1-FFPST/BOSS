/*
 * timer.h
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <boss.h>

#define TIMER2_ID 2
#define TIMER3_ID 3

#define TIMER_ADDR( id )		0x49030000 + ( id - 1 ) * 0x2000

#define TISR_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x018 ) ) )
#define TIER_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x01C ) ) )
#define TCRR_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x028 ) ) )
#define TMAR_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x038 ) ) )
#define TCLR_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x024 ) ) )
#define TTGR_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x030 ) ) )
#define TLDR_GPT( id ) 			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x02C ) ) )
#define TWER_GPT( id )			( *( ( volatile uint32_t* ) ( TIMER_ADDR( id ) + 0x020 ) ) )

#define INTC_ILR_GTP(number)      ( *((volatile uint32_t*) ((long)0x48200100 + 0x4 * (36 + number))))        // 0x4820 01000 + 0x98 (0x04*38)

#define INTC_MIRCLEAR_GTP (*((volatile uint32_t*) 0x482000A8))     //0x4820 0088 + (0x20 * 1)
#define INTCPS_CONTROL (*((volatile uint32_t*) 0x48200048))

#define HAL_TIMER_CE_GPT                              0x40
#define HAL_TIMER_ON_GPT                              0x01
#define HAL_TIMER_AR_GPT                              0x02
#define HAL_TIMER_CLEAR_INTERRUPT                     0x07

#define UNMASK_GPT_INTERRUPT(number)    0x10 << number

void timerInit( uint32_t timerId, uint32_t intervalMs );
void timerStart( uint32_t timerId );
void timerStop( uint32_t timerId );
void timerReset( uint32_t timerId );
void timerSetInterruptMode( uint32_t timerId );


#endif /* TIMER_H_ */
