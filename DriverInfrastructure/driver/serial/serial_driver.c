/*
 * serial_driver.c
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */


#include "serial_driver.h"
#include "../../common/common.h"
#include "../return_code.h"
#include "../ioctl.h"

#define NULL ((void*)0)

#define UART3_BASE 0x49020000

#define DLL_REG 0x000 // Divisor latches lows
#define DLH_REF 0x004 // Divisor latches high // Interrupt enable register ??
//#define IER_REG 0x004 // Interrupt enable register
#define MDR1_REG 0x020 // Mode definition register
#define FCR_REG 0x008 // FIFO control register
#define SSR_REG 0x044 // status register [0] = 1 FIFO = Full
#define TLR_REG 0x01C // Trigger level register
#define LCR_REG 0x00C // Line control register
#define MCR_REG 0x010 // Modem control register
#define LSR_REG 0x014 // Line status register
#define MSR_REG 0x018 // Modem status register
#define SCR_REG 0x040 // Supplementary control register

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

	reg32w(UART3_BASE, MDR1_REG, 0x07);
	reg32w(UART3_BASE, LCR_REG, 0x80|0x03);
	reg32w(UART3_BASE, DLL_REG, (baud_divisor & 0xff));
	reg32w(UART3_BASE, DLH_REF, (baud_divisor >> 8) & 0xff);
	reg32w(UART3_BASE, LCR_REG, 0x03);
	reg32w(UART3_BASE, MCR_REG, 0x01 | 0x02 );
	reg32w(UART3_BASE, FCR_REG, 0x01 | 0x02 | 0x04);
	reg32w(UART3_BASE, MDR1_REG, 0x0);

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
	while ((reg32r(UART3_BASE,LSR_REG) & 0x20) == 0);
 	 	 reg32w(UART3_BASE,0,(*tmp++));
 	}
	return SUCCESS;
}

static void* readSerial(Driver_t *self) {
	return NULL;
}

Driver_t *createSerialDriver() {
	Driver_t *serialDriver = (Driver_t *)malloc(sizeof(Driver_t *));
	serialDriver->ioctl = &ioctlSerial;
	serialDriver->open = &openSerial;

	serialDriver->read = &readSerial;
	serialDriver->write = &writeSerial;

	return serialDriver;
}



