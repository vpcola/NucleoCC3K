/*
 * HTU21D.cpp
 *
 *  Created on: Oct 5, 2014
 *      Author: Vergil
 */
#include "HTU21D.h"
#include "utility.h"

HTU21D::HTU21D(I2CDriver * driver)
  : _driver(driver)
{

}

float HTU21D::getTemp(int tmptype)
{
  msg_t status = RDY_OK;
  uint8_t tx[1];
  uint8_t rx[2];
  float temperature = 0.0;

  tx[0] = TRIGGER_TEMP_MEASURE; // Triggers a temperature measure by feeding correct opcode.
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, HTU21D_I2C_ADDRESS, tx, 1, NULL, 0, 1000);
  i2cReleaseBus(_driver);

  //tx[0] = TRIGGER_TEMP_MEASURE; // Triggers a temperature measure by feeding correct opcode.
  //i2c_->write((HTU21D_I2C_ADDRESS << 1) & 0xFE, tx, 1);
  //wait_ms(1);
  chThdSleepMilliseconds(1);

  // Reads triggered measure
  //i2c_->read((HTU21D_I2C_ADDRESS << 1) | 0x01, rx, 2);
  //wait_ms(1);
  i2cAcquireBus(_driver);
  status = i2cMasterReceiveTimeout(_driver, HTU21D_I2C_ADDRESS, rx, 2, 1000);
  i2cReleaseBus(_driver);
  chThdSleepMilliseconds(1);


  // Algorithm from datasheet to compute temperature.
  unsigned int rawTemperature = ((unsigned int) rx[0] << 8) | (unsigned int) rx[1];
  rawTemperature &= 0xFFFC;

  float tempTemperature = rawTemperature / (float) 65536; //2^16 = 65536
  float realTemperature = (float)(-46.85f + (175.72f * tempTemperature)); //From page 14

  switch(tmptype)
  {

    case FAHRENHEIT:
      temperature = realTemperature * 1.8 + 32;
      break;
    case KELVIN:
      temperature = realTemperature + 274;
      break;
    case CELCIUS:
    default:
      temperature = realTemperature;
      break;
  }

  return temperature;
}

float HTU21D::getHumidity()
{
  msg_t status = RDY_OK;
  uint8_t tx[1];
  uint8_t rx[2];


  tx[0] = TRIGGER_HUMD_MEASURE; // Triggers a humidity measure by feeding correct opcode.
  i2cAcquireBus(_driver);
  status = i2cMasterTransmitTimeout(_driver, HTU21D_I2C_ADDRESS, tx, 1, NULL, 0, 1000);
  i2cReleaseBus(_driver);
  chThdSleepMilliseconds(1);

  // Reads triggered measure
  i2cAcquireBus(_driver);
  status = i2cMasterReceiveTimeout(_driver, HTU21D_I2C_ADDRESS, rx, 2, 1000);
  i2cReleaseBus(_driver);
  chThdSleepMilliseconds(1);

  //Algorithm from datasheet.
  unsigned int rawHumidity = ((unsigned int) rx[0] << 8) | (unsigned int) rx[1];

  rawHumidity &= 0xFFFC; //Zero out the status bits but keep them in place

  //Given the raw humidity data, calculate the actual relative humidity
  float tempRH = rawHumidity / (float)65536; //2^16 = 65536
  return (-6 + (125 * tempRH)); //From page 14

}


