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

typedef enum {
	FAULT,
	MASTER,
	COARSE
} PageTableType;

typedef enum {
	NoAccessTwiceBitch = 0,
	ReadWriteNoAccess,
	ReadWriteReadOnly,
	ReadWriteTwiceBitch
} AccessProtectionType;

typedef enum {
	NotCachedNotBuffered = 0,
	NotCachedBuffered,
	WriteThrough,
	WriteBack
} CacheType;

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

typedef struct {
	uint32_t vAddress;				// virtuelle Startadresse des Bereiches den diese Pagetable übernimmt
	uint32_t ptAddress;				// virtuelle Adresse der Pagetable
	uint32_t ptAddressPhysical;		// physische Adresse der Pagetable
	PageTableType type;					// pagetable type
	uint8_t domain;
} Pagetable;

typedef struct {
	uint32_t vAddress;				// virtuelle Startadresse der Region
	uint32_t physicalStartAdress;	// physische Startadresse der Region
	uint16_t pageSize;				// page size
	uint16_t numPages;				// anzahl der pages in region
	AccessProtectionType AP;		// access permission
	CacheType CB;					// cache and write buffer attributes
	PageTableType ptType;
} Region;

uint32_t mmu_init(void);

#endif /* MMU_H_ */
