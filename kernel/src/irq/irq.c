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

#define INTCPS_SIR_IRQ_ADDR 			0x48200040

#define INTC_CONTROL 					0x48
#define INTC_CONTROL_NEWIRQAGR			0x00000001
#define SOC_AINTC_REGS					0x48200000

#define INTCPS_SIR_IRQ_IRNUMBER_BITS 	127

#define IRQ_INTERVAL 					1000
#define MAX_IRQS 						96

#pragma INTERRUPT ( fiqHandler, FIQ );

irq_clbk callbacks[ MAX_IRQS ];

// TODO: decrypt
#define INTC_MIRCLEAR_GTP (*((volatile uint32_t*) 0x482000A8))     //0x4820 0088 + (0x20 * 1)
#define UNMASK_GPT_INTERRUPT(number)    0x10 << number

uint32_t
irqGetCurrentIrNumber()
{
	uint32_t sirRegValue = READ_REGISTER( INTCPS_SIR_IRQ_ADDR );
	// keep bits 0-6 which contains the current interrupt-number
	BIT_KEEP( sirRegValue, INTCPS_SIR_IRQ_IRNUMBER_BITS );

	return sirRegValue;
}

uint32_t
irqInit()
{
	// TODO: read OMAP35x.pdf chapter about interrupts at page 1057
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
		// TODO: rewrite: unmask interrupt
		INTC_MIRCLEAR_GTP |= UNMASK_GPT_INTERRUPT( 2 );

		// TODO: enable IRQ interrupt for irqId
		callbacks[ irqId ] = clbk;
	}
}

uint32_t
irqHandler( UserContext* ctx )
{
	uint32_t irqNr = irqGetCurrentIrNumber();

	if ( callbacks[ irqNr] )
	{
		callbacks[ irqNr]();
	}

	// reset and clear timer interrupt flags
	irqTimerReset();

	// reset IRQ-interrupt flag
	BIT_SET( READ_REGISTER_OFFSET( SOC_AINTC_REGS, INTC_CONTROL ), INTC_CONTROL_NEWIRQAGR );

	return 0;
}

interrupt
void fiqHandler()
{
	// implement when necessary
}

