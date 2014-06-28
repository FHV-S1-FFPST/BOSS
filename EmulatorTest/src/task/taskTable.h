/*
 * taskTable.h
 *
 *  Created on: 05.03.2014
 *      Author: Michael
 */

#ifndef TASKTABLE_H_
#define TASKTABLE_H_

#include "task.h"

#define MAX_TASKS 255

Task* getTask( uint8_t pid );
int32_t addTask( Task* task );
uint8_t getNextFreePID();
uint8_t getNumOfTasks();

#endif /* TASKTABLE_H_ */
