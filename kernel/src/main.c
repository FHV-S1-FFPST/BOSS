#include "core/core.h"
#include "scheduler/scheduler.h"
#include "irq/irq.h"
#include "fs/fat32/fat32.h"
#include "mmu/mmu.h"
#include "task/taskLoader.h"
#include "page_manager/pageManager.h"
#include "sdrc/sdrc.h"
#include "ipc/channel.h"

#include <stdlib.h>

/**
 * Prototypes
 */
static int32_t initHardware( void );
static int32_t initOs( void );
static int32_t initDrivers( void );
static int32_t initSystem( void );

/**
 * Entry-Point of Kernel.
 * NOTE: the entry-point of each module in the kernel is placed on top of the corresponding module-file
 */
int32_t
main( void )
{
	if ( initHardware() )
	{
		// initializing hardware failed, exit OS
		return 1;
	}

	/* THIS IS THE ACCESS PATTERN
	    allocate:
		4096
		64
		160
		512
		4096
		50516

		free:
		512
		4096
		50516

		allocate:
		512
		4096 -> returns 0 -> data abort

	void* addr = 0;
	void* addrFree1 = 0;
	void* addrFree2 = 0;
	void* addrFree3 = 0;

	addr = malloc( 4096 );
	addr = malloc( 64 );
	addr = malloc( 160 );
	addrFree1 = malloc( 512 );
	addrFree2 = malloc( 4096 );
	addrFree3 = malloc( 50516 );

	free( addrFree1 );
	free( addrFree2 );
	free( addrFree3 );

	addr = malloc( 512 );	// <- returns address from addrFree1 => seems to be working correct!
	addr = malloc( 4096 );
*/

	if ( initOs() )
	{
		// initializing OS failed, exit OS
		return 2;
	}

	if ( initDrivers() )
	{
		// initializing Drivers failed, exit OS
		return 3;
	}

	if ( initSystem() )
	{
		// initializing OS failed, exit OS
		return 4;
	}

	// the mother of all endless-loops...
	while ( 1 ) {}
}

/**
 * This function initializes the hardware of the beagleboard
 */
int32_t
initHardware( void )
{
	if ( initSDRam( ) ) {
		return 1;
	}

	if ( initPageManager( ) )
	{
		return 1;
	}

	if( mmu_init() )
	{
		return 1;
	}

	return 0;
}

/**
 * This function sets up and initializes all internal os data-structures
 * - tasks & scheduling
 * - address spaces
 * - paging
 * - sys-calls
 * - IPC
 * - ... ?
 *
 */
int32_t
initOs( void )
{
	if ( initCore() )
	{
		return 1;
	}

	if ( irqInit() )
	{
		return 1;
	}

	if( schedInit() )
	{
		return 1;
	}

	return 0;
}

/**
 * This function initializes all driver-servers
 */
int32_t
initDrivers( void )
{
	if( fat32Init() )
	{
		return 1;
	}

	loadTaskFromFile( "sys/hdmidrv.out" );
	loadTaskFromFile( "sys/console.out" );

	/*
	loadTaskFromFile( "sys/rs232drv.out" );
	loadTaskFromFile( "usr/saf.out");
*/
	return 0;
}

/**
 * This function initializes all servers necessary for system-functionality:
 * - minimal bash
 * - ...
 */
int32_t
initSystem( void )
{
	// NOTE: at this point we should be able to send through RS232

	irqEnable();

	schedStart();

	return 0;
}
