/*
 * fat32.h
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#ifndef FAT32_H_
#define FAT32_H_

#include <inttypes.h>

typedef struct
{
	uint8_t entryState;
	uint8_t startHead;
	uint16_t startCylinder;
	uint8_t type;
	uint8_t endHead;
	uint16_t endCylinder;
	uint32_t partitionStart;
	uint32_t numbersOfSectors;
} PRIMARY_PARTITION_INFO_STRUCT;

// NOTE: TODO
typedef struct
{
	uint8_t		bootjmp[ 3 ];
	uint8_t		oem_name[ 8 ];

	// align next to 12
	uint8_t		__alignmentHelper1;

	uint16_t	bytes_per_sector;
	uint8_t		sectors_per_cluster;

	// align next to 16
	uint8_t		__alignmentHelper2;

	uint16_t	reserved_sector_count;
	uint8_t		number_fats;

	// align next to 20
	uint8_t		__alignmentHelper3;

	uint16_t	root_entry_count;
	uint16_t	total_sectors_16;
	uint8_t		media_type;

	// align next to 26
	uint8_t		__alignmentHelper4;

	uint16_t	table_size_16;
	uint16_t	sectors_per_track;
	uint16_t	head_side_count;
	uint32_t 	hidden_sector_count;
	uint32_t 	total_sectors_32;

	// FAT32 specific fields
	uint32_t	table_size_32;
	uint16_t	extended_flags;
	uint16_t	fat_version;
	uint32_t	root_cluster;
	uint16_t	fat_info;
	uint16_t	backup_BS_sector;
	uint8_t 	reserved_0[12];
	uint8_t		drive_number;

	// align next to 70
	uint8_t		__alignmentHelper5;

	uint8_t		reserved_1;
	uint8_t		boot_signature;
	uint32_t	volume_id;
	uint8_t		volume_label[ 11 ];

	// align next to 88
	uint8_t		__alignmentHelper6;

	uint8_t		fat_type_label[ 8 ];

	uint8_t 	boot_code[ 420 ];
	uint16_t 	signature;
} FAT32_BPS_INFO_STRUCT;


uint32_t fat32Init();

#endif /* FAT32_H_ */
