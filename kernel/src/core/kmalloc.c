/*
 * kmalloc.c
 *
 *  Created on: 05.06.2014
 *      Author: Thaler
 */

#include "kmalloc.h"

#include <stdlib.h>

// module-local defines

#define MEMORY_POOL_SIZE 		0x1000000
////////////////////////////////////////////////

// module-local data-structures

////////////////////////////////////////////////

// module-local functions

////////////////////////////////////////////////

// module-local data

//static uint8_t _memoryPool[ MEMORY_POOL_SIZE ];
////////////////////////////////////////////////

void*
kmalloc( uint32_t size )
{
	return malloc( size );
}

void
kfree( void* addr )
{
	free( addr );
}
