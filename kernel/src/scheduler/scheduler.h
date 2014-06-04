/*
 * scheduler.h
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "../task/task.h"
#include "../core/core.h"

uint32_t schedInit( void );
uint8_t getCurrentPid( void );
Task* createTask( uint32_t entryPointVAddress, uint32_t stackPointerVAddress );
void schedStart( void );
uint32_t schedule( UserContext* ctx );
uint32_t saveCurrentRunning( UserContext* ctx );
uint32_t scheduleNextReady( UserContext* ctx );

#endif /* SCHEDULER_H_ */
