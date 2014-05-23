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
#include <ctype.h>

// module defines //////////////////////////////////////////////
#define SD_CARD_BLOCK_SIZE				512

#define MBR_AND_BPS_SIZE				0x200

#define MBR_SIGNATURE_VALUE				0xAA55
#define MBR_SIGNATURE_OFFSET			0x1FE
#define PRIMARY_PARTITION_OFFSET		0x1BE

#define PRIMARY_PARTITION_FAT32_TYPE	0xB

#define FAT32_MIN_CLUSTER_COUNT			65525

#define DIRECTORY_UNUSED 			0xE5
#define DIRECTORY_END				0x0

#define DIR_ATTRIB_READONLY			0x1
#define DIR_ATTRIB_HIDDEN			0x2
#define DIR_ATTRIB_SYSTEM			0x4
#define DIR_ATTRIB_VOLID			0x8
#define DIR_ATTRIB_DIRECTORY		0x10
#define DIR_ATTRIB_ARCHIVE			0x20
///////////////////////////////////////////////////////////////////

// module-local structures
typedef struct
{
	uint8_t entryState;
	uint8_t startHead;
	uint16_t startCylinder;
	uint8_t type;
	uint8_t endHead;
	uint16_t endCylinder;
	uint32_t partitionStart_lba;
	uint32_t numbersOfSectors;
} PRIMARY_PARTITION_INFO_STRUCT;

// NOTE: using packed AND enabling --unaligned-access=on in compiler options doesnt help:
// unaligned access will lead to data-abort interruptions => insert alignmentbytes by hand
// and copy piece-wise
typedef struct
{
	uint8_t		bootjmp[ 3 ];
	uint8_t		oem_name[ 8 ];

	// align next to 12
	uint8_t		__alignmentByte1;

	uint16_t	bytes_per_sector;
	uint8_t		sectors_per_cluster;

	// align next to 16
	uint8_t		__alignmentByte2;

	uint16_t	reserved_sector_count;
	uint8_t		number_fats;

	// align next to 20
	uint8_t		__alignmentByte3;

	uint16_t	root_entry_count;
	uint16_t	total_sectors_16;
	uint8_t		media_type;

	// align next to 26
	uint8_t		__alignmentByte4;

	uint16_t	table_size_16;
	uint16_t	sectors_per_track;
	uint16_t	head_side_count;
	uint32_t 	hidden_sector_count;
	uint32_t 	total_sectors_32;

	// FAT32 specific fields
	uint32_t	table_size_32;			// the number of SECTORS (NOT bytes!) occupied by one FAT32 FAT table
	uint16_t	extended_flags;
	uint16_t	fat_version;
	uint32_t	root_cluster;
	uint16_t	fat_info;
	uint16_t	backup_BS_sector;
	uint8_t 	reserved_0[12];
	uint8_t		drive_number;

	// align next to 70
	uint8_t		__alignmentByte5;

	uint8_t		reserved_1;
	uint8_t		boot_signature;
	uint32_t	volume_id;
	uint8_t		volume_label[ 11 ];

	// align next to 88
	uint8_t		__alignmentByte6;

	uint8_t		fat_type_label[ 8 ];

	uint8_t 	boot_code[ 420 ];
	uint16_t 	signature;
} FAT32_BPS_INFO_STRUCT;

typedef struct
{
	char		fileName[ 11 ];
	uint8_t 	attributes;
	uint8_t 	reserved;
	uint8_t 	createdSecFract;
	uint16_t 	creationTime;
	uint16_t 	creationDate;
	uint16_t 	lastAccess;
	uint16_t 	clusterNumberHigh;
	uint16_t 	lastModTime;
	uint16_t 	lastModDate;
	uint16_t 	clusterNumberLow;
	uint32_t	fileSize;
} FAT32_ENTRY;

typedef enum
{
	ARCHIVE = 0,
	DIRECTORY,
	VOLUME
} DIR_TYPE;

