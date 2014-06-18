/*
 * mmu.h
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#ifndef MMU_H_
#define MMU_H_

#include "../task/task.h"

uint32_t mmu_init( void );

/**
 * allocates a L1 entry for a given pid
 */
uint32_t* mmu_allocate_task();
/**
 * maps memory of of mapSize bytes to the address given at addr into
 * the space of task with pid by creating a new L2 region with the necessary number of pages.
 */
uint32_t mmu_map_memory( Task* task, uint32_t addr, uint32_t mapSize );

/**
 * resets the TTB to the address of MASTER PT
 */
void mmu_ttbReset();

void mmu_ttbSet(uint32_t ttbAddr);
void mmu_tlbFlush(void);
void mmu_setProcessID(uint8_t pid);
void mmu_domainAccessSet(uint32_t value, uint32_t mask);

#endif /* MMU_H_ */
