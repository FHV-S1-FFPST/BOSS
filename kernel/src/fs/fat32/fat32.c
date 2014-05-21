/*
 * fat32.c
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#include "fat32.h"

#include "../../hal/sd/sd_hal.h"
#include "../../common/common.h"

#include <string.h>
#include <stdlib.h>

#define GLOBAL_BPS_STRUCT_OFFSET 		0
#define FAT32_BPS_STRUCT_OFFSET			36

#define DATABUFFER_SIZE 				512

#define BOOTSECTOR_SIGNATURE_VALUE		0xAA55
#define BOOTSECTOR_SIGNATURE_OFFSET		0x1FE
#define PRIMARY_PARTITION_OFFSET		0x1BE

#define PRIMARY_PARTITION_FAT32_TYPE	0xB

// module-local data //////////////////////////////////////////////
static PRIMARY_PARTITION_INFO_STRUCT primaryPartition;
static FAT32_BPS_INFO_STRUCT fat32Bps;

static uint8_t dataBuffer[ DATABUFFER_SIZE ];

static uint32_t blocksPerSector;
static uint8_t* sectorSizeBuffer;

static uint8_t* fatTable;
///////////////////////////////////////////////////////////////////

// module-local functions /////////////////////////////////////////
static uint32_t loadPrimaryPartition( void );
static uint32_t loadFAT( void );
static uint32_t loadDirectories( void );
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

	if ( loadPrimaryPartition() )
	{
		return 1;
	}

	if ( loadFAT() )
	{
		return 1;
	}

	if ( loadDirectories() )
	{
		return 1;
	}

	return 0;
}

uint32_t
loadPrimaryPartition( void )
{
	// read the MBR
	if ( sdHalReadBlocks( 0, 1, dataBuffer ) )
	{
		return 1;
	}

	// check signature of MBR
	uint16_t signature = ( dataBuffer[ BOOTSECTOR_SIGNATURE_OFFSET ] ) | ( dataBuffer[ BOOTSECTOR_SIGNATURE_OFFSET + 1 ] << 8 );
	if ( BOOTSECTOR_SIGNATURE_VALUE != signature )
	{
		return 1;
	}

	// store primary-partition information
	memcpy( &primaryPartition, &dataBuffer[ PRIMARY_PARTITION_OFFSET ], sizeof( primaryPartition ) );

	// check if primary partition is FAT32
	if ( PRIMARY_PARTITION_FAT32_TYPE != primaryPartition.type )
	{
		return 1;
	}

	// read first 512 bytes of primary partition, will contain the information
	if ( sdHalReadBlocks( primaryPartition.partitionStart, 1, dataBuffer ) )
	{
		return 1;
	}

	// copy piece-wise ignoring alignment-bytes
	memcpy( &fat32Bps, &dataBuffer, 11 );
	memcpy( &( ( uint8_t* ) &fat32Bps )[ 12 ], &dataBuffer[ 11 ], 3 );
	memcpy( &( ( uint8_t* ) &fat32Bps )[ 16 ], &dataBuffer[ 14 ], 3 );
	memcpy( &( ( uint8_t* ) &fat32Bps )[ 20 ], &dataBuffer[ 17 ], 5 );
	memcpy( &( ( uint8_t* ) &fat32Bps )[ 26 ], &dataBuffer[ 22 ], 43 );
	memcpy( &( ( uint8_t* ) &fat32Bps )[ 70 ], &dataBuffer[ 65 ], 17 );
	memcpy( &( ( uint8_t* ) &fat32Bps )[ 88 ], &dataBuffer[ 82 ], 512 - 82 );

	// calculate total clusters and check if violates FAT32
	uint32_t root_dir_sectors = ( ( fat32Bps.root_entry_count * 32 ) + ( fat32Bps.bytes_per_sector - 1 ) ) /
					fat32Bps.bytes_per_sector;
	uint32_t data_sectors = fat32Bps.total_sectors_16 -
			( fat32Bps.reserved_sector_count +
					( fat32Bps.number_fats * fat32Bps.table_size_32 ) + root_dir_sectors );
	uint32_t total_clusters = data_sectors / fat32Bps.sectors_per_cluster;

	// ERROR: this is a FAT12 filesystem
	if ( total_clusters < 4085 )
	{
		return 1;
	}
	else
	{
		// ERROR: this is a FAT16 filesystem
	   if ( total_clusters < 65525)
	   {
		   return 1;
	   }
	}

	// allocate buffer to read one sector
	if ( DATABUFFER_SIZE == fat32Bps.bytes_per_sector )
	{
		// if a sector is exactly 512 bytes in size, then use the global databuffer (this is the most likely case in FAT32)
		sectorSizeBuffer = dataBuffer;
		// sector consists of exactly 1 block (of 512 bytes)
		blocksPerSector = 1;
	}
	else
	{
		// if a sector is larger than 512 bytes in size, then allocate the required buffer size
		sectorSizeBuffer = malloc( fat32Bps.bytes_per_sector );
		blocksPerSector = fat32Bps.bytes_per_sector / DATABUFFER_SIZE;
		if ( fat32Bps.bytes_per_sector % DATABUFFER_SIZE )
		{
			blocksPerSector++;
		}
	}

	return 0;
}

uint32_t
loadFAT( void )
{
	uint32_t first_fat_sector = primaryPartition.partitionStart +
			fat32Bps.reserved_sector_count;

	// allocate memory to hold the complete fat-table
	// TODO: this malloc fails
	fatTable = malloc( fat32Bps.table_size_32 * fat32Bps.bytes_per_sector );
	// read the complete fat-table in one step
	if ( sdHalReadBlocks( first_fat_sector, fat32Bps.table_size_32, fatTable ) )
	{
		return 1;
	}

	return 0;
}

uint32_t
loadDirectories( void )
{
	uint32_t first_data_sector = primaryPartition.partitionStart +
				fat32Bps.reserved_sector_count +
				( fat32Bps.number_fats * fat32Bps.table_size_32 );

	// read the first data-sector
	if ( sdHalReadBlocks( first_data_sector, 1, sectorSizeBuffer ) )
	{
		return 1;
	}

	return 0;
}
