/*
 * DS1307.h
 *
 *  Created on: Oct 5, 2014
 *      Author: Vergil
 */

#ifndef DS1307_H_
#define DS1307_H_

extern "C"
{
  #include "ch.h"
  #include "hal.h"
  #include "tm.h"
}

#define DS1307_ADDR        0x68 //0xD0    // I2C address
#define DS1307_FREQ        100000  // bus speed
#define DS1307_STRBUFSIZ   50

typedef struct RTC_TIME{
    int sec;        /*!< seconds [0..59] */
    int min;        /*!< minutes {0..59] */
    int hour;       /*!< hours [0..23] */
    int wday;       /*!< weekday [1..7, where 1 = sunday, 2 = monday, ... */
    int date;       /*!< day of month [0..31] */
    int mon;        /*!< month of year [1..12] */
    int year;       /*!< year [2000..2255] */

    const char * to_str();
    static char tmpbuff[DS1307_STRBUFSIZ];
} RTC_TIME;

class DS1307 {
public:
  DS1307(I2CDriver * driver);
  bool get_time(RTC_TIME & time);
  bool set_time(const RTC_TIME time);

private:
  static int bcdToDecimal(int bcd) {
      return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
  }

  static int decimalToBcd(int dec) {
      return (dec % 10) + ((dec / 10) << 4);
  }

  I2CDriver * _driver;
};

#endif /* DS1307_H_ */
