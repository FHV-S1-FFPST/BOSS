/*
 * taskLoader.c
 *
 *  Created on: 24.05.2014
 *      Author: Jonathan Thaler
 */

#include "taskLoader.h"

#include "../fs/fat32/fat32.h"

#include <stdlib.h>
#include <string.h>

// module-local defines
#define EI_CLASS_32_BIT			0x1
#define E_TYPE_EXECUTABLE		0x2
#define E_MACHINE_ARM			0x28
///////////////////////////////////////////////////////////////

// module-local structures
typedef struct
{
	uint8_t	e_identMagic[ 4 ];
	uint8_t	e_identClass;
	uint8_t e_identData;
	uint8_t e_identVersion;

	uint8_t e_identOsabi;
	uint8_t e_identAbiVer;

	uint8_t e_identPad[ 7 ];

	uint16_t e_type;
	uint16_t e_machine;

	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} ELF_HEADER;
///////////////////////////////////////////////////////////////

// module-local data
static uint8_t eIdentMagicNumber[ 4 ] = { 0x7F, 'E', 'L', 'F' };

///////////////////////////////////////////////////////////////
uint32_t
loadTaskFromFile( const char* fileName )
{
	uint32_t fileSize = 0;
	uint8_t* fileBuffer = 0;
	file_id taskImageFile = 0;
	ELF_HEADER* elfHeader = 0;

	if ( fat32Open( fileName, &taskImageFile ) )
	{
		return 1;
	}

	fat32Size( taskImageFile, &fileSize );

	fileBuffer = ( uint8_t* ) malloc( fileSize );
	if ( 0 == fileBuffer )
	{
		return 1;
	}

	// read the elf-header to check for validity
	int32_t ret = fat32Read( taskImageFile, sizeof( ELF_HEADER ), fileBuffer );
	if ( sizeof( ELF_HEADER ) != ret )
	{
		ret = 1;
		goto closeAndExit;
	}

	ret = 1;

	elfHeader = ( ELF_HEADER* ) fileBuffer;

	// ignore invalid ELF-file
	if ( 0 != memcmp( elfHeader->e_identMagic, eIdentMagicNumber, 4 ) )
	{
		goto closeAndExit;
	}

	// accept only 32-bit images
	if ( EI_CLASS_32_BIT != elfHeader->e_identClass )
	{
		goto closeAndExit;
	}

	// accept only excecutable images
	if ( E_TYPE_EXECUTABLE != elfHeader->e_type )
	{
		goto closeAndExit;
	}

	// accept only ARM ISA machines
	if ( E_MACHINE_ARM != elfHeader->e_machine )
	{
		goto closeAndExit;
	}

	// subtract from bytes left to read the header-size
	fileSize -= sizeof( ELF_HEADER );

	// read rest of the file into buffer
	ret = fat32Read( taskImageFile, fileSize, &fileBuffer[ sizeof( ELF_HEADER ) ] );
	if ( ret != fileSize )
	{
		ret = 1;
		goto closeAndExit;
	}

	ret = 0;

closeAndExit:
	fat32Close( taskImageFile );

	return ret;
}
