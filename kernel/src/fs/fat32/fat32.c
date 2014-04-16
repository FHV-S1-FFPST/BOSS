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



// module-local data //////////////////////////////////////////////
static GLOBAL_BPS_STRUCT globalBps;
static FAT32_BPS_STRUCT fat32Bps;
// module-local functions /////////////////////////////////////////
///////////////////////////////////////////////////////////////////

uint32_t
fat32Init()
{
	// TODO: when do we REALLY need to initialize SDHAL and FAT32?
	//			-> one time only at startup? or lazy initialization upon first access?
	//			-> every time a SD card is inserted? then how do we detect when a SD card is inserted?

	sdHalInit();

	sdHalReadBytes( GLOBAL_BPS_STRUCT_OFFSET, &globalBps, sizeof( globalBps ) );
	sdHalReadBytes( FAT32_BPS_STRUCT_OFFSET, &fat32Bps, sizeof( fat32Bps ) );

	// TODO: inspect structs to ensure we are really fat32

	return 0;
}
