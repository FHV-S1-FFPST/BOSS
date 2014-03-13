/*
 * taskTable.h
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#ifndef TASKTABLE_H_
#define TASKTABLE_H_

#include "task.h"

#define MAX_TASKS 32

Task* getTask( uint32_t pid );
int32_t addTask( Task* task );
int32_t getNextFreePID();
int32_t getNumOfTasks();

#endif /* TASKTABLE_H_ */
