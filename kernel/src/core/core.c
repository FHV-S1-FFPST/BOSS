/*
 * core.c
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#include "core.h"

#include "syscalls.h"

#pragma SWI_ALIAS( send, 8 );
#pragma SWI_ALIAS( receive, 9 );
#pragma SWI_ALIAS( sendrcv, 10 );

#pragma INTERRUPT ( resetHandler, RESET );
#pragma INTERRUPT ( undefInstrHandler, UDEF );
#pragma INTERRUPT ( swiHandler, SWI );
#pragma INTERRUPT ( prefetchAbortHandler, PABT );
#pragma INTERRUPT ( dataAbortHandler, DABT );
#pragma INTERRUPT ( irqHandler, IRQ );
#pragma INTERRUPT ( fiqHandler, FIQ );

int32_t
initCore( void )
{
	// TODO: setup global kernel data-structure

	uint8_t data[] = "jonathan";
	uint8_t dataSize = sizeof( data );

	uint32_t ret = send( 42, data, dataSize );

	return 0;
}

interrupt
void resetHandler()
{
	// TODO: implement
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

	//asm( " STMFD sp!,{r0-r12,lr}" ); 	// Store registers.

	asm( " LDR r0,[lr,#-4]" ); 			// Calculate address of SWI instruction and load it into r0.
	asm( " BIC r0,r0,#0xff000000" ); 	// Mask off top 8 bits of

	register int swiId;// asm( " r0" );

	if ( 8 == swiId )
	{

	}
	// TODO: restore registers r0-r3 and r12
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
