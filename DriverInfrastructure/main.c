#include "driver/driver_manager.h"
#include "driver/return_code.h"
#include "driver/driver.h"

int main (void) {

  RETURN_CODE initReturn = initDrivers();
  if(initReturn == FAILURE) {
	  return 1;
  }

  //Write a message out of the serial port
  char *message = "\r\nTACHO!\r\n\0";
  driverWrite(SERIAL_DRIVER, message);
}

