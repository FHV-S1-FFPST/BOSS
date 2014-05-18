/*
 * ioctl.h
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */

#ifndef IOCTL_H_
#define IOCTL_H_

/**
 * Stores all the different IOCTL-commands of all the drivers
 * They are stored in the same enum, so if the right IOCTL-command is sent to the wrong
 * driver, it wont mess things up
 */
enum IOCTL_CMD {
	//Serial commands
	SERIAL_SET_BAUD_RATE = 1,

	// HDMI commands
	SET_SCALE = 2,
	DRAW_HORIZONTAL_LINE = 3,
	DRAW_VERTICAL_LINE = 4,
	DRAW_STRING_WITH_COLOR = 5,
	ATTACH_FRAMEBUFFER = 6
};


#endif /* IOCTL_H_ */
