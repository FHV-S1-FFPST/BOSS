/*
 * irq.c
 *
 *  Created on: 13.04.2014
 *      Author: Jonathan Thaler
 */

#include "irq.h"

#include "../core/core.h"
#include "../timer/irqtimer.h"
#include "../common/common.h"

// TODO: move to another include
#define INTCPS_SIR_IRQ_ADDR 	0x48200040
#define INTC_CONTROL 			0x48
#define INTC_CONTROL_NEWIRQAGR	0x00000001
#define SOC_AINTC_REGS			0x48200000

#define IRQ_INTERVAL 1000
#define MAX_IRQS 96

#pragma INTERRUPT ( fiqHandler, FIQ );

// NOTE: will be initialized to 0 anyway
irq_clbk callbacks[ MAX_IRQS ];

uint32_t
irqInit()
{
	irqTimerInit( IRQ_INTERVAL );

	// TODO: this shouldnt be necessary anymore because this is handled inside timer
	// NOTE: need to waste some time, otherwise IRQ won't hit
	volatile uint32_t i = 100000;
	while ( i > 0 )
		--i;

	return 0;
}

void
irqEnable()
{
	// enable IRQs AFTER we created the initial tasks
	// => when scheduling is called at least the idle-task is available
	_enable_IRQ();

	irqTimerStart();
}

void
irqRegisterClbk( irq_clbk clbk, uint32_t irqId )
{
	if ( irqId < MAX_IRQS )
	{
		callbacks[ irqId ] = clbk;
	}
}

uint32_t
irqHandler( UserContext* ctx )
{
	// fetch irqNr
	uint32_t irqNr = *( ( uint32_t* ) INTCPS_SIR_IRQ_ADDR );

	// TODO: make it more nice, remove magic numbers
	irqNr = ( irqNr & 127 );

	if ( callbacks[ irqNr] )
	{
		callbacks[ irqNr]();
	}

	// reset and clear timer interrupt flags
	irqTimerReset();

	// reset IRQ-interrupt flag
	reg32m( SOC_AINTC_REGS, INTC_CONTROL, INTC_CONTROL_NEWIRQAGR );

	return 0;
}

interrupt
void fiqHandler()
{
	// implement when necessary
}

