/*
 * scheduler.h
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <boss.h>

#include "../core/core.h"

uint32_t initScheduler( void );
uint32_t schedule( UserContext* ctx );

#endif /* SCHEDULER_H_ */
