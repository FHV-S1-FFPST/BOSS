/*
 * pageManager.c
 *
 *  Created on: 27.05.2014
 *      Author: faisstm
 */

#include "pageManager.h"

Pagestatus stati[PAGE_COUNT];	// yes yes i know, stati is not the correct plural, but fuck that shit
uint32_t nextFreePageIndex;

uint8_t initPageManager(void) {
	memset(stati, 0, PAGE_COUNT * sizeof(Pagestatus));
	nextFreePageIndex = 0;

	return 0;
}

uint32_t* getFreePageAbstract(uint32_t numPages, uint32_t processID) {
		int i;
		uint32_t freePagesInRow = 0;
		uint32_t startIndexToAllocate = 0;
		uint32_t endIndexToAllocate = 0;

		for(i = nextFreePageIndex; i < PAGE_COUNT; ++i) {
			if(stati[i].inUse == 0) {

				if(freePagesInRow == 0) {
					startIndexToAllocate = i;
				}

				++freePagesInRow;

				if(freePagesInRow == numPages) {
					break;
				}
			} else {
				startIndexToAllocate = i;
				freePagesInRow = 0;
			}
		}

		if(freePagesInRow != numPages) {
			return 0;
		} else {
			endIndexToAllocate = startIndexToAllocate + numPages;
			for(i = startIndexToAllocate; i <= endIndexToAllocate; i++ ) {
				stati[i].inUse = 1;
				stati[i].processId = processID;
			}
		}

		nextFreePageIndex = startIndexToAllocate + numPages;
		return (uint32_t *)(PAGE_START_ADDR + startIndexToAllocate * 4096);
}

void freePageAbstract(uint32_t* page, uint32_t pageCount) {
	uint32_t index = (((uint32_t) page) - PAGE_START_ADDR) / 4096;
	int i;

	for(i = index; i < pageCount; i++) {
		stati[i].inUse = 0;
	}
}

uint32_t* getFree4KPage(uint32_t processID) {
	return getFreePageAbstract(1, processID);
}

uint32_t* getFree64KPage(uint32_t processID) {
	return getFreePageAbstract(16, processID);
}

uint32_t* getFree1MPage(uint32_t processID) {
	return getFreePageAbstract(256, processID);
}

void free4KPage(uint32_t* page) {
	freePageAbstract(page, 1);
}

void free64KPage(uint32_t *page) {
	freePageAbstract(page, 16);
}

void free1MPage(uint32_t* page) {
	freePageAbstract(page, 256);
}


