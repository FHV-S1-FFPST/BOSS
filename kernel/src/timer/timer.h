/*
 * timer.h
 *
 *  Created on: 19.03.2014
 *      Author: Thaler
 */

#ifndef TIMER_H_
#define TIMER_H_

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

void timerInit( uint32_t timerId, uint32_t intervalMs );
void timerStart( uint32_t timerId );
void timerStop( uint32_t timerId );
void timerReset( uint32_t timerId );
void timerSetInterruptMode( uint32_t timerId );
void timerCfg1MsTick();

#endif /* TIMER_H_ */
