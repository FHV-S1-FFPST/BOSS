/*
 * timer_hal.h
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#ifndef TIMER_HAL_H_
#define TIMER_HAL_H_

#include <boss.h>

#define GPTIMER1_ADDR			0x48318000
#define GPTIMER2_ADDR			0x49032000
#define GPTIMER3_ADDR			0x49034000
#define GPTIMER4_ADDR			0x49036000
#define GPTIMER5_ADDR			0x49038000
#define GPTIMER6_ADDR			0x4903A000
#define GPTIMER7_ADDR			0x4903C000
#define GPTIMER8_ADDR			0x4903E000
#define GPTIMER9_ADDR			0x49040000
#define GPTIMER10_ADDR			0x48086000
#define GPTIMER11_ADDR			0x48088000

#define CAPTURE_IT_BIT			0x04		// capture interrupt
#define OVERFLOW_IT_BIT			0x02		// overflow interrupt
#define MATCH_IT_BIT			0x01		// match interrupt

#define HAL_TIMER_TICKS_PER_MS 	1000		// system-ticks per ms

void halTimerStart( uint32_t addr );
void halTimerStop( uint32_t addr );
void halTimerReset( uint32_t addr );
void halTimerClearAllInterrupts( uint32_t addr );
void halTimerClearInterrupt( uint32_t addr, uint32_t itBit );
void halTimerEnableInterrupt( uint32_t addr, uint32_t itBit );
void halTimerSetCompareValue( uint32_t addr, uint32_t v );
void halTimerEnableCompare( uint32_t addr );
void halTimerSetLoadValue( uint32_t addr, uint32_t v );
void halTimerEnableAutoReload( uint32_t addr );

// TODO: need functions to configure 1ms tick timer
// 		-> select clock interval (32khz, 1Mhz, ... system-clock)
//		-> set positive/negative increment
//		-> ... ?

#endif /* TIMER_HAL_H_ */
