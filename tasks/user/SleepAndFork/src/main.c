/*
 * task1.c
 *
 *  Created on: 27.05.2014
 *      Author: Jonathan Thaler
 */

#include <boss.h>

#include <string.h>

#define GPIO5_OE           (*((volatile unsigned long *)0x49056034))   // Output Data Enable Register (Table 24-27)
#define GPIO5_DATAOUT      (*((volatile unsigned long *)0x4905603C))   // Data Out register (Table 24-31)
#define LED1 0x00200000   // Bit 21
#define LED0 0x00400000   // Bit 22
//
// PRCM
#define CM_FCLKEN_PER      (*((volatile unsigned long *)0x48005000))   // Controls the modules functional clock activity. (Table 4-237)
#define CM_ICLKEN_PER      (*((volatile unsigned long *)0x48005010))   // Controls the modules interface clock activity (Table 4-239)
//
// SCM
#define CONTROL_PADCONF_UART1_TX  (*((volatile unsigned long *)0x4800217C))   // Pad configuration for GPIO_149 [31:16] (Tables 7-4 & 7-74)

static MESSAGE msg;

int
main( void )
{
	channelOpen( 3 );

	channelSubscribe( 3 );

	while( 1 )
	{
		uint64_t startMillis = getSysMillis();

		receive( NULL_CHANNEL, 0, -1 );

		// wait for a message on NULL-CHANNEL for 1000ms -> will timeout, nullchannel is not existing
		receive( NULL_CHANNEL, 0, 10000 );

		const char* str = "Where is my johnny?";

		msg.id = 42;
		msg.dataSize = 19; //strlen( str );
		memset( msg.data, 0, sizeof( msg.data ) );

		strncpy( ( char* ) msg.data, str, msg.dataSize );

		send( 3, &msg );

		memset( &msg, 0, sizeof( MESSAGE ) );

		receive( 3, &msg, 0 );

		send( 3, &msg );

		uint64_t stopMillis = getSysMillis();
		uint64_t deltaMillis = stopMillis - startMillis;

		deltaMillis++;
	}

/*
	  volatile long int i;

	  // Set the pinmux to select the GPIO signal
	  CONTROL_PADCONF_UART1_TX &= 0x0000FFFF; //  [31:16]=GPIO_149  - Clear register bits [31:16]
	  CONTROL_PADCONF_UART1_TX |= 0x00040000; //  [31:16]=GPIO_149  - select mux mode 4 for gpio

	  // Switch on the Interface and functional clocks to the GPIO5 module
	  //CM_FCLKEN_PER |=  0x20; // Enable GPIO5 F clock
	  //CM_ICLKEN_PER |=  0x20; // Enable GPIO5 I clock

	  // Configure the GPIO signals
	  GPIO5_OE      &= ~LED1;  // Set GPIO_149 & GPIO_150 (GPIO 4 bit 2) to output
	  GPIO5_DATAOUT |=  LED1;  // Set GPIO_150 high

	  // Hello world!
	  while(1) {
	    for (i=0; i<0x100000; i++) {}
	      GPIO5_DATAOUT ^=  LED1;  // Toggle GPIO_149
	  }
	  */
}

/*
int
main( void )
{
	if ( fork() )
	{
		// NOTE: at this point we are in the parent of fork

		while( 1 )
		{
			uint64_t startMillis = getSysMillis();

			sleep( 5000 );

			uint64_t stopMillis = getSysMillis();
			uint64_t deltaMillis = stopMillis - startMillis;

			deltaMillis++;
		}
	}
	else
	{
		// NOTE: at this point we are in the child of fork

		while( 1 )
		{
			uint64_t startMillis = getSysMillis();

			sleep( 10000 );

			uint64_t stopMillis = getSysMillis();
			uint64_t deltaMillis = stopMillis - startMillis;

			deltaMillis++;
		}
	}
}
*/
