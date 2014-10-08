/*
 * BMP180.h
 *
 *  Created on: Oct 5, 2014
 *      Author: Vergil
 */

#ifndef BMP180_H_
#define BMP180_H_

extern "C"
{
  #include "ch.h"
  #include "hal.h"
  #include "tm.h"
}

///  default address is 0xEF
#define BMP180_I2C_ADDRESS 0xEF

// Oversampling settings
#define BMP180_OSS_ULTRA_LOW_POWER 0        // 1 sample  and  4.5ms for conversion
#define BMP180_OSS_NORMAL          1        // 2 samples and  7.5ms for conversion
#define BMP180_OSS_HIGH_RESOLUTION 2        // 4 samples and 13.5ms for conversion
#define BMP180_OSS_ULTRA_HIGH_RESOLUTION 3  // 8 samples and 25.5ms for conversion

#define UNSET_BMP180_PRESSURE_VALUE 0.F
#define UNSET_BMP180_TEMPERATURE_VALUE -273.15F // absolute zero


class BMP180 {
public:
  BMP180(I2CDriver * driver, int address = BMP180_I2C_ADDRESS);

  msg_t Initialize(float altitude = 0.F, int overSamplingSetting = BMP180_OSS_NORMAL);
  int ReadData(float* pTemperature = NULL, float* pPressure = NULL);
  float GetTemperature() {return _m_temperature;};
  float GetPressure() {return _m_pressure;};

private:
  I2CDriver * _driver;

  msg_t ReadRawTemperature(long* pUt);
  msg_t ReadRawPressure(long* pUp);
  float TrueTemperature(long ut);
  float TruePressure(long up);


  float _m_temperature;
  float _m_pressure;
  float _m_altitude;

  int _m_addr;
  char _m_data[4];
  int _m_oss;

  // Calibration data
  short ac1, ac2, ac3;
  unsigned short ac4, ac5, ac6;
  short b1, b2;
  short mb, mc, md;
  long x1, x2, x3, b3, b5, b6;
  unsigned long b4, b7;

};

#endif /* BMP180_H_ */
