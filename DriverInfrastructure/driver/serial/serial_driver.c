/*
 * serial_driver.c
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */


#include "serial_driver.h"
#include "../return_code.h"
#include "../ioctl.h"

#include "stdlib.h"

#define UART3_BASE 0x49020000

#pragma pack()

struct UART_CONF {
	unsigned char rbr; /* 0 */
	int pad1:24;
	unsigned char ier; /* 1 */
	int pad2:24;
	unsigned char fcr; /* 2 */
	int pad3:24;
	unsigned char lcr; /* 3 */
	int pad4:24;
	unsigned char mcr; /* 4 */
	int pad5:24;
	unsigned char lsr; /* 5 */
	int pad6:24;
	unsigned char msr; /* 6 */
	int pad7:24;
	unsigned char scr; /* 7 */
	int pad8:24;
	unsigned char mdr1; /* mode select reset TL16C750*/
};

typedef volatile struct UART_CONF *UART_CONF_t;
volatile struct UART_CONF *com_port= (UART_CONF_t) UART3_BASE;

static RETURN_CODE openSerial(Driver_t *self) {
	int baud_divisor=312;

	com_port->mdr1 = 0x7;  // MODE_SELECT=DISABLE
	com_port->lcr = 0x80 | 0x03;
	com_port->rbr = baud_divisor & 0xff;
	com_port->rbr = (baud_divisor >> 8) & 0xff;
    	com_port->lcr = 0x03;
	com_port->mcr = 0x01 | 0x02;
	com_port->fcr = 0x01 | 0x02 | 0x04;
	com_port->mdr1 = 0;
	return SUCCESS;
}

static RETURN_CODE ioctlSerial(Driver_t *self, enum IOCTL_CMD cmd, void *data) {
	switch(cmd) {
	case SERIAL_SET_BAUD_RATE:
		return SUCCESS;
	default:
		return FAILURE;
	}
}

static RETURN_CODE writeSerial(Driver_t *self, void *message) {
	char *tmp = message;

	while (*tmp){
        while ((com_port->lsr & 0x20) == 0);
             com_port->rbr= (*tmp++);
 	}
	return SUCCESS;
}

static void* readSerial(Driver_t *self) {
	return NULL;
}

Driver_t *createSerialDriver() {
	Driver_t *serialDriver = (Driver_t*)malloc(sizeof(Driver_t));
	serialDriver->ioctl = &ioctlSerial;
	serialDriver->open = &openSerial;

	serialDriver->read = &readSerial;
	serialDriver->write = &writeSerial;

	return serialDriver;
}



