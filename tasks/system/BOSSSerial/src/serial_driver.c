/*
 * serial_driver.c
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */

#include "serial_driver.h"

#include <stdlib.h>

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

#define reg32w(b, r, v) (*((volatile uint32_t *)((b)+(r))) = (v))
#define reg32r(b, r) (*(volatile uint32_t *)((b)+(r)))

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

uint32_t
initSerial( void )
{
	reg32w( UART3_BASE, MDR1_REG, 0x0 );

	return 0;
}

uint32_t
setBaudRate( uint32_t rate )
{
	reg32w(UART3_BASE, MDR1_REG, 0x07);
	reg32w(UART3_BASE, LCR_REG, 0x80|0x03);
	reg32w(UART3_BASE, DLL_REG, (rate & 0xff));
	reg32w(UART3_BASE, DLH_REF, (rate >> 8) & 0xff);
	reg32w(UART3_BASE, LCR_REG, 0x03);
	reg32w(UART3_BASE, MCR_REG, 0x01 | 0x02 );
	reg32w(UART3_BASE, FCR_REG, 0x01 | 0x02 | 0x04);

	reg32w(UART3_BASE, MDR1_REG, 0x0);

	return 0;
}

uint32_t
writeBuffer( const uint8_t* data, uint32_t dataSize )
{
	uint32_t i = 0;

	for ( i = 0; i < dataSize; i++ )
	{
		while ((reg32r(UART3_BASE,LSR_REG) & 0x20) == 0)
		{
			 reg32w(UART3_BASE,0,data[ i ]);
		}
	}

	return 0;
}

int32_t
readBuffer( uint8_t* buffer, uint32_t bufferSize )
{
	return -1;
}
