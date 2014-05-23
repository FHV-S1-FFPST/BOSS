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

/**
 * Initializes the FAT32.
 * Returns 0 upon success.
 * Returns 1 upon failure.
 */
uint32_t fat32Init( void );

/**
 * Opens a file (for reading only) at the filePath.
 * Returns 0 upon success, where file will contain a valid FILE pointer.
 * Returns 1 upon failure, where file will not be changed.
 */
uint32_t fat32Open( const char* filePath, FILE* file );
/**
 * Closes a previously opened file.
 * Returns 0 upon success.
 * Returns 1 upon failure.
 */
uint32_t fat32Close( FILE file );

/**
 * Reads nBytes into buffer from file.
 * Returns the number of bytes actually read or -1 if failure or 0 if EOF has reached already in a previous read.
 */
uint32_t fat32Read( FILE file, uint32_t nBytes, uint8_t* buffer );

#endif /* FAT32_H_ */
