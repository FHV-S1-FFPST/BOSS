/*
 * syscalls.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include "syscalls.h"

#pragma SWI_ALIAS( putC, 8 );

int32_t putC( byte c );

int32_t
initSysCalls( void )
{
	byte c = 'c';

	if ( putC( c )  )
	{
		return 1;
	}

	return 0;
}

