/*
 * mmu.h
 *
 *  Created on: 18.04.2014
 *      Author: faisstm
 */

#ifndef MMU_H_
#define MMU_H_

#include <inttypes.h>

#define FAULT 0
#define COARSE 1
#define MASTER 2

#define COARSE_NR_OF_PTES 256
#define MASTER_NR_OF_PTES 4096

/* First 2 chars for privileged mode, second 2 chars for user mode */
/* NA = no access, RO = read only, RW = read and write */
#define NANA 0x00
#define RWNA 0x01
#define RWRO 0x02
#define RWRW 0x03

#define NCNB 0x00	// not cached, not buffered
#define NCB 0x01	// not cached, buffered
#define WT 0x02		// write through cached
#define WB 0x03		// write back cached

#define LARGEPAGE 64
#define SMALLPAGE 4

#define DOM3CLT 0x00000040
#define CHANGEALLDOM 0xFFFFFFFF

#define ENABLEMMU 0x0001
#define ENABLEDCACHE 0x0004
#define ENABLEICACHE 0x1000
#define CHANGEMMU 0x0001
#define CHANGEDCACHE 0x0004
#define CHANGEICACHE 0x1000


typedef struct {

	uint32_t vAddress; 			// virtual Address
	uint32_t ptAddress; 		// address of page table
	uint32_t masterPTAdress; 	// address of master pagetable, if l1 table this = ptAddress
	uint32_t type; 				// coarse, master or fault
	uint32_t dom; 				// domain

} Pagetable_t;

typedef struct {
	uint32_t vAddress; 	// virtual Address
	uint32_t pageSize;  // size of virtual page
	uint32_t numPages;	// number of pages
	uint32_t AP;		// access permission
	uint32_t CB;		// cache and write buffer attributes
	uint32_t pAddress;	// virtual Address
	Pagetable_t *pt;	// pointer to page table
} Region_t;

uint32_t mmuInit(void);
uint8_t mmuMapRegion(Region_t * region);

/* ttb Functions */
void ttbSet(unsigned int ttb);
void tlbFlush(void);
void setProcessID(unsigned int pid);

#endif /* MMU_H_ */
