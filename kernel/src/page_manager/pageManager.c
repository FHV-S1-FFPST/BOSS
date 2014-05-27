/*
 * pageManager.c
 *
 *  Created on: 27.05.2014
 *      Author: faisstm
 */

#include "pageManager.h"

Pagestatus stati[PAGE_COUNT];	// yes yes i know, stati is not the correct plural, but fuck that shit

uint8_t initPageManager(void) {
	volatile int size = sizeof(Pagestatus);
	memset(stati, 0, PAGE_COUNT * sizeof(Pagestatus));

	return 0;
}

uint32_t getFreePage(uint32_t processID) {

	int i;

	for(i = 0; i < PAGE_COUNT; i++) {
		if(stati[i].inUse == 0) {
			stati[i].inUse = 1;
			stati[i].processId = processID;
			return (0x84500000 + i * 4096);		// 0x84500000 is start Address (physical) of page area + 4096 (4K) per page
		}
	}

	return 0;
}

