/*
 * Write a serial number to the first EEPROM address and print it out to the serial
 */

#include <EEPROM.h>

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)

const byte serial_no = SERIAL_NUMBER_GOES_HERE;

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  EEPROM.write(0, serial_no);
}

void loop()
{
  Serial.print("Serial: ");
  Serial.println(EEPROM.read(0));
  while (1);
}
