#include "core/core.h"
#include "scheduler/scheduler.h"
#include "irq/irq.h"
#include "fs/fat32/fat32.h"
#include "mmu/mmu.h"

#include "tasksimpl/task1.h"

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
 * - power management functionality
 * - interrupts
 */
int32_t
initHardware( void )
{
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
	// TODO: setup task structures
	// TODO: setup scheduling structures
	// TODO: setup vmem structures

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
 * - keyboard driver
 * - RS232 (serial) driver
 * - DMX driver
 */
int32_t
initDrivers( void )
{
	// TODO: start keyboard-driver server
	// TODO: start RS232-driver server
	// TODO: start DMX-driver server
	if( fat32Init() )
	{
		return 1;
	}

	FILE boss = 0;
	FILE bossTxt = 0;
	FILE bossDirTxt = 0;
	FILE bossSubTxt = 0;
	FILE bossDir2 = 0;
	uint32_t ret = 0;

	uint8_t buffer[ 512 ];

	ret = fat32Open( "BOSS", &boss );
	ret = fat32Open( "BOSS.txt", &bossTxt );
	ret = fat32Open( "bossdir/bossdir.txt", &bossDirTxt );
	ret = fat32Open( "bossdir/Bosssub/bosssub.txt", &bossSubTxt );
	// should fail - cannot open a directory
	ret = fat32Open( "BossDir2", &bossDir2 );

	ret = fat32Read( boss, 512, buffer );
	ret = fat32Read( bossTxt, 512, buffer );
	ret = fat32Read( bossDirTxt, 512, buffer );
	ret = fat32Read( bossSubTxt, 512, buffer );

	ret = fat32Close( boss );
	ret = fat32Close( bossTxt );
	ret = fat32Close( bossDirTxt );
	ret = fat32Close( bossSubTxt );

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

	createTask( task1 );

	irqEnable();

	schedStart();

	return 0;
}
