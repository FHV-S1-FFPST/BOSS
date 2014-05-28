#include <boss.h>

#include <stdlib.h>
#include <string.h>

static MESSAGE msg;

static uint32_t printText( const char* text );
static uint32_t getCh( char* c );

int
main( void )
{
	if ( channelAttach( SERIAL_CHANNEL ) )
	{
		return 1;
	}

	if ( channelAttach( HDMI_CHANNEL ) )
	{
		return 1;
	}

	msg.data = malloc( MESSAGE_MAX_DATA_SIZE );

	printText( "Hello Johnny!\n" );
	printText( "You are again the latest!\n" );

	char c[] = { '\0', '\0' };

	while ( 1 )
	{
		if ( getCh( c ) )
		{
			break;
		}

		if ( printText( c ) )
		{
			break;
		}
	}

	return 1;
}

uint32_t
printText( const char* text )
{
	msg.id = 0; // WRITE to hdmi
	msg.dataSize = strlen( text );
	memcpy( msg.data, text, msg.dataSize );

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
