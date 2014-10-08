#ifndef __DHT11_H__
#define __DHT11_H__

extern "C"
{
#include "hal.h"
#include "tm.h"
}

#include <stdint.h>  

enum TMPTYPE {
  CELCIUS,
  FAHRENHEIT
};

// Define where we want to read the data
#define DHT11_DATA_GRP  GPIOC
#define DHT11_DATA_PIN  7

/* return codes : */
#define DHT11_OK 0
#define DHT11_CHECKSUM_ERROR 1  
  
#define DHT11_DATA_SIZ 5  

class DHT11{
public:
  DHT11(ioportid_t port, int pad);

  int read_DHT11();
  float getTemp(int tmptype=CELCIUS);
  float getHumidity();
private:
  float convertCtoF(float ctemp);

  TimeMeasurement tmObj;
  ioportid_t _port;
  int _pad;

  uint8_t buf[DHT11_DATA_SIZ];
};

#endif
