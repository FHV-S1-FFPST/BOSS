/*
 * video.h
 *
 *  Created on: 16.04.2014
 *      Author: faisstm
 */

#ifndef VIDEO_H_
#define VIDEO_H_

void initHDMI(uint32_t width, uint32_t height);
void drawCharacterWithColor(char c, uint32_t color);
void drawCharacter(char c);
void drawString(char *s);
void drawStringWithColor(char *s, uint32_t color);
void setScale(uint8_t scale);
char* getFrameBuffer();

#endif /* VIDEO_H_ */
