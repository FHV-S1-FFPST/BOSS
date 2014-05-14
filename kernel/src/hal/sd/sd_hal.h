/*
 * sd_hal.h
 *
 *  Created on: 16.04.2014
 *      Author: Thaler
 */

#ifndef SD_HAL_H_
#define SD_HAL_H_

#include <inttypes.h>

uint32_t sdHalInit();
uint32_t sdHalReadBytes( uint32_t* address, uint32_t* buffer, uint32_t bufferSize );

typedef struct {
	uint32_t ocr;
	uint32_t raw_cid[ 4 ];
	uint32_t raw_csd[ 4 ];
	uint16_t rca;
	uint8_t highCap;
	uint32_t tranSpeed;
	uint32_t blkLen;
	uint32_t size;
	uint32_t nBlks;
} CARD_INFO;

#endif /* SD_HAL_H_ */
