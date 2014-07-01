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

	/*
	srand ( getSysMillis() );
	while ( 1 )
	{
		writeHDMI( "Hello ITM13!\n" );

		int randTo = rand() % 4000;

		receive( NULL_CHANNEL, 0, randTo + 1000 );
	}
*/

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