// NOTE: using void* for children instead of DIR_ENTRY as this FUCKING TI compiler is not
// capable of handling this
typedef struct __DIR_ENTRY
{
	char				fileName[ 12 ];
	DIR_TYPE 			type;
	uint32_t 			clusterNumber;
	uint32_t			fileSize;

	int32_t					childrenCount;
	struct __DIR_ENTRY*		children;
} DIR_ENTRY;


///////////////////////////////////////////////////////////////////

// module-local data //////////////////////////////////////////////
static PRIMARY_PARTITION_INFO_STRUCT _primaryPartition;
static FAT32_BPS_INFO_STRUCT _fat32Bps;
static DIR_ENTRY _fsRoot;

static uint8_t _blocksPerSector;

static uint8_t* _fatTable;

static uint8_t* _clusterBuffer;
static uint32_t _clusterBufferSize;

static uint32_t _clusterBegin_lba;
///////////////////////////////////////////////////////////////////

// module-local functions /////////////////////////////////////////
static uint32_t loadPrimaryPartition( void );
static uint32_t loadFAT( void );
static uint32_t loadFsRoot( void );
static uint32_t loadDirectory( uint32_t cluster, DIR_ENTRY* dir );
static void readDirectory( uint8_t* buffer, DIR_ENTRY* dir );
static uint32_t locateDirEntry( const char* filePath, DIR_ENTRY* parent, DIR_ENTRY** entry );
///////////////////////////////////////////////////////////////////

uint32_t
fat32Init( void )
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

	if ( loadFsRoot() )
	{
		return 1;
	}

	return 0;
}

uint32_t
fat32Open( const char* filePath, FILE* file )
{
	DIR_ENTRY* entry = 0;

	if ( locateDirEntry( filePath, &_fsRoot, &entry ) )
	{
		return 1;
	}



	return 0;
}

uint32_t
fat32Close( FILE file )
{
	// TODO: implement

	return 0;
}

uint32_t
fat32Read( FILE file, uint32_t nBytes, uint8_t* buffer )
{
	// TODO: implement

	return 0;
}

uint32_t
locateDirEntry( const char* filePath, DIR_ENTRY* parent, DIR_ENTRY** entry )
{
	uint32_t i = 0;

	char* isDirectory = strchr( filePath, '/' );
	char* fileName = ( char* ) filePath;

	if ( isDirectory )
	{
		fileName = strtok( fileName, "/" );
	}

	for ( i = 0; i < parent->childrenCount; ++i )
	{
		DIR_ENTRY* child = &( ( DIR_ENTRY* ) parent->children )[ i ];

		// look for a matching file-name
		if ( fileName == strstr( fileName, child->fileName ) )
		{
			// the entry is a directory, check if its part of the filename and traverse down
			if ( DIRECTORY == child->type )
			{
				// its part of the filename, look recursively in the subdirectory
				if ( isDirectory )
				{
					// directory not yet loaded, load it now
					if ( -1 == child->childrenCount )
					{
						uint32_t cluster_lba = _clusterBegin_lba + ( child->clusterNumber - 2 ) * _fat32Bps.sectors_per_cluster;

						// loading failed
						if ( loadDirectory( cluster_lba, child ) )
						{
							return 1;
						}
					}

					// look recursively in subdirectory
					return locateDirEntry( isDirectory + 1, child, entry );
				}
				// its the ending of the filename
				else
				{
					// want to open a directory, is not allowed
					return 1;
				}
			}
			// the entry is a file, found it if its not part of the filname but its ending
			else if ( ARCHIVE == child->type )
			{
				if ( ! isDirectory )
				{
					// found
					*entry = child;
					return 0;
				}
			}
		}
	}

	return 0;
}

