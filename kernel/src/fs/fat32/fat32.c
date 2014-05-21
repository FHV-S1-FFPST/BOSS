/*
 * fat32.c
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#include "fat32.h"

#include "../../hal/sd/sd_hal.h"
#include "../../common/common.h"

#define GLOBAL_BPS_STRUCT_OFFSET 	0
#define FAT32_BPS_STRUCT_OFFSET		36

#define DATABUFFER_BLOCKSIZE		512
#define DATABUFFER_BLOCKCOUNT 		1
#define DATABUFFER_SIZE 			DATABUFFER_BLOCKSIZE * DATABUFFER_BLOCKCOUNT

#define BOOTSECTOR_SIGNATURE_VALUE		0x55AA
#define BOOTSECTOR_SIGNATURE_OFFSET		0x1FE
#define PRIMARY_PARTITION_OFFSET		0x1BE

#define PRIMARY_PARTITION_FAT32_TYPE	0xB

// module-local data //////////////////////////////////////////////
static PRIMARY_PARTITION_STRUCT primaryPartition;
static GLOBAL_BPS_STRUCT globalBps;
static FAT32_BPS_STRUCT fat32Bps;

static uint8_t dataBuffer[ DATABUFFER_SIZE ];
///////////////////////////////////////////////////////////////////

// module-local functions /////////////////////////////////////////
static uint32_t readDataFromAddress( void );
static bool checkPartitionEntry( uint8_t* mbr, uint16_t offset );
///////////////////////////////////////////////////////////////////

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

	// read the MBR ( 512 bytes )
	if ( sdHalReadBlocks( 0, DATABUFFER_BLOCKCOUNT, dataBuffer ) )
	{
		return 1;
	}

	uint16_t signature = dataBuffer[ BOOTSECTOR_SIGNATURE_OFFSET ];

	if ( checkPartitionEntry( dataBuffer, PRIMARY_PARTITION_OFFSET ) )
	{

	}

	if ( BOOTSECTOR_SIGNATURE_VALUE != signature )
	{
		return 1;
	}

	return 0;
}

bool
checkPartitionEntry( uint8_t* mbr, uint16_t offset )
{
	memcpy( &primaryPartition, &mbr[ offset ], sizeof( primaryPartition ) );

	uint32_t cylinder = ( 0xFF00 & primaryPartition.startCylinder ) |
			( ( ( 0xFF & primaryPartition.startCylinder ) & 0xC0 ) << 2 );
	uint32_t sector = primaryPartition.startCylinder & 0x3F;

	// check if partition-type is FAT32
	if ( PRIMARY_PARTITION_FAT32_TYPE != primaryPartition.type )
	{
		return FALSE;
	}

	if ( sdHalReadBlocks( primaryPartition.sectorsDeltaMBRToFirstSector, DATABUFFER_BLOCKCOUNT, dataBuffer ) )
	{
		return FALSE;
	}

	memcpy( &globalBps, &dataBuffer[ GLOBAL_BPS_STRUCT_OFFSET ], sizeof( globalBps ) );
	memcpy( &fat32Bps, &dataBuffer[ FAT32_BPS_STRUCT_OFFSET ], sizeof( fat32Bps ) );

	return TRUE;
}

uint32_t
readDataFromAddress( void )
{
	return 0;
}
