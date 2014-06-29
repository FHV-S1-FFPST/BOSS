/* MoarNES source - rom.c

   This open-source software is (c)2010-2012 Mike Chambers, and is released
   under the terms of the GNU GPL v2 license.

   This file contains the code to load NES ROM files.
*/

#include <boss.h>

#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "rom.h"
#include "PPU.h"

struct cart_s cartridge;
extern struct PPU_s *PPU;

//static const uint8_t validhdr[4] = { 'N', 'E', 'S', 0x1A };

extern bool mapperinit(struct cart_s *cart);


bool loadROM( const char* filename) {


	file_id fileID = 0;
	if(bossfopen(filename, &fileID)) {
		return 1;
	}

	uint32_t fileSize = 0;
	bossfsize(fileID, &fileSize);

	uint8_t* buffer = 0;
	buffer = malloc(fileSize);

	bossfread(fileID, fileSize, buffer );


	static struct hdr_s header;

	memcpy(&header, buffer, sizeof(header));

	//fread(&header, 1, sizeof (header), rom);
	if (header.match[0] != 'N' || header.match[1] != 'E' || header.match[2] != 'S' || header.match[3] != 0x1A) {
		return 0;
	}

	if (header.flags[0] & 0x8) {
		cartridge.mirroring = MIR_FOURSCREEN;
	}
	else if (header.flags[0] & 0x1) {
		cartridge.mirroring = MIR_VERTICAL;
	}
	else {
		cartridge.mirroring = MIR_HORIZONTAL;
	}

	cartridge.PRGcount = (uint16_t)header.PRGsize << 4;
	cartridge.CHRcount = (uint16_t)header.CHRsize << 3;
	cartridge.hasSRAM = 1;

	if (header.flags[0] & 0x4) {
		cartridge.hasTrainer = 1;
	}
	else {
		cartridge.hasTrainer = 0;
	}

	switch (cartridge.mirroring) {
	case 0:
		PPU->ntmap[0] = 0x2000;
		PPU->ntmap[1] = 0x2000;
		PPU->ntmap[2] = 0x2400;
		PPU->ntmap[3] = 0x2400;
		break;
	case 1:
		PPU->ntmap[0] = 0x2000;
		PPU->ntmap[1] = 0x2400;
		PPU->ntmap[2] = 0x2000;
		PPU->ntmap[3] = 0x2400;
		break;
	case 2:
		PPU->ntmap[0] = 0x2000;
		PPU->ntmap[1] = 0x2400;
		PPU->ntmap[2] = 0x2600;
		PPU->ntmap[3] = 0x2800;
		break;
	default:
		break;
	}

	cartridge.mapper = (header.flags[0] >> 4) & 0x0F;
	cartridge.mapper |= (header.flags[1] & 0xF0);

	cartridge.PRGfull = malloc(cartridge.PRGcount * 1024);
	if (cartridge.PRGfull == NULL) {
		return 0;
	}

	if (cartridge.CHRcount > 0) {
		cartridge.CHRfull = malloc(cartridge.CHRcount * 1024);

		if (cartridge.CHRfull == NULL) {
			return 0;
		}
	}
	else {
		cartridge.CHRfull = malloc(1024 << 10);
	}

	if (cartridge.hasTrainer) {
		//fseek(rom, 528, SEEK_SET);
		buffer += 528;
	}
	else {
		buffer += 16;
		//fseek(rom, 16, SEEK_SET);
	}


	memcpy(cartridge.PRGfull, buffer, (uint32_t) cartridge.PRGcount << 10);
	buffer += (uint32_t) cartridge.PRGcount << 10;


	if (cartridge.CHRcount > 0) {
		memcpy(cartridge.CHRfull, buffer, (uint32_t) cartridge.CHRcount << 10);
	}
	//fclose(rom);
	free(buffer);

	uint16_t setptr;
	for (setptr = 0; setptr < 1024; setptr++) {
		cartridge.PRGbank[setptr] = cartridge.PRGfull;
		cartridge.CHRbank[setptr] = cartridge.CHRfull;
	}

	if (mapperinit(&cartridge) == 0) {
		return 0;
	}

	return 1;
}
