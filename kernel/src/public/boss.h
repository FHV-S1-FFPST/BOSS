/*
 * boss.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef BOSS_H_
#define BOSS_H_

#include <types.h>

/**
 * This is the header-file to be used by applications
 * need to access the API of BOSS and OS-Specific stuff
 */

/**
 * Sys-Calls
 */
int32 sendRecv( int32 id, byte* data, uint16 dataSize );

#endif /* BOSS_H_ */
