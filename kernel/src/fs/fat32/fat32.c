/*
 * fat32.c
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#include "fat32.h"

#include "../../hal/sd/sd_hal.h"

#define GLOBAL_BPS_STRUCT_OFFSET 	0
#define FAT32_BPS_STRUCT_OFFSET		36

#define DATABUFFER_BLOCKSIZE		512
#define DATABUFFER_BLOCKCOUNT 		1
#define DATABUFFER_SIZE 			DATABUFFER_BLOCKSIZE * DATABUFFER_BLOCKCOUNT

// module-local data //////////////////////////////////////////////
static GLOBAL_BPS_STRUCT globalBps;
static FAT32_BPS_STRUCT fat32Bps;
// module-local functions /////////////////////////////////////////
///////////////////////////////////////////////////////////////////

static uint8_t dataBuffer[ DATABUFFER_SIZE ];

uint32_t
fat32Init()
{
	// TODO: need to get an interrupt when card is inserted

	// TODO: when do we REALLY need to initialize SDHAL and FAT32?
	//			-> one time only at startup? or lazy initialization upon first access?
	//			-> every time a SD card is inserted? then how do we detect when a SD card is inserted?

	// initialize sd
	if ( sdHalInit() )
	{
		return 1;
	}

	// read the first block ( 512 bytes )
	if ( sdHalReadBlocks( 0, 1, dataBuffer ) )
	{
		return 1;
	}

	memcpy( &globalBps, ( dataBuffer + 446 ), sizeof( GLOBAL_BPS_STRUCT ) );
	memcpy( &fat32Bps, ( dataBuffer + 446 + 36 ), sizeof( FAT32_BPS_STRUCT ) );

	return 0;
}
