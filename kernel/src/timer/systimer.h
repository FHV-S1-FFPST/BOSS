/*
 * systimer.h
 *
 *  Created on: 12.04.2014
 *      Author: Jonathan Thaler
 */

#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#include <inttypes.h>

void sysTimerInit( uint32_t ovfAfterMs );
uint32_t sysTimerValue( void );

#endif /* SYSTIMER_H_ */
