/*
 * syscalls.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include "syscalls.h"

#pragma INTERRUPT ( SWI_Handler, SWI );
#pragma SWI_ALIAS( putC, 8 );

int32_t putC( byte c );

int32_t
initSysCalls( void )
{
	volatile uint32_t cpsr = _enable_interrupts();

	byte c = 'c';

	if ( putC( c )  )
	{
		return 1;
	}

	return 0;
}

interrupt
void SWI_Handler()
{
	int32_t i = 0;
	i = 42 * 2;
	int32_t x = i * i;

	// TODO: implement
}
