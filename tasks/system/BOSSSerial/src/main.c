#include "serial_driver.h"

#include <stdlib.h>

#define SERIAL_MSG_WRITE 	0
#define SERIAL_MSG_READ 	1
#define SERIAL_MSG_BAUD 	2

static MESSAGE msg;

int
main( void )
{
	initSerial();

	if ( channelOpen( SERIAL_CHANNEL ) )
	{
		return 1;
	}

	while ( 0 == receive( SERIAL_CHANNEL, &msg, 0 ) )
	{
		if ( SERIAL_MSG_WRITE == msg.id )
		{
			writeBuffer( msg.data, msg.dataSize );
		}
		else if ( SERIAL_MSG_READ == msg.id )
		{
			msg.dataSize = 1;
			msg.data[ 0 ] = 'X';

			send( SERIAL_CHANNEL, &msg );
		}
		else if ( SERIAL_MSG_BAUD == msg.id )
		{
			uint32_t baudRate = *msg.data;

			setBaudRate( baudRate );
		}
	}

	return 0;
}
