/*
 * syscalls.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <types.h>

int32_t send( uint32_t id, byte* data, uint8_t dataSize );
int32_t receive( uint32_t id, byte* data, uint8_t dataSize );
int32_t sendrcv( uint32_t id, byte* data, uint8_t dataSize );

#endif /* SYSCALLS_H_ */
