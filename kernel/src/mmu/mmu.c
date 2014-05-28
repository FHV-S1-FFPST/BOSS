/*
 * mmu.c
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#include "mmu.h"

#define MASTER_PT 0x80000000

extern void _ttb_set(unsigned int ttb);
extern void _tlb_flush(unsigned int c8format);
extern void _pid_set(unsigned int pid);
extern void _mmu_setDomainAccess(unsigned int value, unsigned int mask);
extern void _mmu_init();
extern void _mmu_activate();

void mmu_initPagetable(Pagetable* pt);
void mmu_mapRegion(Region* reg, uint32_t processID);
void mmu_mapSectionTableRegion(Region* reg, uint32_t processID);
void mmu_mapCoarseTableRegion(Region* reg, uint32_t processID);

void ttbSet(unsigned int ttb);
void tlbFlush(void);
void setProcessID(unsigned int pid);
void domainAccessSet(uint32_t value, uint32_t mask);

uint32_t *nextFreePT;

uint32_t mmu_init(void) {

	nextFreePT = (uint32_t *) 0x80004000;

	Pagetable masterPT;
	masterPT.vAddress = 0x00000000;
	masterPT.ptAddress = MASTER_PT;
	masterPT.ptAddressPhysical = MASTER_PT;
	masterPT.type = MASTER;
	masterPT.domain = 3;

	Region osRegion;
	osRegion.pageSize = 1024;
	osRegion.numPages = 32;
	osRegion.vAddress = 0x80500000;
	osRegion.physicalStartAdress = 0x80500000;
	osRegion.AP = ReadWriteNoAccess;
	osRegion.CB = WriteBack;
	osRegion.ptType = MASTER;
	osRegion.mappingType = Fixed;

	Region peripheralRegion;
	peripheralRegion.pageSize = 1024;
	peripheralRegion.numPages = 896;
	peripheralRegion.vAddress = 0x48000000;
	peripheralRegion.physicalStartAdress = 0x48000000;
	peripheralRegion.AP = ReadWriteNoAccess;
	peripheralRegion.CB = NotCachedNotBuffered;
	peripheralRegion.ptType = MASTER;
	peripheralRegion.mappingType = Fixed;

	Region pageTableRegion;
	pageTableRegion.pageSize = 1024;
	pageTableRegion.numPages = 5;
	pageTableRegion.vAddress = MASTER_PT;
	pageTableRegion.physicalStartAdress = 0x80000000;
	pageTableRegion.AP = ReadWriteNoAccess;
	pageTableRegion.CB = WriteBack;
	pageTableRegion.ptType = MASTER;
	pageTableRegion.mappingType = Fixed;

	Region sramRegion;
	sramRegion.pageSize = 4;
	sramRegion.numPages = 16;
	sramRegion.vAddress = 0x40200000;
	sramRegion.physicalStartAdress = 0x40200000;
	sramRegion.AP = ReadWriteNoAccess;
	sramRegion.CB = WriteBack;
	sramRegion.ptType = COARSE;
	sramRegion.mappingType = Fixed;

	mmu_initPagetable(&masterPT);

	mmu_mapRegion(&osRegion, 0);
	mmu_mapRegion(&peripheralRegion, 0);
	mmu_mapRegion(&pageTableRegion, 0);
	mmu_mapRegion(&sramRegion, 0);

	_ttb_set(MASTER_PT);
	_mmu_init();
	_mmu_activate();

	return 0;
}

// initializes Pagetables for Regions
void mmu_mapRegion(Region* reg, uint32_t processID) {

	switch(reg->ptType) {
	case MASTER:
		mmu_mapSectionTableRegion(reg, processID);
		break;
	case COARSE:
		mmu_mapCoarseTableRegion(reg, processID);
		break;
	default:
		break;
	}

}

void addProcess() {

}

void mmu_mapCoarseTableRegion(Region* reg, uint32_t processID) {

	uint32_t *masterPT = (uint32_t *) 0x80000000;
	uint32_t numPageTables = (reg->numPages / 256) + 1;
	uint32_t i, j;

	uint32_t regVAdressTemp = reg->vAddress;
	uint32_t regPAdressTemp = reg->physicalStartAdress;

	uint32_t PTE = 0;
	uint32_t pagesToWrite = 0;


	for(i = 0; i < numPageTables; i++) {
		Pagetable tempTable;
		tempTable.ptAddress = (uint32_t) nextFreePT;
		tempTable.ptAddressPhysical = tempTable.ptAddress;
		tempTable.type = COARSE;
		tempTable.domain = 3;

		mmu_initPagetable(&tempTable);

		// write entry in l1 table that points to l2 table
		uint32_t indexInL1 = (regVAdressTemp >> 20) & 0x00000FFF;

		PTE = 0;
		PTE = (tempTable.ptAddress & 0xFFFFFC00);
		PTE |= tempTable.domain << 5;
		PTE |= 0x1;

		masterPT[indexInL1] = PTE;
		nextFreePT += 0x400;

		if(numPageTables == i + 1) {
			pagesToWrite = reg->numPages % 256;
		} else {
			pagesToWrite = 256;
		}

		// write l2 table
		for(j = 0; j < pagesToWrite; j++) {
			PTE = 0;

			if(reg->mappingType == Fixed) {
				PTE = regPAdressTemp & 0xFFFFF000;
				regPAdressTemp += 0x1000;
			} else {
				PTE = ((uint32_t)getFree4KPage(processID)) & 0xFFFFF000;
			}

			PTE |= reg->AP << 10;
			PTE |= reg->AP << 8;
			PTE |= reg->AP << 6;
			PTE |= reg->AP << 4;
			PTE |= reg->CB << 2;
			PTE |= 0x2;

			((uint32_t *)tempTable.ptAddress)[j] = PTE;

		}

		regVAdressTemp += 0x100000;
	}


}

void mmu_mapSectionTableRegion(Region* reg, uint32_t processID) {
	uint32_t* master = ( uint32_t *) 0x80000000;		// get master page table
	uint32_t tempVAdress = reg->vAddress;				// get start address of region
	uint32_t tempPAdress;
	uint32_t PTE = 0;

	uint32_t i;


	if(reg->mappingType == Fixed) {
		tempPAdress = reg->physicalStartAdress;
	} else {
		tempPAdress = (uint32_t)getFree1MPage(processID);
	}

	for(i = 0; i < reg->numPages; i++) {					// iterate through all pages

		uint32_t index = (tempVAdress >> 20) & 0x00000FFF;	// get base of virtual address [20:31]

		PTE = 0;
		PTE = (tempPAdress & 0xFFF00000);
		PTE |= ( reg->AP << 10 );
		PTE |= ( 0x03 << 5);								// TODO: domain
		PTE |= ( reg->CB << 2);
		PTE |= 0x2;

		master[ index ] = PTE;
		tempVAdress += 0x100000;						// increase address to next index of virtual memory (1 MB)

		if(reg->mappingType == Fixed) {
			tempPAdress += 0x100000;						// increase address to next index of physical memory (1 MB)
		} else {
			tempPAdress = (uint32_t)getFree1MPage(processID);
		}
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

void domainAccessSet(uint32_t value, uint32_t mask) {
	_mmu_setDomainAccess(value, mask);
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

