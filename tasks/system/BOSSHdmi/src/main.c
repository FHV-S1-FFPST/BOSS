#include "hdmi_driver.h"

#include <stdlib.h>

#define HDMI_MSG_WRITE 		0

static MESSAGE msg;

int
main( void )
{
	openHDMI();
	
	/*if ( channelOpen( HDMI_CHANNEL ) )
	{
		return 1;
	}

	while ( 0 == receive( HDMI_CHANNEL, &msg, 0 ) )
	{
		if ( HDMI_MSG_WRITE == msg.id )
		{
			writeHDMI( ( const char* ) msg.data );
		}
	} */

	volatile long int i = 0;

	while(1) {

		writeHDMI("asdf");

		for(i = 0; i < 0x100000; i++) {

		}
	}





	return 0;
}
