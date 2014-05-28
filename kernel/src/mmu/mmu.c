/*
 * mmu.c
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#include "mmu.h"

// module-local defines
#define MASTER_PT 					0x80000000

#define MASTER_PT_VADDR				0x00000000

#define OS_REGION_VADDR				0x80500000
#define PERIPHERAL_REGION_VADDR		0x48000000
#define PAGETABLE_REGION_PADDR		0x80000000
#define SRAM_REGION_VADDR			0x40200000
#define TASK_REGION_VADDR			0x02100000

#define DOM3CLT 					0xC0
#define CHANGEALLDOM				0xFFFFFFFF

#define ENABLEMMU 					0x0001
#define ENABLEDCACHE 				0x0004
#define ENABLEICACHE 				0x1000
#define CHANGEMMU 					0x0001
#define CHANGEDCACHE 				0x0004
#define CHANGEICACHE 				0x1000
////////////////////////////////////////////////////////


// module-local structures
typedef enum {
	FAULT = 0,
	MASTER = 1,
	COARSE = 2
} PageTableType;

typedef enum {
	NoAccessTwiceBitch = 0,
	ReadWriteNoAccess = 1,
	ReadWriteReadOnly = 2,
	ReadWriteTwiceBitch = 3
} AccessProtectionType;

typedef enum {
	NotCachedNotBuffered = 0,
	NotCachedBuffered = 1,
	WriteThrough = 2,
	WriteBack = 3
} CacheType;

typedef enum {
	Fixed = 0,
	Dynamic = 1
} MappingType;

typedef struct {
	uint32_t vAddress;				// virtuelle Startadresse des Bereiches den diese Pagetable übernimmt
	uint32_t ptAddress;				// virtuelle Adresse der Pagetable
	uint32_t ptAddressPhysical;		// physische Adresse der Pagetable
	PageTableType type;				// pagetable type
	uint8_t domain;
} Pagetable;

typedef struct {
	uint32_t vAddress;				// virtuelle Startadresse der Region
	uint32_t physicalStartAdress;	// physische Startadresse der Region
	uint32_t numPages;				// anzahl der pages in region
	uint16_t pageSize;				// page size
	AccessProtectionType AP;		// access permission
	CacheType CB;					// cache and write buffer attributes
	PageTableType ptType;
	MappingType mappingType;
} Region;
////////////////////////////////////////////////////////


// prototypes for functions implemented in asm
extern void _ttb_set(unsigned int ttb);
extern void _tlb_flush(unsigned int c8format);
extern void _pid_set(unsigned int pid);
extern void _mmu_setDomainAccess(unsigned int value, unsigned int mask);
extern void _mmu_init();
extern void _mmu_activate();
////////////////////////////////////////////////////////


// module-local function prototypes
void mmu_initPagetable(Pagetable* pt);
void mmu_mapRegion(Region* reg, uint32_t processID);
void mmu_mapSectionTableRegion(Region* reg, uint32_t processID);
void mmu_mapCoarseTableRegion(Region* reg, uint32_t processID);

void ttbSet(unsigned int ttb);
void tlbFlush(void);
void setProcessID(unsigned int pid);
void domainAccessSet(uint32_t value, uint32_t mask);
////////////////////////////////////////////////////////


// Modul-local DATA
uint32_t* nextFreePT;

Pagetable _masterPT = {
	.vAddress = MASTER_PT_VADDR,
	.ptAddress = MASTER_PT,
	.ptAddressPhysical = MASTER_PT,
	.type = MASTER,
	.domain = 3
};

Region _osRegion =
{
	.pageSize = 1024,
	.numPages = 32,
	.vAddress = OS_REGION_VADDR,
	.physicalStartAdress = OS_REGION_VADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = MASTER,
	.mappingType = Fixed
};

Region _peripheralRegion =
{
	.pageSize = 1024,
	.numPages = 896,
	.vAddress = PERIPHERAL_REGION_VADDR,
	.physicalStartAdress = PERIPHERAL_REGION_VADDR,
	.AP = ReadWriteNoAccess,
	.CB = NotCachedNotBuffered,
	.ptType = MASTER,
	.mappingType = Fixed
};

Region _pageTableRegion =
{
	.pageSize = 1024,
	.numPages = 5,
	.vAddress = MASTER_PT,
	.physicalStartAdress = PAGETABLE_REGION_PADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = MASTER,
	.mappingType = Fixed
};

Region _sramRegion =
{
	.pageSize = 4,
	.numPages = 16,
	.vAddress = SRAM_REGION_VADDR,
	.physicalStartAdress = SRAM_REGION_VADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = COARSE,
	.mappingType = Fixed
};

Region _taskRegionTemplate =
{
	.pageSize = 1024,
	.numPages = 1,
	.vAddress = TASK_REGION_VADDR,
	.physicalStartAdress = TASK_REGION_VADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = MASTER,
	.mappingType = Dynamic
};

Region _memMapRegionTemplate =
{
	.pageSize = 4,
	.numPages = 0,
	.vAddress = 0,
	.physicalStartAdress = 0,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = COARSE,
	.mappingType = Dynamic
};
/////////////////////////////////////////////////////

uint32_t mmu_init(void) {

	nextFreePT = (uint32_t *) 0x80004000;

	mmu_initPagetable(&_masterPT);

	mmu_mapRegion(&_osRegion, 0);
	mmu_mapRegion(&_peripheralRegion, 0);
	mmu_mapRegion(&_pageTableRegion, 0);
	mmu_mapRegion(&_sramRegion, 0);

	_ttb_set(MASTER_PT);
	_mmu_init();
	_mmu_activate();

	return 0;
}

uint32_t
mmu_allocateTask( uint32_t pid )
{
	mmu_mapRegion( &_taskRegionTemplate, pid );

	return 0;
}

uint32_t
mmu_map_memory( uint32_t pid, uint32_t addr, uint8_t* mem, uint32_t memSize )
{
	_memMapRegionTemplate.numPages = ( memSize / ( _memMapRegionTemplate.pageSize * 1024 ) );
	_memMapRegionTemplate.vAddress = addr;
	_memMapRegionTemplate.physicalStartAdress = addr;

	if ( memSize % ( _memMapRegionTemplate.pageSize * 1024 ) )
	{
		_memMapRegionTemplate.numPages++;
	}

	mmu_mapRegion( &_memMapRegionTemplate, pid );

	uint8_t* pAddr = ( uint8_t* ) addr;

	memcpy( pAddr, mem, memSize );

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

	for(i = 0; i < reg->numPages; ++i) {					// iterate through all pages

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

