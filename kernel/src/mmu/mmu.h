/*
 * mmu.h
 *
 *  Created on: 18.04.2014
 *      Author: faisstm
 */

#ifndef MMU_H_
#define MMU_H_

typedef struct {

	uint32_t vAddress;
	uint32_t ptAdress;
	uint32_t masterPTAdress;
	uint32_t type;
	uint32_t dom;

} Pagetable_t;

void ttbSet(unsigned int ttb);
void tlbFlush(void);
void setProcessID(unsigned int pid);

#endif /* MMU_H_ */
