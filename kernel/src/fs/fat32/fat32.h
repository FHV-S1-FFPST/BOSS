/*
 * fat32.h
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#ifndef FAT32_H_
#define FAT32_H_

#include <inttypes.h>

// NOTE: starts at offset 0 in BIOS PARAMETER BLOCK
typedef struct
{
	uint8_t  BS_jmpBoot[ 3 ];
	uint8_t	 BS_OEMName[ 8 ];
	uint16_t BPB_BytsPerSec;
	uint8_t  BPB_SecPerClus;
	uint16_t BPB_RsvdSecCnt;
	uint8_t  BPB_NumFATs;
	uint16_t BPB_RootEntCnt;
	uint16_t BPB_TotSec16;
	uint8_t  BPB_Media;
	uint16_t BPB_FATSz16;
	uint16_t BPB_SecPerTrk;
	uint16_t BPB_NumHeads;
	uint32_t BPB_HiddSec;
	uint32_t BPB_TotSec32;
} GLOBAL_BPS_STRUCT;

// NOTE: starts at offset 36 in BIOS PARAMETER BLOCK
typedef struct
{
	uint32_t BPB_FATSz32;
	uint16_t BPB_ExtFlags;
	uint16_t BPB_FSVer;
	uint32_t BPB_RootClus;
	uint16_t BPB_FSInfo;
	uint16_t BPB_BkBootSec;
	uint32_t BPB_Reserved[ 3 ];
	uint8_t	 BS_DrvNum;
	uint8_t  BS_Reserved1;
	uint8_t  BS_BootSig;
	uint32_t BS_VolID;
	uint8_t	 BS_VolLab[ 11 ];
	uint8_t	 BS_FilSysType[ 8 ];
} FAT32_BPS_STRUCT;

uint32_t fat32Init();

#endif /* FAT32_H_ */
