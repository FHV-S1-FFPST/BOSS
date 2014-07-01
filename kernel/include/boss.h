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

// agreed channel-ids /////////////////////////////////////////////////////////
#define NULL_CHANNEL			0
#define SERIAL_CHANNEL			1
#define HDMI_CHANNEL			2
///////////////////////////////////////////////////////////////////////////////

// Sys-Call IDs ///////////////////////////////////////////////////////////////
// IPC CALLS
#define SYSC_CH_OPEN			8
#define SYSC_CH_CLOSE			9
#define SYSC_CH_SUBSCRIBE		10

#define SYSC_SEND				11
#define SYSC_RECEIVE			12
#define SYSC_SENDRCV			13

// REGISTER-ACCESS CALLS
#define SYSC_READ_REG			14
#define SYSC_WRITE_REG			15

// EXIT-CALL
#define SYSC_EXIT_TASK			16

// MISC CALLS
#define SYSC_SYSMILLIS			17
#define SYSC_PID				18

// FILE-ACCESS CALLS
#define SYSC_FOPEN				19
#define SYSC_FCLOSE				20
#define SYSC_FSIZE				21
#define SYSC_FREAD				22

// SHARED-MEM CALLS
#define SYSC_ALLOCSHAREDMEM		23
#define SYSC_FREESHAREDMEM		24
///////////////////////////////////////////////////////////////////////////////

// defined SWI-aliases for the given sys-calls for user-apps //////////////////
#ifndef KERNEL
	// IPC CALLS
	#pragma SWI_ALIAS( channelOpen, SYSC_CH_OPEN );
	#pragma SWI_ALIAS( channelClose, SYSC_CH_CLOSE );
	#pragma SWI_ALIAS( channelSubscribe, SYSC_CH_SUBSCRIBE );

	#pragma SWI_ALIAS( send, SYSC_SEND );
	#pragma SWI_ALIAS( receive, SYSC_RECEIVE );
	#pragma SWI_ALIAS( sendrcv, SYSC_SENDRCV );

	// TASK-MANAGEMENT CALLS
	#pragma SWI_ALIAS( readReg, SYSC_READ_REG );
	#pragma SWI_ALIAS( writeReg, SYSC_WRITE_REG );

	// EXIT-CALL
	#pragma SWI_ALIAS( exitTask, SYSC_EXIT_TASK );

	// MISC CALLS
	#pragma SWI_ALIAS( getSysMillis, SYSC_SYSMILLIS );
	#pragma SWI_ALIAS( getPid, SYSC_PID );

	// FILE-ACCESS CALLS
	#pragma SWI_ALIAS( bossfopen, SYSC_FOPEN );
	#pragma SWI_ALIAS( bossfclose, SYSC_FCLOSE );
	#pragma SWI_ALIAS( bossfsize, SYSC_FSIZE );
	#pragma SWI_ALIAS( bossfread, SYSC_FREAD );

	// SHARED-MEM CALLS
	#pragma SWI_ALIAS( allocSharedMem, SYSC_ALLOCSHAREDMEM );
	#pragma SWI_ALIAS( freeSharedMem, SYSC_FREESHAREDMEM );
#endif
///////////////////////////////////////////////////////////////////////////////

// SYSTEM-STRCUTURES

#define MESSAGE_MAX_DATA_SIZE 256

typedef struct
{
	uint32_t id;

	uint8_t data[ MESSAGE_MAX_DATA_SIZE ];
	uint8_t dataSize;
} MESSAGE;

typedef int32_t file_id;

// SYSTEM-CALLS ///////////////////////////////////////////////////////////////

// IPC calls
/**
 * Opens a channel with the given id.
 * If id is already in use or failed then will return 1.
 * If succeeds return 0.
 */
int32_t channelOpen( uint32_t channelId );
/**
 * closes an opened channel with given id.
 * If channel doesnt exist or failed, return 1.
 * If succeeds return 0.
 */
int32_t channelClose( uint32_t channelId );
/**
 * Subscribers to an opened channel with the given id.
 * If channel doesnt exist it returns 1.
 * If succeeds return 0.
 */
int32_t channelSubscribe( uint32_t channelId );

/**
 * Sends async a message to the given channel.
 */
int32_t send( uint32_t channelId, MESSAGE* msg );

/**
 * Receives a message from a given channel waiting timeout milliseconds for a message to arrive.
 * If timeout is > 0 it will block at least timeout ms until a message arrives.
 * If timeout is 0, it will block forever.
 * If timeout is -1, it will return immediately if no message is present and will have msg set to 0.
 *
 * returns 0 if message a was received
 * returns -1 if timeout occured
 * return > 0 for other errors
 */
int32_t receive( uint32_t channelId, MESSAGE* msg, int32_t timeout );

/**
 * Sends a message and receives a response waiting for timeout milliseconds.
 * msg is used to hold both the sending message and the receiving message.
 * If timeout is 0 this will block forever.
 */
int32_t sendrcv( uint32_t channelId, MESSAGE* msg, uint32_t timeout );

/**
 * Reads the value from a given register and returns it.
 */
uint32_t readReg( uint32_t address );

/**
 * Writes a value to a given register.
 */
uint32_t writeReg( uint32_t address, uint32_t value );

/**
 * Exits the task calling this function
 */
void exitTask( uint32_t code );

/**
 * Returns the milliseconds since the system was started.
 */
uint64_t getSysMillis();

/**
 * Returns the pid of the current process
 */
int32_t getPid();

/**
 * Opens a file (for reading only) from the filePath.
 * Returns 0 upon success, where file will contain a valid FILE pointer.
 * Returns 1 upon failure, where file will not be changed.
 */
uint32_t bossfopen( const char* filePath, file_id* fileId );

/**
 * Closes a previously opened file.
 * Returns 0 upon success.
 * Returns 1 if not opened or invalid fileid.
 */
uint32_t bossfclose( file_id fileId );

/**
 * Reads up to nBytes into buffer from file.
 * Returns the number of bytes actually read or -1 if failure or 0 if EOF has reached already during a previous read.
 *
 * HINT: read always less than 512 bytes as it is much faster.
 * A test reading a 5MB MP3 file delivered:
 * 133bytes: 8.3 sec
 * 510bytes: 7.5 sec
 * 512,1024,496 bytes: 13.9sec
 */
int32_t bossfread( file_id fileId, uint32_t nBytes, uint8_t* buffer );

/**
 * Stores the fileSize of the file with fileId in size.
 * Returns 0 upon success.
 * Returns 1 if not opened or invalid fileid.
 */
uint32_t bossfsize( file_id fileId, uint32_t* size );

/**
 * allocates shared memory with given size
 * returns 0 upon failure
 * returns the address upon success
 */
void* allocSharedMem( uint32_t size );

/**
 * frees previously allocated shared memory
 */
void freeSharedMem( void* mem );
///////////////////////////////////////////////////////////////////////////////

#endif /* BOSS_H_ */
