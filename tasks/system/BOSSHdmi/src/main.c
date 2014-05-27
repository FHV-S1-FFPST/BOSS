#include "hdmi_driver.h"

#define HDMI_MSG_WRITE 0

int
main( void )
{
	openHDMI();
	
	MESSAGE msg;

	while ( 0 == receive( HDMI_CHANNEL, &msg ) )
	{
		if ( HDMI_MSG_WRITE == msg.id )
		{
			writeHDMI( ( const char* ) msg.data );
		}
	}

	return 0;
}
