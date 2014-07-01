#include <boss.h>

#include <stdlib.h>
#include <string.h>

static MESSAGE msg;

static uint32_t printText( const char* text );

void
main( void )
{
	receive( NULL_CHANNEL, 0, 15000 );

	/*
	if ( channelSubscribe( HDMI_CHANNEL ) )
	{
		exitTask( 1 );
	}
*/

	srand ( getSysMillis() );

	printText( "Hello I'm Producer 2!\n" );

	while ( 1 )
	{
		if ( printText( "Producer 2 is sad!\n" ) )
		{
			break;
		}

		int randTo = rand() % 4000;

		receive( NULL_CHANNEL, 0, randTo + 1000 );
	}

	exitTask( 0 );
}

uint32_t
printText( const char* text )
{
	msg.id = 0;
	msg.dataSize = strlen( text );
	memcpy( msg.data, text, msg.dataSize );
	msg.data[ msg.dataSize ] = '\0';

	if ( send( HDMI_CHANNEL, &msg ) )
	{
		return 1;
	}

	return 0;
}
