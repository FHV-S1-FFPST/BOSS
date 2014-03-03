/*
 * core.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef CORE_H_
#define CORE_H_

#include <inttypes.h>

// defines the context data-structure of the kernel
typedef struct
{
	// TODO: kernel-data goes here
	uint8_t version;
} KernelContext;

// global kernel-context instance
KernelContext kernelContext;

int32_t initCore( void );

#endif /* CORE_H_ */