uint32_t
loadPrimaryPartition( void )
{
	uint8_t dataBuffer[ MBR_AND_BPS_SIZE ];
	memset( dataBuffer, 0, MBR_AND_BPS_SIZE );

	// read the MBR from card
	if ( sdHalReadBlocks( 0, 1, dataBuffer ) )
	{
		return 1;
	}

	// check signature of MBR and reject invalid MBR signatures
	uint16_t signature = ( dataBuffer[ MBR_SIGNATURE_OFFSET ] ) | ( dataBuffer[ MBR_SIGNATURE_OFFSET + 1 ] << 8 );
	if ( MBR_SIGNATURE_VALUE != signature )
	{
		return 1;
	}

	// store primary-partition information in structure
	memcpy( &_primaryPartition, &dataBuffer[ PRIMARY_PARTITION_OFFSET ], sizeof( _primaryPartition ) );

	// check if primary partition is FAT32
	if ( PRIMARY_PARTITION_FAT32_TYPE != _primaryPartition.type )
	{
		return 1;
	}

	// read BPS of primary partition, will contain the information
	if ( sdHalReadBlocks( _primaryPartition.partitionStart_lba, 1, dataBuffer ) )
	{
		return 1;
	}

	// copy piece-wise ignoring alignment-bytes - this is a workaround of the alignment problems (see declaration of struct)
	memcpy( &_fat32Bps, &dataBuffer, 11 );
	memcpy( &( ( uint8_t* ) &_fat32Bps )[ 12 ], &dataBuffer[ 11 ], 3 );
	memcpy( &( ( uint8_t* ) &_fat32Bps )[ 16 ], &dataBuffer[ 14 ], 3 );
	memcpy( &( ( uint8_t* ) &_fat32Bps )[ 20 ], &dataBuffer[ 17 ], 5 );
	memcpy( &( ( uint8_t* ) &_fat32Bps )[ 26 ], &dataBuffer[ 22 ], 43 );
	memcpy( &( ( uint8_t* ) &_fat32Bps )[ 70 ], &dataBuffer[ 65 ], 17 );
	memcpy( &( ( uint8_t* ) &_fat32Bps )[ 88 ], &dataBuffer[ 82 ], 512 - 82 );

	// calculate total clusters and check if violates FAT32
	uint32_t root_dir_sectors = ( ( _fat32Bps.root_entry_count * 32 ) + ( _fat32Bps.bytes_per_sector - 1 ) ) / _fat32Bps.bytes_per_sector;
	uint32_t data_sectors = _fat32Bps.total_sectors_16 - ( _fat32Bps.reserved_sector_count + ( _fat32Bps.number_fats * _fat32Bps.table_size_32 ) + root_dir_sectors );
	uint32_t total_clusters = data_sectors / _fat32Bps.sectors_per_cluster;

	// ERROR: this is not a FAT32 system
	if ( total_clusters < FAT32_MIN_CLUSTER_COUNT )
	{
		return 1;
	}

	// will always be a division without rest as bytes_per_sector is either 512, 1024, 2048 or 4096
	_blocksPerSector = _fat32Bps.bytes_per_sector / SD_CARD_BLOCK_SIZE;

	return 0;
}

uint32_t
loadFAT( void )
{
	// TODO: maybe we don't need to read the complete FAT-table once, at it takes some time the larger the filesystem

	uint32_t fatTableSize = _fat32Bps.table_size_32 * _fat32Bps.bytes_per_sector; // is sectors * bytes
	uint32_t firstFatSector_lba = _primaryPartition.partitionStart_lba + _fat32Bps.reserved_sector_count;

	// allocate memory to hold the complete fat-table
	_fatTable = malloc( fatTableSize );
	memset( _fatTable, 0, fatTableSize );

	// read the complete fat-table in one step
	if ( sdHalReadBlocks( firstFatSector_lba, _fat32Bps.table_size_32 * _blocksPerSector, _fatTable ) )
	{
		return 1;
	}

	return 0;
}

uint32_t
loadFsRoot( void )
{
	_clusterBegin_lba = _primaryPartition.partitionStart_lba + _fat32Bps.reserved_sector_count + ( _fat32Bps.number_fats * _fat32Bps.table_size_32 );

	// allocate memory to hold one complete cluster
	_clusterBufferSize = _fat32Bps.bytes_per_sector * _fat32Bps.sectors_per_cluster;
	_clusterBuffer = malloc( _clusterBufferSize );
	memset( _clusterBuffer, 0, _clusterBufferSize );

	if ( loadDirectory( _clusterBegin_lba, &_fsRoot ) )
	{
		return 1;
	}

	return 0;
}

