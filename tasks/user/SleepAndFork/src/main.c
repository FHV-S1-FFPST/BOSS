/*
 * task1.c
 *
 *  Created on: 27.05.2014
 *      Author: Jonathan Thaler
 */

#include <boss.h>

int
main( void )
{
	if ( fork() )
	{
		// NOTE: at this point we are in the parent of fork

		while( 1 )
		{
			uint64_t startMillis = getSysMillis();

			sleep( 5000 );

			uint64_t stopMillis = getSysMillis();
			uint64_t deltaMillis = stopMillis - startMillis;

			deltaMillis++;
		}
	}
	else
	{
		// NOTE: at this point we are in the child of fork

		while( 1 )
		{
			uint64_t startMillis = getSysMillis();

			sleep( 10000 );

			uint64_t stopMillis = getSysMillis();
			uint64_t deltaMillis = stopMillis - startMillis;

			deltaMillis++;
		}
	}
}
