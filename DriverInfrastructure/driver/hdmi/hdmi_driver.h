/*
 * hdmi_driver.h
 *
 *  Created on: 14.05.2014
 *      Author: faisstm
 */

#ifndef HDMI_DRIVER_H_
#define HDMI_DRIVER_H_

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "../driver.h"

typedef struct OffsetLengthData {
	uint32_t offset;
	uint32_t length;
} OffsetLengthData_t;

typedef struct DrawStringColorData {
	char *message;
	uint32_t color;
} DrawStringColorData_t;


#define DSS_BASE 0x48050000
#define DSPC_BASE 0x48050400

#define DSS_CONTROL 0x040

#define DSPC_IRQENABLE 0x01C
#define DSPC_CONTROL 0x040
#define DSPC_DEFAULT_COLOR_0 0x04C
#define DSPC_DEFAULT_COLOR_1 0x050

#define DSPC_SIZE_LCD 0x07C
#define DSPC_SIZE_DIG 0x078

#define DSPC_TIMING_H 0x064
#define DSPC_TIMING_V 0x068

#define DSPC_GFX_BA1 0x080
#define DSPC_GFX_ATTRIBUTES 0x0A0
#define DSPC_GFX_SIZE 0x08C
#define DSPC_GFX_POS 0x088

Driver_t *createHDMIDriver();

#endif /* HDMI_DRIVER_H_ */
