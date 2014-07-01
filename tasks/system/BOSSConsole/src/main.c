#include <boss.h>

#include <stdlib.h>
#include <string.h>

static MESSAGE msg;

static uint32_t printText( const char* text );
static uint32_t getCh( char* c );

void
main( void )
{
	// NOTE: need to wait 20 seconds to ensure HDMI-channel was opened before by HDMI-driver
	receive( NULL_CHANNEL, 0, 20000 );

	if ( channelSubscribe( HDMI_CHANNEL ) )
	{
		exitTask( 1 );
	}

	srand ( getSysMillis() );

	printText( "Hello I'm the Console!\n" );

	while ( 1 )
	{
		if ( printText( "Hello ITM13!\n" ) )
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

uint32_t
getCh( char* c )
{
	msg.id = 1; // READ from RS232

	if ( sendrcv( SERIAL_CHANNEL, &msg, 0 ) )
	{
		return 1;
	}

	if ( 1 != msg.dataSize )
	{
		return 1;
	}

	*c = msg.data[ 0 ];

	return 0;
}
