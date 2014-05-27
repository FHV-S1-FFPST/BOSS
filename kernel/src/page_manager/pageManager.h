/*
 * pageManager.h
 *
 *  Created on: 27.05.2014
 *      Author: faisstm
 */

#ifndef PAGEMANAGER_H_
#define PAGEMANAGER_H_

#include <string.h>
#include <inttypes.h>

#define PAGE_COUNT 131072

typedef struct {
	uint32_t inUse;
	uint32_t processId;
} Pagestatus;

uint8_t initPageManager(void);
uint32_t getFreePage(uint32_t processID);

#endif /* PAGEMANAGER_H_ */
