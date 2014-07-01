/*
 * mmu.c
 *
 *  Created on: 23.05.2014
 *      Author: faisstm
 */

#include "mmu.h"

#include "../common/common.h"

#include "../page_manager/pageManager.h"

// module-local defines
#define MASTER_PT_ADDR				0x80000000
#define MASTER_PT_VADDR				0x00000000

#define OS_REGION_VADDR				0x80500000
#define PERIPHERAL_REGION_VADDR		0x48000000
#define PAGETABLE_REGION_PADDR		0x80000000
#define SRAM_REGION_VADDR			0x40200000
#define SHARED_REGION_VADDR			0x82500000

#define PAGETABLE_SIZE				0x4000

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
	uint32_t* parentAddress;		// addresse der parent pagetable
	uint32_t vAddress;				// virtuelle Startadresse der Region
	uint32_t physicalStartAdress;	// physische Startadresse der Region
	uint32_t numPages;				// anzahl der pages in region
	uint16_t pageSize;				// page size
	AccessProtectionType AP;		// access permission
	CacheType CB;					// cache and write buffer attributes
	PageTableType ptType;
	MappingType mappingType;
	bool local;
} Region;
////////////////////////////////////////////////////////


// prototypes for functions implemented in asm
extern void _ttb_set(uint32_t ttbAddr);
extern void _tlb_flush(unsigned int c8format);
extern void _pid_set(uint32_t val);
extern void _mmu_setDomainAccess(unsigned int value, unsigned int mask);
extern void _mmu_init();
extern void _mmu_activate();
////////////////////////////////////////////////////////


// module-local function prototypes
void mmu_initPagetable(Pagetable* pt);
void mmu_mapRegion(Region* reg, uint8_t pid);
void mmu_mapSectionTableRegion(Region* reg, uint8_t pid);
void mmu_mapCoarseTableRegion(Region* reg, uint8_t pid);
////////////////////////////////////////////////////////


// Modul-local DATA
uint32_t* nextFreePT;

Pagetable _masterPT =
{
	.vAddress = MASTER_PT_VADDR,
	.ptAddress = MASTER_PT_ADDR,
	.ptAddressPhysical = MASTER_PT_ADDR,
	.type = MASTER,
	.domain = 3
};

Region _osRegion =
{
	.parentAddress = ( uint32_t* ) MASTER_PT_ADDR,
	.pageSize = 1024,
	.numPages = 32,
	.vAddress = OS_REGION_VADDR,
	.physicalStartAdress = OS_REGION_VADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = MASTER,
	.mappingType = Fixed,
	.local = FALSE
};

Region _peripheralRegion =
{
	.parentAddress = ( uint32_t* ) MASTER_PT_ADDR,
	.pageSize = 1024,
	.numPages = 896,
	.vAddress = PERIPHERAL_REGION_VADDR,
	.physicalStartAdress = PERIPHERAL_REGION_VADDR,
	.AP = ReadWriteTwiceBitch,
	.CB = NotCachedNotBuffered,
	.ptType = MASTER,
	.mappingType = Fixed,
	.local = FALSE
};

Region _pageTableRegion =
{
	.parentAddress = ( uint32_t* ) MASTER_PT_ADDR,
	.pageSize = 1024,
	.numPages = 5,
	.vAddress = PAGETABLE_REGION_PADDR,
	.physicalStartAdress = PAGETABLE_REGION_PADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = MASTER,
	.mappingType = Fixed,
	.local = FALSE
};

Region _sharedRegion = {
		.parentAddress = (uint32_t*) MASTER_PT_ADDR,
		.pageSize = 1024,
		.numPages = 32,
		.vAddress = SHARED_REGION_VADDR,
		.physicalStartAdress = SHARED_REGION_VADDR,
		.AP = ReadWriteTwiceBitch,
		.CB = WriteBack,
		.ptType = MASTER,
		.mappingType = Fixed,
		.local = false
};

Region _sramRegion =
{
	.parentAddress = ( uint32_t* ) MASTER_PT_ADDR,
	.pageSize = 1024,
	.numPages = 1,
	.vAddress = SRAM_REGION_VADDR,
	.physicalStartAdress = SRAM_REGION_VADDR,
	.AP = ReadWriteNoAccess,
	.CB = WriteBack,
	.ptType = MASTER,
	.mappingType = Fixed,
	.local = FALSE
};

Region _memMapRegionTemplate =
{
	.parentAddress = 0,
	.pageSize = 4,
	.numPages = 0,
	.vAddress = 0,
	.physicalStartAdress = 0,
	.AP = ReadWriteTwiceBitch,
	.CB = WriteBack,
	.ptType = COARSE,
	.mappingType = Dynamic,
	.local = TRUE
};
/////////////////////////////////////////////////////

uint32_t mmu_init(void) {

	nextFreePT = (uint32_t *) ( MASTER_PT_ADDR + PAGETABLE_SIZE );

	mmu_initPagetable(&_masterPT);

	mmu_mapRegion(&_osRegion, 0);
	mmu_mapRegion(&_peripheralRegion, 0);
	mmu_mapRegion(&_pageTableRegion, 0);
	mmu_mapRegion(&_sramRegion, 0);
	mmu_mapRegion(&_sharedRegion, 0);

	_ttb_set(MASTER_PT_ADDR);
	_mmu_init();
	_mmu_activate();

	return 0;
}

