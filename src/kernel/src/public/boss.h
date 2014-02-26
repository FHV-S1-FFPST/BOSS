/*
 * boss.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef BOSS_H_
#define BOSS_H_

/**
 * This is the header-file to be used by applications
 * need to access the API of BOSS and OS-Specific stuff
 */

/**
 * Sys-Calls
 */
int sendRecv( int id, unsigned char* data, unsigned short dataSize );

#endif /* BOSS_H_ */
