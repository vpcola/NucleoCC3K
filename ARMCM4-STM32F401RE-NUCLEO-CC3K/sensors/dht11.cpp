#include "dht11.h"
#include "tm.h"
#include "hwdefs.h"
#include "chprintf.h"

// static TimeMeasurement tm3Obj;
 
#define SET_DIRECTION_INPUT(port, pad) palSetPadMode(port, pad, PAL_MODE_INPUT_PULLUP) 
#define SET_DIRECTION_OUTPUT(port, pad) palSetPadMode(port, pad, PAL_MODE_OUTPUT_PUSHPULL)
#define GPIO_SET(port, pad)        palSetPad(port, pad)
#define GPIO_RESET(port, pad)      palClearPad(port, pad)
#define GPIO_GET(port, pad)        palReadPad(port, pad)

DHT11::DHT11(ioportid_t port, int pad)
    : _port(port), _pad(pad)
{
    /* Initialize the timer object */
    tmObjectInit(&tmObj);

    palSetPadMode(_port,  _pad, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(_port, _pad);
}

int DHT11::read_DHT11()
{
  uint8_t i;
  int to;
  uint8_t cnt = 7;
  uint8_t idx = 0;

  // tmObjectInit(&tm3Obj);

  for ( i = 0; i < 5; i++) buf[i] = 0;
  //chSysLock();
  
  /* Start sequence */
  SET_DIRECTION_OUTPUT(_port, _pad);
  /* Send the start signal MCU -> DHT11 */
  GPIO_RESET(_port, _pad);
  chThdSleep(US2ST(18000));
  GPIO_SET(_port, _pad);
  chThdSleep(US2ST(40));
  SET_DIRECTION_INPUT(_port, _pad);

  /* Detect the start signal DHT11 -> MCU */
  to = 100000;
  while(GPIO_GET(_port, _pad) == PAL_LOW) if(to-- == 0) {  return -1; }  /* 50us on '0' */
  to = 100000;    
  while(GPIO_GET(_port, _pad) == PAL_HIGH) if(to-- == 0) {  return -2; } /* 80us on '1' */
  /* Start sequence - end */
  /* Read data sequence, iterate over 5 bytes */

  for( i = 0; i < 40; i++) 
  {
      /* Wait until line goes to 1 */
      to = 100000;
      while(GPIO_GET(_port, _pad) == PAL_LOW) if(to-- == 0) {  return -3; }

      tmObj.last = 0;
      
      /* Mark the start time */
      tmStartMeasurement(&tmObj);

      to = 100000;
      while(GPIO_GET(_port, _pad) == PAL_HIGH) if(to-- == 0) { return -4; }
      
      tmStopMeasurement(&tmObj);
      //If pulse width at 1 is greater tha 40us
      // then bit value is 1
      if ( RTT2US(tmObj.last) > 40) 
          buf[i] |= (1 << cnt);

      if (cnt == 0) 
      {
          cnt = 7;
          idx++;
      }else cnt--;   


  }

 
  /* Determine if checksum is ok */
  if (((buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF) != buf[4])
    return DHT11_CHECKSUM_ERROR;
  
  return DHT11_OK;
}

float DHT11::getTemp(int tmptype)
{
  // temperature is buf[2] (integer part) buf[3] (decimal part)
  float val = 0.0;
  
  val += buf[2];
  val += ((float) buf[3]) / 100.0;
  
  switch(tmptype)
  {
    case CELCIUS:
      return val;
    case FAHRENHEIT:
      return convertCtoF(val);
    default:
      return 0.0;
  }
  
  return val;
}

float DHT11::getHumidity()
{
  // humidity is buf[0] (integer part) buf[1] (decimal part)
  float val = 0.0;
  
  val += buf[0];
  val += ((float) buf[1]) / 100.0;
    
  return val;
}

float DHT11::convertCtoF(float ctemp)
{
  return ctemp * 9/5 + 32;
}
