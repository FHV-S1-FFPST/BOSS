/*
 * mmu.c
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#include "mmu.h"

void mmu_initPagetable(Pagetable* pt);
void mmu_mapRegion(Region* reg);
void mmu_mapSectionTableRegion(Region* reg);

uint32_t mmu_init(void) {

	uint32_t ptArea = 0x80000000;

	Pagetable masterPT;
	masterPT.vAddress = 0x00000000;
	masterPT.ptAddress = 0x00100000;
	masterPT.ptAddressPhysical = ptArea;
	masterPT.type = MASTER;

	Region osRegion;
	osRegion.pageSize = 1024;
	osRegion.numPages = 32;
	osRegion.vAddress = 0x00600000;
	osRegion.physicalStartAdress = 0x80500000;
	osRegion.AP = ReadWriteNoAccess;
	osRegion.CB = WriteThrough;
	osRegion.ptType = MASTER;

	mmu_initPagetable(&masterPT);
	mmu_mapRegion(&osRegion);

	return 0;
}

// initializes Pagetables for Regions
void mmu_mapRegion(Region* reg) {

	switch(reg->ptType) {
	case MASTER:
		mmu_mapSectionTableRegion(reg);
		break;
	case COARSE:
		break;
	default:
		break;
	}

}

void mmu_mapSectionTableRegion(Region* reg) {
	uint32_t* master = (uint32_t *) 0x80000000;		// get master page table
	uint32_t tempVAdress = reg->vAddress;				// get start adress of region
	uint32_t tempPAdress = reg->physicalStartAdress;
	uint32_t PTE = 0;

	uint32_t i;

	for(i = 0; i < reg->numPages; i++) {					// iterate through all pages
		uint32_t index = (tempVAdress >> 20) & 0x00000FFF;	// get base of virtual adress [20:31]

		PTE = (tempPAdress & 0xFFF00000);
		PTE |= ( reg->AP << 10 );
		PTE |= ( 0x0 << 5);								// TODO: domain
		PTE |= ( reg->CB << 2);
		PTE |= 0x2;

		*(master + index) = PTE;
		tempVAdress += 0x100000;						// increase address to next index of virtual memory (1 MB)
		tempPAdress += 0x100000;						// increase address to next index of physical memory (1 MB)
	}
}

// initializes Pagetable with 0s
void mmu_initPagetable(Pagetable* pt) {

	int i = 0;
	uint32_t* PTE = (uint32_t *)(pt->ptAddressPhysical);

	switch(pt->type) {
	case MASTER:
		for(i = 0; i < 4096; i++) {
			*PTE++ = 0x00000000;
		}
		break;
	case COARSE:

		for(i = 0; i < 256; i++) {
			*PTE++ = 0x00000000;
		}

		break;
	default:
		break;
	}
}


