/*
 * irq.c
 *
 *  Created on: 13.04.2014
 *      Author: Jonathan Thaler
 */

#include "irq.h"

#include "../core/core.h"
#include "../common/common.h"

// NOTE: base address of all interrupt-controller registers
#define MPU_INTC_ADDR					0x48200000

// INTERRUPT REGISTER-ACCESS ////////////////////////////////////////////////////////////////////////////
#define INTCPS_REVISION					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x0 ) 		// R
#define INTCPS_SYSCONFIG				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x10 ) 	// RW
#define INTCPS_SYSSTATUS				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x14 ) 	// R
#define INTCPS_SIR_IRQ					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x40 ) 	// R
#define INTCPS_SIR_FIQ					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x44 ) 	// R
#define INTCPS_CONTROL					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x48 ) 	// RW
#define INTCPS_PROTECTION				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x4C ) 	// RW
#define INTCPS_IDLE						READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x50 ) 	// RW
#define INTCPS_IRQ_PRIORITY				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x60 ) 	// RW
#define INTCPS_FIQ_PRIORITY				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x64 ) 	// RW
#define INTCPS_THRESHOLD				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x68 ) 	// RW
// NOTE: n = 0 to 2
#define INTCPS_ITR( n )					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x80 + ( 0x20 * n ) ) 	// R
#define INTCPS_MIR( n )					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x84 + ( 0x20 * n ) ) 	// RW
#define INTCPS_MIR_CLEAR( n )			READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x88 + ( 0x20 * n ) ) 	// W
#define INTCPS_MIR_SET( n )				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x8C + ( 0x20 * n ) ) 	// W
#define INTCPS_ISR_SET( n )				READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x90 + ( 0x20 * n ) ) 	// RW
#define INTCPS_ISR_CLEAR( n )			READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x94 + ( 0x20 * n ) ) 	// W
#define INTCPS_PENDING_IRQ( n )			READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x98 + ( 0x20 * n ) ) 	// R
#define INTCPS_PENDING_FIQ( n )			READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x9C + ( 0x20 * n ) ) 	// R
// NOTE: m = 0 to 95
#define INTCPS_ILR( m )					READ_REGISTER_OFFSET( MPU_INTC_ADDR, 0x100 + ( 0x4 * m ) ) // RW
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// REGISTER-BIT PATTERNS /////////////////////////////////////////////////////////////////////////////////
#define INTCPS_CONTROL_NEWIRQAGR_BIT	0x1
#define INTCPS_CONTROL_NEWFIQAGR_BIT	0x2

#define INTCPS_SIR_IRQ_INTERRUPT_BITS 	127
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_IRQS 						96

irq_clbk callbacks[ MAX_IRQS ];

void
irqEnableIrq( uint32_t irqNumber )
{
	uint32_t n = ( irqNumber / 32 );
	uint32_t bit = irqNumber - ( n * 32 );
	INTCPS_MIR_CLEAR( n ) = 0x1 << bit;
}

void
irqResetIrq()
{
	BIT_SET( INTCPS_CONTROL, INTCPS_CONTROL_NEWIRQAGR_BIT );
}

void
irqResetFiq()
{
	BIT_SET( INTCPS_CONTROL, INTCPS_CONTROL_NEWFIQAGR_BIT );
}

uint32_t
irqGetCurrentInterrupt()
{
	uint32_t sirRegValue = INTCPS_SIR_IRQ;
	// keep bits 0-6 which contains the current interrupt-number
	BIT_KEEP( sirRegValue, INTCPS_SIR_IRQ_INTERRUPT_BITS );

	return sirRegValue;
}

uint32_t
irqInit()
{
	// TODO: read OMAP35x.pdf chapter about interrupts at page 1057

	return 0;
}

void
irqEnable()
{
	// enable IRQs AFTER we created the initial tasks
	// => when scheduling is called at least the idle-task is available
	_enable_IRQ();
}

void
irqRegisterClbk( irq_clbk clbk, uint32_t irqId )
{
	if ( irqId < MAX_IRQS )
	{
		irqEnableIrq( irqId );

		callbacks[ irqId ] = clbk;
	}
}

// NOTE: will be called by asm
uint32_t
irqHandler( UserContext* ctx )
{
	uint32_t irqNr = irqGetCurrentInterrupt();

	if ( callbacks[ irqNr] )
	{
		callbacks[ irqNr ]( ctx );
	}

	irqResetIrq();

	return 0;
}

#pragma INTERRUPT ( fiqHandler, FIQ );
interrupt
void fiqHandler()
{
	// implement when necessary
}

