/*
 * mmu.c
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#include "mmu.h"

extern void _ttb_set(unsigned int ttb);
extern void _tlb_flush(unsigned int c8format);
extern void _pid_set(unsigned int pid);
extern void _mmu_initPT(unsigned int PTE, unsigned int index, unsigned int * PTEptr);
extern void _mmu_setDomainAccess(unsigned int value, unsigned int mask);
extern void _control_set(unsigned int value, unsigned int mask);
extern void _ttbr_set_ctrl_bits(unsigned int bits);
extern void _ttbr_enable_write_buffer();
extern void test();
extern void _mmu_activate();

#define MASTER_PT 0x80000000


void mmu_initPagetable(Pagetable* pt);
void mmu_mapRegion(Region* reg);
void mmu_mapSectionTableRegion(Region* reg);

void ttbSet(unsigned int ttb);
void tlbFlush(void);
void setProcessID(unsigned int pid);
void domainAccessSet(uint32_t value, uint32_t mask);

uint32_t mmu_init(void) {

	uint32_t ptArea = 0x80000000;

	Pagetable masterPT;
	masterPT.vAddress = 0x00000000;
	masterPT.ptAddress = MASTER_PT;
	masterPT.ptAddressPhysical = ptArea;
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

	Region peripheralRegion;
	peripheralRegion.pageSize = 1024;
	peripheralRegion.numPages = 896;
	peripheralRegion.vAddress = 0x48000000;
	peripheralRegion.physicalStartAdress = 0x48000000;
	peripheralRegion.AP = ReadWriteNoAccess;
	peripheralRegion.CB = NotCachedNotBuffered;
	peripheralRegion.ptType = MASTER;

	Region pageTableRegion;
	pageTableRegion.pageSize = 1024;
	pageTableRegion.numPages = 5;
	pageTableRegion.vAddress = MASTER_PT;
	pageTableRegion.physicalStartAdress = 0x80000000;
	pageTableRegion.AP = ReadWriteNoAccess;
	pageTableRegion.CB = WriteBack;
	pageTableRegion.ptType = MASTER;

	Region sramRegion;
	sramRegion.pageSize = 1024;
	sramRegion.numPages = 1;
	sramRegion.vAddress = 0x40200000;
	sramRegion.physicalStartAdress = 0x40200000;
	sramRegion.AP = ReadWriteNoAccess;
	sramRegion.CB = WriteBack;
	sramRegion.ptType = MASTER;

	mmu_initPagetable(&masterPT);

	mmu_mapRegion(&osRegion);
	mmu_mapRegion(&peripheralRegion);
	mmu_mapRegion(&pageTableRegion);
	mmu_mapRegion(&sramRegion);

	/*_ttbr_set_ctrl_bits(0x0); //First 3 bits != null OS/HW call else VM
	_ttbr_enable_write_buffer();

	_ttb_set((uint32_t) 0x80000000, 0);

	domainAccessSet(DOM3CLT, CHANGEALLDOM);

	uint32_t enable = ENABLEMMU | 0x08;
	uint32_t change = CHANGEMMU | 0x08;

	_control_set(enable, change);*/

	_ttb_set(MASTER_PT);
	test();
	_mmu_activate();

	uint32_t *testP = (uint32_t *)0x00100000;
	*testP = 5;

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
	uint32_t tempVAdress = reg->vAddress;				// get start address of region
	uint32_t tempPAdress = reg->physicalStartAdress;
	uint32_t PTE = 0;

	uint32_t i;

	for(i = 0; i < reg->numPages; i++) {					// iterate through all pages
		uint32_t index = (tempVAdress >> 20) & 0x00000FFF;	// get base of virtual address [20:31]

		PTE = (tempPAdress & 0xFFF00000);
		PTE |= ( reg->AP << 10 );
		PTE |= ( 0x03 << 5);								// TODO: domain
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

