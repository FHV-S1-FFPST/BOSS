/*
 * driver_manager.c
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */

#include "driver_manager.h"
#define NULL ((void*)0)

static Driver_t loadedDrivers[2];

//Initialize all known drivers
RETURN_CODE initDrivers() {
	Driver_t *serialDriver = createSerialDriver();

	if(serialDriver->open(serialDriver) == SUCCESS) {
		loadedDrivers[SERIAL_DRIVER] = *serialDriver;
	}

	Driver_t *hdmiDriver = createHDMIDriver();

	if(hdmiDriver->open(hdmiDriver) == SUCCESS) {
		loadedDrivers[HDMI_DRIVER] = *hdmiDriver;
	}

	return SUCCESS;
}

RETURN_CODE driverWrite(enum DRIVER_TYPE type, void *data) {
	Driver_t *driver = &loadedDrivers[type];

	if(driver != NULL) {
		return driver->write(driver, data);
	}

	return FAILURE;
}

RETURN_CODE driverIoctl(enum DRIVER_TYPE type, enum IOCTL_CMD cmd, void *data) {
	Driver_t *driver = &loadedDrivers[type];

	if(driver != NULL) {
		return driver->ioctl(driver, cmd, data);
	}

	return FAILURE;
}

void* driverRead(enum DRIVER_TYPE type, void *data) {
	Driver_t *driver = &loadedDrivers[type];

	if(driver != NULL) {
		return driver->read(driver);
	}

	return NULL;
}
