#include "hdmi_driver.h"

#include <stdlib.h>

#define HDMI_MSG_WRITE 		0

static MESSAGE msg;

void
main( void )
{
	openHDMI();
	setScale( 5 );
	
	writeHDMI( "Hello I'm the HDMI-Driver!\n" );

	if ( channelOpen( HDMI_CHANNEL ) )
	{
		exitTask( 1 );
	}

	while ( 0 == receive( HDMI_CHANNEL, &msg, 0 ) )
	{
		if ( HDMI_MSG_WRITE == msg.id )
		{
			writeHDMI( ( const char* ) msg.data );
		}
	}

	closeHDMI();

	exitTask( 0 );
}