uint32_t*
mmu_allocate_task()
{
	uint32_t* taskL1Table = nextFreePT;
	nextFreePT += PAGETABLE_SIZE;

	// copy master pagetable
	memcpy( taskL1Table, ( void* ) MASTER_PT_ADDR, PAGETABLE_SIZE );

	return taskL1Table;
}

uint32_t
mmu_map_memory( Task* task, uint32_t addr, uint32_t mapSize )
{
	_memMapRegionTemplate.parentAddress = task->pageTable;
	_memMapRegionTemplate.numPages = ( mapSize / ( _memMapRegionTemplate.pageSize * 1024 ) );
	_memMapRegionTemplate.vAddress = addr;
	_memMapRegionTemplate.physicalStartAdress = addr;

	if ( mapSize % ( _memMapRegionTemplate.pageSize * 1024 ) )
	{
		_memMapRegionTemplate.numPages++;
	}

	mmu_mapRegion( &_memMapRegionTemplate, task->pid );

	return 0;
}

// initializes Pagetables for Regions
void mmu_mapRegion(Region* reg, uint8_t pid) {

	switch(reg->ptType) {
	case MASTER:
		mmu_mapSectionTableRegion(reg, pid);
		break;
	case COARSE:
		mmu_mapCoarseTableRegion(reg, pid);
		break;
	default:
		break;
	}
}

void mmu_mapSectionTableRegion(Region* reg, uint8_t pid) {
	uint32_t* master = ( uint32_t *) 0x80000000;		// get master page table
	uint32_t tempVAdress = reg->vAddress;				// get start address of region
	uint32_t tempPAdress = reg->physicalStartAdress;
	uint32_t PTE = 0;

	uint32_t i;

	for(i = 0; i < reg->numPages; ++i) {					// iterate through all pages
		uint32_t index = (tempVAdress >> 20) & 0x00000FFF;	// get base of virtual address [20:31]

		if( reg->mappingType == Dynamic) {
			tempPAdress = (uint32_t)getFree1MPage(pid);
		}

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
		}
	}
}

void mmu_mapCoarseTableRegion(Region* reg, uint8_t pid) {

	uint32_t* masterPT = (uint32_t *) reg->parentAddress;
	uint32_t numPageTables = (reg->numPages / 256) + 3;
	uint32_t i, j;

	uint32_t regVAdressTemp = reg->vAddress;
	uint32_t regPAdressTemp = reg->physicalStartAdress;

	uint32_t PTE = 0;
	uint32_t pagesToWrite = 0;
	uint32_t pagesWritenTotal = 0;

	for(i = 0; i < numPageTables; i++) {

		if(pagesWritenTotal == reg->numPages) {
			break;
		}

		uint32_t indexInL1 = (regVAdressTemp >> 20) & 0x00000FFF;
		uint32_t tempL2Adress;

		if((masterPT[indexInL1] & 0x00000003) == 0x2) {
			tempL2Adress = masterPT[indexInL1] & 0xFFFFFC00;
		} else {
			Pagetable tempTable;
			tempTable.ptAddress = (uint32_t) nextFreePT;
			tempTable.ptAddressPhysical = tempTable.ptAddress;
			tempTable.type = COARSE;
			tempTable.domain = 3;
			mmu_initPagetable(&tempTable);
			tempL2Adress = tempTable.ptAddress;

			PTE = 0;
			PTE = (tempTable.ptAddress & 0xFFFFFC00);
			PTE |= tempTable.domain << 5;
			PTE |= reg->local << 17; // if local -> TLB will use processid -> no need to flush TLB
			PTE |= 0x2;

			masterPT[indexInL1] = PTE;
			nextFreePT += PAGETABLE_SIZE; // TODO: fix it. need only 0x400 but add 16k to be aligned to 16k;
		}

		if(reg->numPages - pagesWritenTotal > 256) {
			pagesToWrite = 256;
		} else {
			pagesToWrite = reg->numPages - pagesWritenTotal;
		}

		// write l2 table
		for(j = 0; j < pagesToWrite; j++) {
			PTE = 0;
			uint32_t l2Index = (regVAdressTemp & 0x000FF000) >> 12;

			if(reg->mappingType == Fixed) {
				PTE = regPAdressTemp & 0xFFFFF000;
				regPAdressTemp += 0x1000;
			} else {

				if((((uint32_t *)tempL2Adress)[l2Index] & 0x00000003) == 0x2) {
					regVAdressTemp += 0x1000;
					++pagesWritenTotal;
					continue;
				} else {
					PTE = ((uint32_t)getFree4KPage(pid)) & 0xFFFFF000;
					++pagesWritenTotal;
				}
			}

			PTE |= reg->AP << 10;
			PTE |= reg->AP << 8;
			PTE |= reg->AP << 6;
			PTE |= reg->AP << 4;
			PTE |= reg->CB << 2;
			PTE |= 0x2;

			((uint32_t *)tempL2Adress)[l2Index] = PTE;
			regVAdressTemp += 0x1000;

			if(l2Index == 255) {
				break;
			}

		}

	}

	pagesWritenTotal++;
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

void mmu_ttbReset() {
	mmu_ttbSet( MASTER_PT_ADDR );
}

void mmu_domainAccessSet(uint32_t value, uint32_t mask) {
	_mmu_setDomainAccess(value, mask);
}

void mmu_ttbSet( uint32_t ttb) {
	ttb &= 0xffffc000;
	_ttb_set(ttb);
}

void mmu_tlbFlush(void) {
	_tlb_flush( 0 );
}

void mmu_setProcessID(uint8_t pid) {
	uint32_t val = pid << 25;

	_pid_set( val );
}
