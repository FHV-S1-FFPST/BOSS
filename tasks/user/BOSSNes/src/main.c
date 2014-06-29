/*
 * main.c
 *
 *  Created on: 29.06.2014
 *      Author: Jonathan Thaler
 */

#include <boss.h>

#include <stdlib.h>

#include "emu/NES.h"

void
main( void )
{
	/*
	initHDMI(1024, 960);
	setScale(5);
	//drawString("Nobody knows where my Johnny has gone");
	*/

	startNES( "roms/mario.nes" );

	exitTask( 0 );
}
