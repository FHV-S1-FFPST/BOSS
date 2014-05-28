/*
 * mmu.h
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#ifndef MMU_H_
#define MMU_H_

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "../page_manager/pageManager.h"

uint32_t mmu_init( void );

/**
 * allocates a L1 entry for a given pid
 */
uint32_t mmu_allocateTask( uint32_t pid );
/**
 * maps the memory at mem with size of memSize to the address given at addr into
 * the space of task with pid by creating a new L2 region with the necessary number of pages.
 */
uint32_t mmu_map_memory( uint32_t pid, uint32_t addr, uint8_t* mem, uint32_t memSize );

#endif /* MMU_H_ */
