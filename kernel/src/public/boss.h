/*
 * boss.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef BOSS_H_
#define BOSS_H_

#include <inttypes.h>

/**
 * This is the header-file to be used by applications
 * need to access the API of BOSS and OS-Specific stuff
 */

/**
 * Sys-Call IDs
 */
#define SYSCALL_SEND_ID			8
#define SYSCALL_RECEIVE_ID		9
#define SYSCALL_SENDRCV_ID		10

/**
 * define SWI-aliases for the given sys-calls for user-apps
 */
#ifndef KERNEL
	#pragma SWI_ALIAS( send, SYSCALL_SEND_ID );
	#pragma SWI_ALIAS( receive, SYSCALL_RECEIVE_ID );
	#pragma SWI_ALIAS( sendrcv, SYSCALL_SENDRCV_ID );
#endif

/**
 * the signatures of the system-calls
 */
int32_t send( uint32_t id, uint8_t* data, uint8_t dataSize );
int32_t receive( uint32_t id, uint8_t* data, uint8_t dataSize );
int32_t sendrcv( uint32_t id, uint8_t* data, uint8_t dataSize );

#endif /* BOSS_H_ */
