/*
 * core.h
 *
 *  Created on: 26.02.2014
 *      Author: Thaler
 */

#ifndef CORE_H_
#define CORE_H_

// defines the context data-structure of the kernel
typedef struct
{
	// TODO: kernel-data goes here
	char version;
} KernelContext;

// global kernel-context instance
KernelContext kernelContext;

int initCore( void );

#endif /* CORE_H_ */
