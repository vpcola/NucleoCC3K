/*
 * HTU21D.h
 *
 *  Created on: Oct 5, 2014
 *      Author: Vergil
 */

#ifndef HTU21D_H_
#define HTU21D_H_

extern "C"
{
  #include "ch.h"
  #include "hal.h"
  #include "tm.h"
}

// Address defines
#define HTU21D_I2C_ADDRESS  0x40
#define TRIGGER_TEMP_MEASURE  0xE3
#define TRIGGER_HUMD_MEASURE  0xE5


//Commands.
#define HTU21D_EEPROM_WRITE 0x80
#define HTU21D_EEPROM_READ  0x81

class HTU21D {
public:
  enum TMPTYPE {
    CELCIUS,
    FAHRENHEIT,
    KELVIN
  };

  HTU21D(I2CDriver * driver);

  float getTemp(int tmpType=CELCIUS);
  float getHumidity();

private:
  I2CDriver * _driver;
};


#endif /* HTU21D_H_ */
