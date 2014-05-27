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

#define DOM3CLT 0xC0
#define CHANGEALLDOM 0xFFFFFFFF

#define ENABLEMMU 0x0001
#define ENABLEDCACHE 0x0004
#define ENABLEICACHE 0x1000
#define CHANGEMMU 0x0001
#define CHANGEDCACHE 0x0004
#define CHANGEICACHE 0x1000

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
	uint16_t pageSize;				// page size
	uint16_t numPages;				// anzahl der pages in region
	AccessProtectionType AP;		// access permission
	CacheType CB;					// cache and write buffer attributes
	PageTableType ptType;
} Region;

uint32_t mmu_init(void);

#endif /* MMU_H_ */
