/*
 * scheduler.h
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "../task/task.h"

uint32_t schedInit( void );
int32_t getCurrentPid( void );
Task* createTask( uint32_t* entryPoint, uint32_t size );
void schedStart( void );

#endif /* SCHEDULER_H_ */
