/*
 * core.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include "core.h"

#pragma INTERRUPT ( undefInstrHandler, UDEF );
#pragma INTERRUPT ( swiHandler, SWI );
#pragma INTERRUPT ( prefetchAbortHandler, PABT );
#pragma INTERRUPT ( dataAbortHandler, DABT );
#pragma INTERRUPT ( irqHandler, IRQ );
#pragma INTERRUPT ( fiqHandler, FIQ );

int32_t
initCore( void )
{
	// TODO init kernel-structure

	return 0;
}

interrupt
void undefInstrHandler()
{
	// TODO: implement
}

interrupt
void swiHandler()
{
	// TODO: implement
}

interrupt
void prefetchAbortHandler()
{
	// TODO: implement
}

interrupt
void dataAbortHandler()
{
	// TODO: implement
}

interrupt
void irqHandler()
{
	// TODO: implement
}

interrupt
void fiqHandler()
{
	// TODO: implement
}
