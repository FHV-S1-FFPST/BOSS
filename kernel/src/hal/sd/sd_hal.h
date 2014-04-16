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

#endif /* SD_HAL_H_ */
