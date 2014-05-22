/*
 * fat32.h
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#ifndef FAT32_H_
#define FAT32_H_

#include <inttypes.h>

typedef uint32_t FILE;

uint32_t fat32Init( void );
uint32_t fat32Open( int8_t* filePath, FILE* file );
uint32_t fat32Close( FILE file );
uint32_t fat32Read( uint32_t nBytes, uint8_t* buffer );

#endif /* FAT32_H_ */
