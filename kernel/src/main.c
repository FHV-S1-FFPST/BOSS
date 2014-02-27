#include "core/core.h"
#include "syscalls/syscalls.h"

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

	// NOTE: at this point we can send through RS232

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
	// TODO: setup global kernel data-structure

	// TODO: setup task structures
	// TODO: setup scheduling structures
	// TODO: setup vmem structures

	if ( initCore() )
	{
		return 1;
	}

	if ( initSysCalls() )
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
	// TODO: start minimal bash

	return 0;
}
