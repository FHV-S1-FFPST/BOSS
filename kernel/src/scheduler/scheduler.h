/*
 * scheduler.h
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <boss.h>

uint32_t initScheduler( void );
void schedule( uint32_t* pc, uint32_t userCpsr, uint32_t* userRegs );
void scheduleTask( uint32_t pid );

#endif /* SCHEDULER_H_ */
