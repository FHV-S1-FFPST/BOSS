/*
 * irqtimer.h
 *
 *  Created on: 12.04.2014
 *      Author: Jonathan Thaler
 */

#ifndef IRQTIMER_H_
#define IRQTIMER_H_

#include <inttypes.h>

void irqTimerInit( uint32_t irqIntervalMs );
void irqTimerReset( void );
void irqTimerStart( void );
void irqTimerStop( void );

#endif /* IRQTIMER_H_ */
