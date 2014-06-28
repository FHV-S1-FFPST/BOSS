/*
 * video.c
 *
 *  Created on: 16.04.2014
 *      Author: faisstm
 */


#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "video.h"

#define BPP 4
#define DSS_BASE 0x48050000
#define DSPC_BASE 0x48050400

#define DSS_CONTROL 0x040

#define DSPC_IRQENABLE 0x01C
#define DSPC_CONTROL 0x040
#define DSPC_DEFAULT_COLOR_0 0x04C
#define DSPC_DEFAULT_COLOR_1 0x050

#define DSPC_SIZE_LCD 0x07C
#define DSPC_SIZE_DIG 0x078

#define DSPC_TIMING_H 0x064
#define DSPC_TIMING_V 0x068

#define DSPC_GFX_BA1 0x080
#define DSPC_GFX_ATTRIBUTES 0x0A0
#define DSPC_GFX_SIZE 0x08C
#define DSPC_GFX_POS 0x088

#define reg32r(b, r) (*(volatile uint32_t *)((b)+(r)))
#define reg32w(b, r, v) (*((volatile uint32_t *)((b)+(r))) = (v))

uint32_t window_width;
uint32_t screen_height;

uint32_t currentRowStart;
uint32_t currentRow;
uint32_t currentLine;
uint32_t currentLineStart;
uint8_t scalingFactor;

char* framebuff;
long buffSize;

extern uint8_t outputNES[240][256];

void setScale(uint8_t scale) {
	scalingFactor = scale;
}


void drawCharacterWithColor(char c, uint32_t color) {

	if(c == '\n') {
		currentLine += 8 * scalingFactor;
		currentLineStart = currentLine;
		currentRow = 0;
		currentRowStart = 0;
		return;
	}

	uint16_t lineBitmap;
	uint32_t framebuffPos = 0;

	uint32_t i;
	uint32_t j;
	uint32_t k;

	if(currentRow + 8*scalingFactor > window_width * BPP) {
		drawCharacter('\n');
	}


	if(currentLine + 8 * scalingFactor > screen_height) {
		currentLine -= 8 * scalingFactor;
		currentLineStart = currentLine;
		memcpy(framebuff, &framebuff[window_width * BPP * scalingFactor * 8], buffSize - window_width * BPP * scalingFactor * 8);
		//memset(&framebuff[(960-(8 * scalingFactor))*window_width*3], '0', window_width * 8 * 3 * scalingFactor);
	}

	for(i = 0; i < 8; i++) {
		lineBitmap = font[c*8+i];

		for(k = 0; k < 8; k++) {
			uint32_t temp = (lineBitmap >> (7-k)) & 0x01;

			if(temp == 1) {
				framebuffPos = currentLine * window_width * BPP + currentRow;
				for(j = 0; j < BPP * scalingFactor; j += BPP) {
					framebuff[framebuffPos + j] = (char) (color & 0xFF);
					framebuff[framebuffPos + j + 1] = (char) ((color >> 8) & 0xFF);
					framebuff[framebuffPos + j + 2] = (char) ((color >> 16) & 0xFF);
				}
			}

			currentRow += BPP * scalingFactor;
		}

		uint32_t tempLine = currentLine;
		for(j = 1; j < scalingFactor; j++) {
			++currentLine;
			memcpy(&framebuff[currentLine * window_width * BPP], &framebuff[tempLine * window_width * BPP], window_width * BPP);
		}

		++currentLine;
		currentRow = currentRowStart;

	}

	currentLine = currentLineStart;
	currentRowStart += BPP * scalingFactor * 8;
	currentRow = currentRowStart;

}

void drawCharacter(char c) {
	drawCharacterWithColor(c, 0xFFFFFF);
}

void drawHorizontalLine(uint32_t offset, uint32_t length) {

	uint32_t i;
	uint32_t lengthRGB = length * BPP;
	uint32_t startAdress = offset * window_width * BPP;
	for(i = 0; i < lengthRGB; i += BPP) {
		framebuff[startAdress + i] = (char) 255;
		framebuff[startAdress + i + 1] = (char) 255;
		framebuff[startAdress + i + 2] = (char) 255;
	}
}

void drawVerticalLine(uint32_t offset, uint32_t length) {
	uint32_t i;
	uint32_t j;

	for(j = 0; j < 15; j += 3) {
		for(i = 0; i < length; i++) {
			uint32_t startAdress = i*window_width * BPP + offset * BPP;
			framebuff[startAdress + j] = (char) 255;
			framebuff[startAdress + 1 + j] = (char) 255;
			framebuff[startAdress + 2 + j] = (char) 255;
		}

	}
}

void drawStringWithColor(char *s, uint32_t color) {
	uint32_t i = 0;
	while(s[i] != '\0') {
		drawCharacterWithColor(s[i], color);
		++i;
	}
}

void drawString(char *s) {
	drawStringWithColor(s, 0xFFFFFF);
}

char* getFrameBuffer() {
	return framebuff;
}

void initHDMI(uint32_t width, uint32_t height) {
		screen_height = height;
		window_width = width;
		scalingFactor = 1;

		currentRow = 0;
		currentLine = 0;
		currentRowStart = 0;

		buffSize = screen_height * window_width * BPP;

		reg32w(DSPC_BASE, DSPC_DEFAULT_COLOR_0, 0x000000);
		reg32w(DSPC_BASE, DSPC_SIZE_LCD, (screen_height - 1) << 16 | (window_width - 1));
		reg32w(DSPC_BASE, DSPC_TIMING_H, 0x0cf03f31);
		reg32w(DSPC_BASE, DSPC_TIMING_V, 0x01400504);

		framebuff = malloc(sizeof(char) * buffSize);
		/*int i;
		for(i = 0; i < buffSize; i++) {
			framebuff[i] = (char) (rand() % 255);
		} */

		memset(framebuff, (char)0, buffSize);

		reg32w(DSPC_BASE, DSPC_GFX_BA1, (uint32_t) framebuff);
		reg32w(DSPC_BASE, DSPC_GFX_SIZE, ((screen_height -1) << 16) | ( window_width -1));
		reg32w(DSPC_BASE, DSPC_GFX_POS, (0 << 16) | 0);
		reg32w(DSPC_BASE, DSPC_GFX_ATTRIBUTES, (0x0 << 8) | (0x2 << 6) | (0x8 << 1) | 0x1);
		reg32w(DSPC_BASE, DSPC_CONTROL, reg32r(DSPC_BASE, DSPC_CONTROL) | 0x01 << 5);
}

