#include "serial_driver.h"

#define SERIAL_MSG_WRITE 0
#define SERIAL_MSG_READ 1
#define SERIAL_MSG_BAUD 2

int
main( void )
{
	initSerial();

	MESSAGE msg;

	while ( 0 == receive( SERIAL_CHANNEL, &msg ) )
	{
		if ( SERIAL_MSG_WRITE == msg.id )
		{
			writeBuffer( msg.data, msg.dataSize );
		}
		else if ( SERIAL_MSG_READ == msg.id )
		{
			// TODO: howto response?
		}
		else if ( SERIAL_MSG_BAUD == msg.id )
		{
			uint32_t baudRate = *msg.data;

			setBaudRate( baudRate );
		}
	}

	return 0;
}
