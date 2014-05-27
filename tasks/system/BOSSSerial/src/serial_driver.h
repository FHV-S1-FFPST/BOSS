/*
 * serial_driver.h
 *
 *  Created on: 21.03.2014
 *      Author: sinz
 */

#ifndef SERIAL_DRIVER_H_
#define SERIAL_DRIVER_H_

#include <boss.h>

uint32_t initSerial( void );
uint32_t setBaudRate( uint32_t rate );
uint32_t writeBuffer( const uint8_t* data, uint32_t dataSize );
int32_t readBuffer( uint8_t* buffer, uint32_t bufferSize );

#endif /* SERIAL_DRIVER_H_ */
