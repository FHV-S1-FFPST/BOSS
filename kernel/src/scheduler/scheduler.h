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
Task* createTask( uint32_t* entryPoint );
void schedStart( void );
uint32_t schedule( UserContext* ctx );

#endif /* SCHEDULER_H_ */
