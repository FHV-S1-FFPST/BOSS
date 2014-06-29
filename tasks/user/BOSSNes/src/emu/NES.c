/* MoarNES source - NES.c

   This open-source software is (c)2010-2012 Mike Chambers, and is released
   under the terms of the GNU GPL v2 license.

   This file contains the code to simulate the internal NES architecture,
   such as memory mapping, redirecting read/writes to I/O registers, etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "../common/common.h"
//#include "../hdmi/video.h"
#include "NES.h"
#include "stdint.h"
#include "rom.h"
#include "PPU.h"

/*
   Nintendo Entertainment System: CPU memory map

   $0000-$07FF = 2 KB internal RAM
   $0800-$1FFF = Mirrors of $0000-$07FF
   $2000-$2007 = PPU registers
   $2008-$3FFF = Mirrors of $2000-$2007
   $4000-$4017 = APU and I/O registers
   $4018-$FFFF = Cartridge PRG ROM, cartridge PRG RAM, and mapper registers

   $FFFA = NMI vector
   $FFFC = Reset vector
   $FFFE = IRQ/BRK vector

   In some cartridges, $6000-$7FFF is SRAM.

*/

uint32_t totalframes = 0;
uint8_t NESRAM[0x800]; //2 KB internal RAM
uint32_t cpugoal = 0, cpuadjust;
uint16_t sprtablesave;
char persistenttext[64];
uint16_t texttimer = 0;
struct cheat_s cheat[3];

extern struct cart_s cartridge; //struct is declared in rom.c
extern struct PPU_s *PPU;
extern struct OAM_s *OAM;

extern struct map4_s *map4;

extern uint64_t exec6502(uint64_t tickcount);
extern void step6502();
extern void reset6502();
extern void nmi6502();
extern uint16_t getPC();
extern uint64_t getclockticks();

extern void initPPU();
extern void killPPU();
extern uint8_t readPPU(uint16_t addr);
extern void writePPU(uint16_t addr, uint8_t value);
extern uint8_t readPPUregs(uint16_t addr);
extern void writePPUregs(uint16_t addr, uint8_t value);
extern void renderscanline(uint16_t scanline);
extern char* getTempBuffer();

extern uint8_t mapperwrite(uint16_t addr, uint8_t value);

extern bool loadROM(const char *filename);

extern uint8_t outputNES[240][256];


uint64_t timerfreq, starttimer, endtimer, difftimer, curtimer, nextframe, latestframe, frametime;
double doubletimer;

extern void savestate();
extern void loadstate();

bool running = FALSE;
bool paused = FALSE;
bool doloadstate = FALSE;
bool dosavestate = FALSE;

uint8_t read6502(uint16_t addr) {
	uint8_t retval;

	if (addr < 0x2000) retval = NESRAM[addr & 0x7FF];
	else if (addr < 0x4000) retval = readPPUregs(0x2000 | (addr & 7));
	else if (addr < 0x4018) {
		retval = 0;
	}
	else if (addr < 0x8000) {
		if (cartridge.hasSRAM == 1) {
			retval = cartridge.SRAM[addr & 0x1FFF];
		}
		else retval = 0; //open data bus if no cartidge SRAM
	}
	else {
		//if we get to this point, it's a read from the cartridge
		addr &= 0x7FFF;
		retval = cartridge.PRGbank[addr >> 10][addr & 1023];
	}

	return (retval);
}

void write6502(uint16_t addr, uint8_t value) {
	uint16_t tempsprite;

	if (addr < 0x2000) {
		NESRAM[addr & 0x7FF] = value;
		return;
	}
	if (addr < 0x4000) {
		writePPUregs(0x2000 | (addr & 7), value);
		return;
	}
	if (addr < 0x4018) {
		if (addr == 0x4014) {
			for (tempsprite = 0; tempsprite < 256; tempsprite++) {
				OAM->RAM[(OAM->addr + tempsprite) & 255] = read6502(value * 256 + tempsprite);
			}
			return;
		}
	}

	if (addr < 0x8000) {
		if (cartridge.hasSRAM == 1) {
			cartridge.SRAM[addr & 0x1FFF] = value;
		}
		else return; //open data bus if no cartidge SRAM
	}

	//if we get to this point, it's an attempted write to the cartridge.
	//obviously, being ROM, nothing would actually get written anywhere.
}

extern void irq6502();
extern void map4irqdecrement();
void execframe() {
	uint16_t scanline;

	PPU->vblank = 0;
	PPU->sprzero = 0;
	PPU->sprover = 0;
	sprtablesave = PPU->sprtable;

	if (PPU->bgvisible) {
		exec6502(101);
		PPU->addr = PPU->tempaddr;
		exec6502(13);
	}
	else exec6502(114);
	for (scanline = 1; scanline<241; scanline++) {
		if ((scanline % 3) == 0) exec6502(86);
		else exec6502(85);
		renderscanline(scanline - 1);

		if (PPU->bgvisible) PPU->addr = (PPU->addr & 0xFBE0) | (PPU->tempaddr & 0x041F);

		exec6502(28);
	}

	PPU->vblank = 1;
	if (PPU->nmivblank) nmi6502();

	for (scanline = 241; scanline<260; scanline++) { //20 scanlines during vblank
		if ((scanline % 3) == 0) exec6502(114);
		else exec6502(113);
	}

	totalframes++;
}

void execNES() {
	reset6502();
	memset(&NESRAM[0], 0, 2048);

	volatile uint64_t i = 0;
	while (1) {
		execframe();
	}

}

void startNES( const char* file ) {
	initPPU();
	memset(&outputNES[0][0], 65, 61440);

	loadROM( file );

	while(1) {
		execNES();
	}
}

void timedtext(const char *text) {
	sprintf(&persistenttext[0], text);
	texttimer = 240; //unit of measure is frames (at 60 FPS)
}
