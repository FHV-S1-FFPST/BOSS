/*
 * irq.h
 *
 *  Created on: 13.04.2014
 *      Author: Jonathan Thaler
 */

#ifndef IRQ_H_
#define IRQ_H_

#include <inttypes.h>

typedef uint32_t ( *irq_clbk ) ( void );

// TODO: implement configuration of IRQ and FIRQ through irqtimer
// TODO: move irqtimer in this directory
// TODO: implement registration of function-pointers as IRQ callbacks

#define GPT2_IRQ 38

uint32_t irqInit();
void irqEnable();

void irqRegisterClbk( irq_clbk, uint32_t irqId );

#endif /* IRQ_H_ */
