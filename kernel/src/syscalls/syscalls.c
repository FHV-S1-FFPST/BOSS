/*
 * syscalls.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include "syscalls.h"

#pragma INTERRUPT ( SWI_Handler, SWI );

#pragma SWI_ALIAS( put, 48 );

int put ( char *key, int value );

int
initSysCalls( void )
{
	char c = 'c';

	if ( put( &c, 42 )  )
	{
		return 1;
	}

	return 0;
}

interrupt void SWI_Handler()
{
	int i = 0;
	i = 42 * 2;
	int x = i * i;

}

