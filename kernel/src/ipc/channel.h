/*
 * channel.h
 *
 *  Created on: 03.06.2014
 *      Author: Jonathan Thaler
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "../task/task.h"

uint32_t channel_open( uint32_t channelId );
uint32_t channel_close( uint32_t channelId );
uint32_t channel_subscribe( uint32_t channelId, Task* task );

uint32_t channel_receivesMessage( uint32_t channelId, MESSAGE* msg, Task* task );
int32_t channel_waitForMessage( uint32_t channelId, Task* task, int32_t timeout );

#endif /* CHANNEL_H_ */
