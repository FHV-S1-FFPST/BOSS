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
	_enable_FIQ();
	_enable_IRQ();

	cpsr = _get_CPSR();

	byte c = 'c';

	if ( putC( c )  )
	{
		return 1;
	}

	return 0;
}

__interrupt
void SWI_Handler( uint32_t id )
{
	int32_t i = id;
	i = 42 * 2;
	int32_t x = i * i;

}