uint32_t
loadDirectory( uint32_t cluster_lba, DIR_ENTRY* dir )
{
	// read the complete first cluster
	if ( sdHalReadBlocks( cluster_lba, _fat32Bps.sectors_per_cluster * _blocksPerSector, _clusterBuffer ) )
	{
		return 1;
	}

	readDirectory( _clusterBuffer, dir );

	return 0;
}

void
readDirectory( uint8_t* buffer, DIR_ENTRY* dir )
{
	uint32_t i = 0;
	uint32_t childIndex = 0;
	uint32_t bytesPerCluster = _fat32Bps.sectors_per_cluster * _fat32Bps.bytes_per_sector;

	for ( i = 0; i < bytesPerCluster; i += 32 )
	{
		// ignore unused files
		if ( DIRECTORY_UNUSED == buffer[ i ] )
		{
			continue;
		}
		else if ( DIRECTORY_END == buffer[ i ] )
		{
			// reached end of directory, stop iterating through data
			break;
		}

		FAT32_ENTRY* fat32Entry = ( FAT32_ENTRY* ) &buffer[ i ];

		if ( DIR_ATTRIB_HIDDEN & fat32Entry->attributes || DIR_ATTRIB_SYSTEM & fat32Entry->attributes )
		{
			// ignore hidden or system files
			continue;
		}

		if ( DIR_ATTRIB_VOLID & fat32Entry->attributes )
		{
			// NOTE: this is a hack to store the VolumeID in the root of the FS
			memcpy( dir->fileName, fat32Entry->fileName, sizeof( fat32Entry->fileName ) );
			dir->type = VOLUME;

			// ignore volume-id
			continue;
		}

		dir->childrenCount++;
	}

	dir->children = malloc( dir->childrenCount * sizeof( DIR_ENTRY ) );
	memset( dir->children, 0, dir->childrenCount * sizeof( DIR_ENTRY ) );

	for ( i = 0; i < bytesPerCluster; i += 32 )
	{
		// ignore unused files
		if ( DIRECTORY_UNUSED == buffer[ i ] )
		{
			continue;
		}
		else if ( DIRECTORY_END == buffer[ i ] )
		{
			// reached end of directory, stop iterating through data
			break;
		}

		FAT32_ENTRY* fat32Entry = ( FAT32_ENTRY* ) &buffer[ i ];

		if ( DIR_ATTRIB_HIDDEN & fat32Entry->attributes ||
				 DIR_ATTRIB_SYSTEM & fat32Entry->attributes ||
				 DIR_ATTRIB_VOLID & fat32Entry->attributes )
		{
			// ignore hidden/system/volume-id files
			continue;
		}

		if ( DIR_ATTRIB_DIRECTORY & fat32Entry->attributes )
		{
			dir->children[ childIndex ].type = DIRECTORY;
			dir->children[ childIndex ].childrenCount = -1; // set to -1 to mark as not loaded yet - will be done lazily when searching through the filesystem during open file
		}

		if ( DIR_ATTRIB_ARCHIVE & fat32Entry->attributes )
		{
			dir->children[ childIndex ].type = ARCHIVE;
		}

		dir->children[ childIndex ].fileSize = fat32Entry->fileSize;
		dir->children[ childIndex ].clusterNumber = ( fat32Entry->clusterNumberHigh << 0x10 ) | fat32Entry->clusterNumberLow;

		uint8_t c = 0;
		uint8_t cDir = 0;
		bool foundSpace = FALSE;

		for ( c = 0; c < sizeof( fat32Entry->fileName ); ++c )
		{
			if ( ' ' == fat32Entry->fileName[ c ] )
			{
				foundSpace = TRUE;
			}
			else
			{
				if ( foundSpace )
				{
					dir->children[ childIndex ].fileName[ cDir ] = '.';
					++cDir;
					foundSpace = FALSE;
				}

				dir->children[ childIndex ].fileName[ cDir ] = tolower( fat32Entry->fileName[ c ] );
				++cDir;
			}
		}

		++childIndex;
	}
}
