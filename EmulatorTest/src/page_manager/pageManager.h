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
#define PAGE_START_ADDR 0x82500000

typedef struct {
	uint32_t inUse;
	uint32_t processId;
} Pagestatus;

uint8_t initPageManager(void);
uint32_t* getFree4KPage(uint32_t processID);
uint32_t* getFree64KPage(uint32_t processID);
uint32_t* getFree1MPage(uint32_t processID);

void free4KPage(uint32_t* page);
void free64KPage(uint32_t *page);
void free1MPage(uint32_t* page);

#endif /* PAGEMANAGER_H_ */
