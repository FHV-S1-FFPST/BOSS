/*
 * driver.h
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */

#ifndef DRIVER_H_
#define DRIVER_H_

#include "ioctl.h"
#include "return_code.h"

typedef struct driver {
	RETURN_CODE (*open) (struct driver *self);
	RETURN_CODE (*ioctl) (struct driver *self, enum IOCTL_CMD cmd, void *data);
	RETURN_CODE (*write) (struct driver *self, void *data);
	void* (*read) (struct driver *self);
} Driver_t;

enum DRIVER_TYPE {
	SERIAL_DRIVER = 0,
	HDMI_DRIVER = 1
};

#endif /* DRIVER_H_ */
