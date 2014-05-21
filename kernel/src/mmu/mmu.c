/*
 * mmu.c
 *
 *  Created on: 18.04.2014
 *      Author: faisstm
 */

/* asm methods */

#include "mmu.h"

extern void _ttb_set(unsigned int ttb);
extern void _tlb_flush(unsigned int c8format);
extern void _pid_set(unsigned int pid);
extern void _mmu_initPT(unsigned int PTE, unsigned int index, unsigned int * PTEptr);
extern void _mmu_setDomainAccess(unsigned int value, unsigned int mask);
extern void _control_set(unsigned int value, unsigned int mask);

void mmuInitPT(Pagetable_t *pt);
uint8_t mmuMapSectionTableRegion(Region_t *region);
uint8_t mmuMapCoarseTableRegion(Region_t *region);
uint32_t mmuAttachPT(Pagetable_t *pt);
void domainAccessSet(uint32_t value, uint32_t mask);

Pagetable_t masterPT = { 0x00000000, 0x80018000, 0x80018000, MASTER, 3 };
Pagetable_t systemPT = { 0x00000000, 0x8001C000, 0x80018000, COARSE, 3 };
Pagetable_t task1PT =  { 0x00400000, 0x8001C400, 0x80018000, COARSE, 3 };

Region_t kernelRegion = { 0x00000000, SMALLPAGE, 16, RWNA, WT, 0x00000000, &systemPT };
Region_t sharedRegion = { 0x00010000, SMALLPAGE, 8, RWRW, WT, 0x00010000, &systemPT };
Region_t pageTableRegion = { 0x00018000, SMALLPAGE, 8, RWNA, WT, 0x00018000, &systemPT };

uint32_t mmuInit(void) {

	uint32_t enable;
	uint32_t change;

	mmuInitPT(&masterPT);
	mmuInitPT(&systemPT);

	mmuMapRegion(&kernelRegion);
	mmuMapRegion(&sharedRegion);
	mmuMapRegion(&pageTableRegion);

	mmuAttachPT(&masterPT);
	mmuAttachPT(&systemPT);

	domainAccessSet(DOM3CLT, CHANGEALLDOM);

	enable = ENABLEMMU | ENABLEICACHE | ENABLEDCACHE;
	change = CHANGEMMU | CHANGEICACHE | CHANGEDCACHE;

	_control_set(enable, change);

	return 0;
}

void domainAccessSet(uint32_t value, uint32_t mask) {
	_mmu_setDomainAccess(value, mask);
}

void mmuInitPT(Pagetable_t *pt) {
	int index;
	uint32_t PTE = 0;
	uint32_t *PTEptr = 0;

	PTEptr = (uint32_t *)pt->ptAddress;
	PTE = FAULT;

	switch(pt->type) {
	case COARSE:
		index = COARSE_NR_OF_PTES / 32;	// durch 32 weil in der asm function pro schleifendurchlauf 32 ptes geschrieben werden
		break;
	case MASTER:
		index = MASTER_NR_OF_PTES / 32;
		break;
	default:
		break;
	}

	_mmu_initPT(PTE, index, PTEptr);

}

uint8_t mmuMapRegion(Region_t * region) {
	switch(region->pt->type) {

	case MASTER:
		return mmuMapSectionTableRegion(region);

	case COARSE:
		return mmuMapCoarseTableRegion(region);

	default:
		return 1;
	}
}

uint8_t mmuMapSectionTableRegion(Region_t *region) {

	int i;
	uint32_t PTE;
	uint32_t *PTEptr;

	PTEptr = (uint32_t *) region->pt->ptAddress;
	PTEptr += region->vAddress >> 20;
	PTEptr += region->pageSize - 1;

	PTE = region->pAddress & 0xFFF00000;  			// Set base address [20:31]
	PTE |= region->AP << 10;						// Set access protection [10:11]
	PTE |= region->pt->dom << 5;					// Set domain [5:8]
	PTE |= region->CB << 2;							// Set cached/buffered [2:3]
	PTE |= 0x2;

	for(i = region->numPages - 1; i >= 0; i--) {
		*PTEptr-- = PTE + (i << 20);
	}

	return 0;

}

uint8_t mmuMapCoarseTableRegion(Region_t *region) {

	int i,j ;
	uint32_t PTE;
	uint32_t *PTEptr;

	PTEptr = (uint32_t *) region->pt->ptAddress;

	switch(region->pageSize) {
	case LARGEPAGE:
		PTEptr += (region->vAddress & 0x000FF000) >> 12;
		PTEptr += (region->numPages * 16) - 1;

		PTE = region->pAddress & 0xFFFF0000;	// set base physical address [16:31]
		PTE |= region->AP << 10;				// set accession protection bits [2:11]
		PTE |= region->AP << 8;
		PTE |= region->AP << 6;
		PTE |= region->AP << 4;
		PTE |= region->CB << 2;
		PTE |= 0x1;								// set type large page

		// write pte 16 times for 64 kb page
		for(i = region->numPages - 1; i >= 0; i--) {
			for(j = 15; j >= 0; j--) {
				*PTEptr-- = PTE + (i << 16);
			}
		}
		break;

	case SMALLPAGE:
		PTEptr += (region->vAddress & 0x000FF000) >> 12;
		PTEptr += region->numPages - 1;

		PTE = region->pAddress & 0xFFFFF000; 	// set base physical address [12:31]
		PTE |= region->AP << 10;				// set accession protection bits [2:11]
		PTE |= region->AP << 8;
		PTE |= region->AP << 6;
		PTE |= region->AP << 4;
		PTE |= region->CB << 2;
		PTE |= 0x2;

		for(i = region->numPages - 1; i >= 0; i--) {
			*PTEptr-- = PTE + (i << 12);
		}

		break;

	default:
		return 1;
	}

	return 0;
}

uint32_t mmuAttachPT(Pagetable_t *pt) {

	uint32_t *ttb;
	uint32_t PTE;
	uint32_t offset;

	ttb = (uint32_t *) pt->masterPTAdress;
	offset = (pt->vAddress) >> 20;

	switch(pt->type) {
	case MASTER:
		_ttb_set((uint32_t) ttb);
		break;
	case COARSE:
		PTE = (pt->ptAddress & 0xFFFFFC00);
		PTE |= pt->dom << 5;
		PTE |= 0x1;
		ttb[offset] = PTE;
		break;

	default:
		return 1;
	}

	return 0;
}

void ttbSet(unsigned int ttb) {
	ttb &= 0xffffc000;
	_ttb_set(ttb);
}

void tlbFlush(void) {
	unsigned int c8format = 0;
	_tlb_flush(c8format);
}

void setProcessID(unsigned int pid) {
	pid = pid << 25;
	_pid_set(pid);
}



