#include "hdmi_driver.h"

#include <stdlib.h>

#define HDMI_MSG_WRITE 		0

static MESSAGE msg;

int
main( void )
{
	openHDMI();
	
	if ( channelOpen( HDMI_CHANNEL ) )
	{
		return 1;
	}

	msg.data = malloc( MESSAGE_MAX_DATA_SIZE );

	while ( 0 == receive( HDMI_CHANNEL, &msg, 0 ) )
	{
		if ( HDMI_MSG_WRITE == msg.id )
		{
			writeHDMI( ( const char* ) msg.data );
		}
	}

	return 0;
}
