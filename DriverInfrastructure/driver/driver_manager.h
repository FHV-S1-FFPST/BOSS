/*
 * driver_manager.h
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */

#ifndef DRIVER_MANAGER_H_
#define DRIVER_MANAGER_H_

#include "serial/serial_driver.h"
#include "return_code.h"
#include "driver.h"
#include "ioctl.h"

RETURN_CODE initDrivers();

RETURN_CODE driverWrite(enum DRIVER_TYPE type, void *data);

RETURN_CODE driverIoctl(enum DRIVER_TYPE type, enum IOCTL_CMD cmd, void *data);

void* driverRead(enum DRIVER_TYPE type, void *data);
#endif /* DRIVER_MANAGER_H_ */
