/*
 * BMP180.cpp
 *
 *  Created on: Oct 5, 2014
 *      Author: Vergil
 */

#include "BMP180.h"
#include "math.h"

BMP180::BMP180(I2CDriver * driver, int address)
  :_driver(driver), _m_addr(address)
{
  // TODO Auto-generated constructor stub

}

msg_t BMP180::Initialize(float altitude, int overSamplingSetting)
{
  msg_t status = RDY_OK;
  uint8_t data[22];

  _m_altitude = altitude;
  _m_oss = overSamplingSetting;
  _m_temperature = UNSET_BMP180_TEMPERATURE_VALUE;
  _m_pressure = UNSET_BMP180_PRESSURE_VALUE;

  // read calibration data
  data[0]=0xAA;
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, _m_addr, data, 1, NULL, 0, 1000);
  i2cReleaseBus(_driver);

  i2cAcquireBus(_driver);
  status = i2cMasterReceiveTimeout(_driver, _m_addr, data, 22, 1000);
  i2cReleaseBus(_driver);
  chThdSleepMilliseconds(10);

  // store calibration data for further calculation
  ac1 = data[0]  << 8 | data[1];
  ac2 = data[2]  << 8 | data[3];
  ac3 = data[4]  << 8 | data[5];
  ac4 = data[6]  << 8 | data[7];
  ac5 = data[8]  << 8 | data[9];
  ac6 = data[10] << 8 | data[11];
  b1  = data[12] << 8 | data[13];
  b2  = data[14] << 8 | data[15];
  mb  = data[16] << 8 | data[17];
  mc  = data[18] << 8 | data[19];
  md  = data[20] << 8 | data[21];

  return status;
}

int BMP180::ReadData(float* pTemperature, float* pPressure)
{
  long t, p;

  if (!ReadRawTemperature(&t) || !ReadRawPressure(&p))
  {
      _m_temperature = UNSET_BMP180_TEMPERATURE_VALUE;
      _m_pressure = UNSET_BMP180_PRESSURE_VALUE;
      return 0;
  }

  _m_temperature = TrueTemperature(t);
  _m_pressure = TruePressure(p);

  if (pPressure)
      *pPressure = _m_pressure;
  if (pTemperature)
      *pTemperature = _m_temperature;

  return 1;
}

msg_t BMP180::ReadRawTemperature(long* pUt)
{
  msg_t status = RDY_OK;
  uint8_t data[2];

  // request temperature measurement
  data[0] = 0xF4;
  data[1] = 0x2E;
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, _m_addr, data, 2, NULL, 0, 1000);
  i2cReleaseBus(_driver);

  chThdSleepMilliseconds(5);

  // read raw temperature data
  data[0] = 0xF6;
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, _m_addr, data, 2, NULL, 0, 1000); // set eeprom pointer position to 0XF6
  i2cReleaseBus(_driver);


  i2cAcquireBus(_driver);
  status = i2cMasterReceiveTimeout(_driver, _m_addr, data, 2, 1000);
  i2cReleaseBus(_driver);

  *pUt = data[0] << 8 | data[1];

  return status;
}

msg_t BMP180::ReadRawPressure(long* pUp)
{
  msg_t status = RDY_OK;
  uint8_t data[2];

  // request pressure measurement
  data[0] = 0xF4;
  data[1] = 0x34 + (_m_oss << 6);
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, _m_addr, data, 2, NULL, 0, 1000);
  i2cReleaseBus(_driver);

  switch (_m_oss)
  {
      case BMP180_OSS_ULTRA_LOW_POWER:        chThdSleepMilliseconds(5); break;
      case BMP180_OSS_NORMAL:                 chThdSleepMilliseconds(8); break;
      case BMP180_OSS_HIGH_RESOLUTION:        chThdSleepMilliseconds(14); break;
      case BMP180_OSS_ULTRA_HIGH_RESOLUTION:  chThdSleepMilliseconds(26); break;
  }

  // read raw pressure data
  data[0] = 0xF6;
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, _m_addr, data, 1, NULL, 0, 1000); // set eeprom pointer position to 0XF6
  //i2cReleaseBus(_driver);

  //i2cAcquireBus(_driver);
  status = i2cMasterReceiveTimeout(_driver, _m_addr, data, 2, 1000);
  i2cReleaseBus(_driver);

  // errors += m_i2c.read(m_addr, data, 2);  // get 16 bits at this position

  *pUp = (data[0] << 16 | data[1] << 8) >> (8 - _m_oss);

  return status;
}

float BMP180::TrueTemperature(long ut)
{
    long t;

    // straight out from the documentation
    x1 = ((ut - ac6) * ac5) >> 15;
    x2 = ((long)mc << 11) / (x1 + md);
    b5 = x1 + x2;
    t = (b5 + 8) >> 4;

    // convert to celcius
    return t / 10.F;
}

float BMP180::TruePressure(long up)
{
    long p;

    // straight out from the documentation
    b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = ac2 * b6 >> 11;
    x3 = x1 + x2;
    b3 = (((ac1 * 4 + x3) << _m_oss) + 2) >> 2;
    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = ac4 * (unsigned long)(x3 + 32768) >> 15;
    b7 = ((unsigned long)up - b3)* (50000 >> _m_oss);
    if (b7 < 0x80000000)
        p = (b7 << 1) / b4;
    else
        p = (b7 / b4) << 1;
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);

    // convert to hPa and, if altitude has been initialized, to sea level pressure
    if (_m_altitude == 0.F)
        return p / 100.F;
    else
        return  p / (100.F * pow((1.F - _m_altitude / 44330.0L), 5.255L));
}
