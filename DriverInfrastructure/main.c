#include "driver/driver_manager.h"
#include "driver/return_code.h"
#include "driver/driver.h"

int main (void) {

  RETURN_CODE initReturn = initDrivers();
  if(initReturn == FAILURE) {
	  return 1;
  }

  // Serial Test

/*
  //Set the baud-rate
  int baud_rate = 312;
  driverIoctl(SERIAL_DRIVER, SERIAL_SET_BAUD_RATE, &baud_rate);

  //Write a message out of the serial port
  char *message = "\r\nTACHO!\r\n\0";
  driverWrite(SERIAL_DRIVER, message);
*/
  // HDMI Test

  uint8_t scale = 5;
  driverIoctl(HDMI_DRIVER, SET_SCALE, &scale );
  driverWrite(HDMI_DRIVER, "Hallo!");

  while(1) {

  }

}

