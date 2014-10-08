/*
 * DS1307.cpp
 *
 *  Created on: Oct 5, 2014
 *      Author: Vergil
 */

#include "DS1307.h"
#include "chprintf.h"

static const char * weekdays[] = { "Saturday",
                                   "Sunday",
                                   "Monday",
                                   "Tuesday",
                                   "Wednesday",
                                   "Thursday",
                                   "Friday" };

static const char * months[] = { "January",
                                 "February",
                                 "March",
                                 "April",
                                 "May",
                                 "June",
                                 "July",
                                 "August",
                                 "September",
                                 "October",
                                 "November",
                                 "December" };

char RTC_TIME::tmpbuff[] = {0};

const char * RTC_TIME::to_str()
{
  // Prints Monday October 6, 2014 12:00:00

  chsnprintf(tmpbuff, DS1307_STRBUFSIZ, "%s %s %2d, %4d %02d:%02d:%02d",
          weekdays[wday],
          months[mon],
          date,
          year + 1900, // Date starts at 1900
          hour,
          min,
          sec);

  return tmpbuff;
}

DS1307::DS1307(I2CDriver * driver)
:_driver(driver)
{
  // TODO Auto-generated constructor stub

}

bool DS1307::get_time(RTC_TIME & time)
{
    msg_t status = RDY_OK;
    uint8_t addr = 0x00; // memory address
    uint8_t buffer[7];

    i2cAcquireBus(_driver);
    status = i2cMasterTransmitTimeout(_driver, DS1307_ADDR, &addr, 1, NULL, 0, 1000);
    i2cReleaseBus(_driver);

    if (status != RDY_OK) return false;

    i2cAcquireBus(_driver);
    status = i2cMasterReceiveTimeout(_driver, DS1307_ADDR, buffer, 7, 1000);
    i2cReleaseBus(_driver);
    if (status != RDY_OK) return 0;

    if (buffer[0] & 0x80) return 0; // clock stopped
    if (buffer[2] & 0x40) return 0; // 12-hour format not supported
    time.sec = bcdToDecimal(buffer[0] & 0x7F);
    time.min = bcdToDecimal(buffer[1]);
    time.hour = bcdToDecimal(buffer[2] & 0x3F);
    time.wday = bcdToDecimal(buffer[3]) - 1;
    time.date = bcdToDecimal(buffer[4]);
    time.mon = bcdToDecimal(buffer[5]) - 1;
    time.year = bcdToDecimal(buffer[6]) + 2000 - 1900;

    return true;
}

bool DS1307::set_time(const RTC_TIME time)
{
    msg_t status = RDY_OK;
    uint8_t buffer[9];


    buffer[0] = 0x00; // memory address
    buffer[1] = decimalToBcd(time.sec) & 0x7F; // CH = 0
    buffer[2] = decimalToBcd(time.min);
    buffer[3] = decimalToBcd(time.hour) & 0x3F; // 24-hour format
    buffer[4] = time.wday + 1;
    buffer[5] = decimalToBcd(time.date);
    buffer[6] = decimalToBcd(time.mon+1);
    buffer[7] = decimalToBcd(time.year + 1900 - 2000);
    buffer[8] = 0x00; // OUT = 0

    i2cAcquireBus(_driver);
    status = i2cMasterTransmitTimeout(_driver, DS1307_ADDR, buffer, 9, NULL, 0, 1000);
    i2cReleaseBus(_driver);

    if (status != RDY_OK) return false;

    return true;
}
