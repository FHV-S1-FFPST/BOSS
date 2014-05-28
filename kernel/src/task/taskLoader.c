/*
 * taskLoader.c
 *
 *  Created on: 24.05.2014
 *      Author: Jonathan Thaler
 */

#include "taskLoader.h"

#include "../fs/fat32/fat32.h"
#include "../scheduler/scheduler.h"
#include "../page_manager/pageManager.h"

#include <stdlib.h>
#include <string.h>

// module-local defines
#define EI_CLASS_32_BIT			0x1
#define E_TYPE_EXECUTABLE		0x2
#define E_MACHINE_ARM			0x28

#define PT_LOAD 				1

#define PF_X 					0x1
#define PF_W 					0x2
#define PF_R 					0x4

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

typedef struct
{
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} PROGRAM_HEADER;
///////////////////////////////////////////////////////////////

// module-local data
static uint8_t eIdentMagicNumber[ 4 ] = { 0x7F, 'E', 'L', 'F' };

///////////////////////////////////////////////////////////////
uint32_t
loadTaskFromFile( const char* fileName )
{
	uint32_t i = 0;
	uint32_t fileSize = 0;
	uint8_t* fileBuffer = 0;
	file_id taskImageFile = 0;
	ELF_HEADER* elfHeader = 0;
	Task* task = 0;

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

	task = createTask( ( uint32_t* ) elfHeader->e_entry, fileSize );
	if ( 0 == task )
	{
		ret = 1;
		goto closeAndExit;
	}

	for ( i = 0; i < elfHeader->e_phnum; ++i )
	{
		PROGRAM_HEADER* programHeader = ( PROGRAM_HEADER* ) &fileBuffer[ elfHeader->e_phoff + ( i * elfHeader->e_phentsize ) ];

		if ( PT_LOAD != programHeader->p_type )
		{
			continue;
		}

		memcpy( ( uint32_t* ) programHeader->p_vaddr, &fileBuffer[ programHeader->p_offset ], programHeader->p_memsz );
	}

	ret = 0;

closeAndExit:
	fat32Close( taskImageFile );

	return ret;
}
