/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "test.h"
#include "ff.h"
#include "cc3000_chibios_api.h"
#include "hwdefs.h"
#include "nvmem.h"
#include "security.h"
#include "socket.h"
#include "netapp.h"
#include "HTU21D.h"
#include "BMP180.h"
#include "DS1307.h"
#include "utility.h"

#include <stdlib.h>
#include <string.h>

/*===========================================================================*/
/* Hardware configurations                                                   */
/*===========================================================================*/
static const SerialConfig serialcfg = {
    115200, // baud rate
    0,
    0,
    0,
};

/* BR bits and clock speeds:
 *  000  /2   =   42 MHz
 *  001  /4   =   21 MHz
 *  010  /8   =   10 MHz
 *  011  /16  =   5.25 MHz
 *  100  /32  =   2 MHz
 *  101  /64  =   1 kHz
 *  110  /128 =   250 kHz
 *  111  /256 =   125 kHz
 **/
static SPIConfig chCC3SpiConfig = {
    NULL,
    CHIBIOS_CC3000_NSS_PORT,
    CHIBIOS_CC3000_NSS_PAD,
    /* 2nd clock transition first data capture edge */
    /* CPHA = 1, BR: 011 - 2 MHz */
    SPI_CR1_CPHA | SPI_CR1_BR_0
    //SPI_CR1_CPHA | (SPI_CR1_BR_1 | SPI_CR1_BR_0 )
};

static EXTConfig chExtConfig;

/* I2C interface #2 */
static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2,
};

static const GPTConfig gpt3cfg = {
    1000000, /* 1 MHz timer */
    NULL,
    0
};

/**
 * @brief FS object.
 */
FATFS MMC_FS;

/**
 * MMC driver instance.
 */
MMCDriver MMCD1;



/* BR bits and clock speeds:
*  000  /2   =   42 MHz
*  001  /4   =   21 MHz
*  010  /8   =   10 MHz
*  011  /16  =   5.25 MHz
*  100  /32  =   2 MHz
*  101  /64  =   1 kHz
*  110  /128 =   250 kHz
*  111  /256 =   125 kHz
**/

/* Maximum speed SPI configuration (18MHz, CPHA=0, CPOL=0, MSb first).*/
static SPIConfig hs_spicfg = {NULL, MMC_CS_PORT, MMC_CS_PAD, SPI_CR1_BR_0};

/* Low speed SPI configuration (281.250kHz, CPHA=0, CPOL=0, MSb first).*/
static SPIConfig ls_spicfg = {NULL, MMC_CS_PORT, MMC_CS_PAD,
                              SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0};

/* MMC/SD over SPI driver configuration.*/
static MMCConfig mmccfg = {&MMC_SPI_DRIVER, &ls_spicfg, &hs_spicfg};

static char smartConfigPrefix[] = { 'T', 'T', 'T' };
static char deviceName[] = "CC3000";

/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/

/* FS mounted and ready.*/
static bool_t fs_ready = FALSE;

/* Generic large buffer.*/
uint8_t fbuff[1024];

/* Root Path */
char rootpath[50] = "/";

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int i;
  char *fn;

#if _USE_LFN
  fno.lfname = 0;
  fno.lfsize = 0;
#endif
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;
      if (fno.fname[0] == '.')
        continue;
      fn = fno.fname;
      if (fno.fattrib & AM_DIR) {
        path[i++] = '/';
        strcpy(&path[i], fn);
        res = scan_files(chp, path);
        if (res != FR_OK)
          break;
        path[--i] = 0;
      }
      else {
        chprintf(chp, "%s/%s\r\n", path, fn);
      }
    }
  }
  return res;
}

/**
 * Mount a default filesystem on the root FS
 **/
static int mountFS(void)
{
    FRESULT err;
    if (mmcConnect(&MMCD1))
    {
        chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "Can not connect to MMC!\r\n");
        return 0;
    }
    err = f_mount(0, &MMC_FS);
    if (err != FR_OK)
    {
        mmcDisconnect(&MMCD1);
        return 0;
    }
    chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "Card mounted!\r\n");
    fs_ready = TRUE;
    return 1;
}

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WA_SIZE(1024)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriority(),
                           TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

static void cmd_setup_cc3000(BaseSequentialStream *chp, int argc, char *argv[])
{
    char ssid_name[32];
    char ssid_key[40];
    char ssid_sectype[3];
    unsigned long sectype;
    unsigned char rval;

    (void)argv;
    if(argc > 0)
    {
        chprintf(chp, "Usage: smartcfg\r\n");
        return;
    }

    chprintf(chp, "Enter Security profile: \r\n");
    chprintf(chp, "  [0] Unsecured\r\n");
    chprintf(chp, "  [1] WEP\r\n");
    chprintf(chp, "  [2] WPA\r\n");
    chprintf(chp, "  [3] WPA2\r\n");

    shellGetLine(chp, ssid_sectype, 3);
    sectype = ssid_sectype[0] - '0';

    chprintf(chp, "Enter the AP SSID\r\n");
    shellGetLine(chp, ssid_name, 32);

    chprintf(chp, "Enter SSID Key\r\n");
    shellGetLine(chp, ssid_key, 40);

    chprintf(chp, "Type[%d]\r\n", sectype);
    chprintf(chp, "SSID[%s]\r\n", ssid_name);
    chprintf(chp, "Passkey[%s]\r\n", ssid_key);

    chprintf(chp, "Disabling connection policy ...\r\n");
    wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

    chprintf(chp, "Adding profile ...\r\n");
    rval = wlan_add_profile(sectype,
            (unsigned char *) ssid_name,
            strlen(ssid_name),
            NULL,
            0,
            0x18,
            0x1e,
            0x2,
            (unsigned char *) ssid_key,
            strlen(ssid_key)
            );

    if (rval != 255)
    {
        chprintf(chp, "Add profile returned: %d\r\n", rval);

        chprintf(chp, "Enabling auto connect ...\r\n");
        wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);
        chprintf(chp, "Restarting wlan ...\r\n");
        wlan_stop();
        chThdSleep(MS2ST(500));
        wlan_start(0);

        return;

    }else
    {
        // TODO: Delete profiles and add again
        chprintf(chp, "Error adding profile (list full) ...\r\n");
        return;
    }

    chprintf(chp, "Lan profile added!\r\n");
}


static void cmd_smart_cc3000(BaseSequentialStream *chp, int argc, char *argv[])
{
    long rval;

    (void)argv;
    if(argc > 0)
    {
        chprintf(chp, "Usage: setup\r\n");
        return;
    }

    chprintf(chp, "Disconnecting ");
    wlan_disconnect();
    while(cc3000AsyncData.connected == 1)
    {
        chprintf(chp, ".");
    }
    chprintf(chp, "done!\r\n");

    chprintf(chp, "Disabling auto connect policy ...\r\n");
    /* Set connection policy to disable */
    if ((rval = wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE)) != 0)
    {
        chprintf(chp, "Error disabling auto connect policy ...\r\n");
        return;
    }

    /* Delete all profiles */
    chprintf(chp, "Deleting all profiles ...\r\n");
    if ((rval = wlan_ioctl_del_profile(255)) != 0)
    {
        chprintf(chp, "Error deleting profiles ...\r\n");
        return;
    }


    chprintf(chp, "Creating AES keys ...\r\n");
    nvmem_create_entry(NVMEM_AES128_KEY_FILEID, AES128_KEY_SIZE);
    aes_write_key((UINT8 *) AES_KEY);

    // Set the smart config prefix
    chprintf(chp, "Setting smart config prefix ...\r\n");
    if((rval = wlan_smart_config_set_prefix(smartConfigPrefix)) != 0)
    {
        chprintf(chp, "Error setting smart config prefix ... \r\n");
        return;
    }
    chprintf(chp, "Starting CC3000 SmartConfig ...\r\n");
    if((rval = wlan_smart_config_start(0)) != 0)
    {
        chprintf(chp, "Error starting smart config ...\r\n");
        return;
    }

    chprintf(chp, "Waiting for SmartConfig to finish ...\r\n");
    /* Wait until smart config is finished */
    while(cc3000AsyncData.smartConfigFinished == 0)
    {
        // We blink the led here .. the thread process
        // will set this to PAL_LOW (since we are disconnected)
        palWritePad(CON_LED_PORT, CON_LED_PIN, PAL_HIGH);
        chThdSleep(MS2ST(200));
    }

    chprintf(chp, "Smart config packet received ...\r\n");
    /* Re enable auto connect policy */
    if ((rval = wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE)) != 0)
    {
        chprintf(chp, "Error setting auto connect policy ...\r\n");
        return;
    }

    /* Restart wlan */
    wlan_stop();
    chprintf(chp, "Reconnecting ...\r\n");
    chThdSleep(MS2ST(2000));
    wlan_start(0);
    /* No need to call connect, hopefully auto connect policy
     * can connect to our AP now
     **/
    chprintf(chp, "Waiting for connection to AP ...\r\n");
    while (cc3000AsyncData.dhcp.present != 1)
    {
        chThdSleep(MS2ST(5));
    }

    show_cc3_dhcp_info(chp);
    mdnsAdvertiser(1, deviceName, strlen(deviceName));

}

static void cmd_ipconfig(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;
    if(argc > 0)
    {
        chprintf(chp, "Usage: ipconfig\r\n");
        return;
    }

    if (cc3000AsyncData.connected != 1)
    {
        chprintf(chp, "Not connected!\r\n");
        return;
    }

    show_cc3_dhcp_info(chp);
}

static void cmd_ping(BaseSequentialStream *chp, int argc, char *argv[])
{
    uint32_t remoteHostIp;
    (void)argv;
    if (argc != 1)
    {
        chprintf(chp, "Usage:\r\n");
        chprintf(chp, "ping <hostname>\r\n");
        return;
    }


    chprintf(chp,"Looking up IP of %s...\r\n", argv[0]);
    gethostbyname(argv[0], strlen(argv[0]), &remoteHostIp);
    chprintf(chp,"Pinging...", NULL); print_ip(chp, (const char *) &remoteHostIp);
    remoteHostIp = htonl(remoteHostIp);

    memset((void *)&cc3000AsyncData.ping, 0, sizeof(cc3000AsyncData.ping));
    netapp_ping_send(&remoteHostIp, 3, 10, 3000);

    while (cc3000AsyncData.ping.present != TRUE)
    {
        chThdSleep(MS2ST(100));
    }
    chprintf(chp,"--Ping Results--:\r\n", NULL);
    chprintf(chp,"Number of Packets Sent: %u\r\n", cc3000AsyncData.ping.report.packets_sent);
    chprintf(chp,"Number of Packet Received: %u\r\n", cc3000AsyncData.ping.report.packets_received);
    chprintf(chp,"Min Round Time: %u\r\n", cc3000AsyncData.ping.report.min_round_time);
    chprintf(chp,"Max Round Time: %u\r\n", cc3000AsyncData.ping.report.max_round_time);
    chprintf(chp,"Avg Round Time: %u\r\n", cc3000AsyncData.ping.report.avg_round_time);
    chprintf(chp,"--End of Ping Results--\r\n", NULL);

}

static void cmd_getsensor(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) return;


    HTU21D tmpSensor(&I2C_DRIVER);
    BMP180 tmpBarometer(&I2C_DRIVER);

    float temperature = tmpSensor.getTemp();
    float humidity = tmpSensor.getHumidity();
    float pressure;

    chprintf(chp,"Temperature : %.2f C\r\n", temperature);
    chprintf(chp,"Humidity : %.2f %%\r\n", humidity);
    if (tmpBarometer.ReadData(&temperature, &pressure))
      chprintf(chp, "Pressure : %.2f Hpa\r\n", pressure);
}

static void cmd_listfiles(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) return;

    if(fs_ready)
    {
      chprintf(chp, "Listing files on the SD card ...\r\n");
      scan_files(chp, rootpath);
    }
}

static void cmd_gettime(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) return;

    RTC_TIME tm;
    DS1307 rtc(&I2C_DRIVER);

    if (rtc.get_time(tm))
      chprintf(chp, "%s\r\n", tm.to_str());
    else
      chprintf(chp, "Failed to get system time\r\n");
}

static void cmd_settime(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) return;

    RTC_TIME tm;
    char line[SHELL_MAX_LINE_LENGTH];

    DS1307 rtc(&I2C_DRIVER);


    chprintf(chp, "Enter the date (date 1..31): ", NULL);
    shellGetLine(chp, line, sizeof(line));
    tm.date = atol(line);
    chprintf(chp, "Enter the date (month 1..12): ", NULL);
    shellGetLine(chp, line, sizeof(line));
    tm.mon = atol(line);
    chprintf(chp, "Enter the date (year): ", NULL);
    shellGetLine(chp, line, sizeof(line));
    tm.year = atol(line);
    chprintf(chp, "Enter the time (hours 0..23): ", NULL);
    shellGetLine(chp, line, sizeof(line));
    tm.hour = atol(line);
    chprintf(chp, "Enter the time (minutes 0..59): ", NULL);
    shellGetLine(chp, line, sizeof(line));
    tm.min = atol(line);
    chprintf(chp, "Enter the time (seconds 0..59): ", NULL);
    shellGetLine(chp, line, sizeof(line));
    tm.sec = atol(line);
    chprintf(chp, "Setting RTC with %04d/%02d/%02d %02d:%02d:%02d\r\n",
             tm.year, tm.mon, tm.date, tm.hour, tm.min, tm.sec);
    // TODO some validation here
    tm.year = tm.year - 1900;
    tm.mon = tm.mon - 1; // Months since Jan (0-11)

    if(!rtc.set_time(tm))
      chprintf(chp, "Failed setting up RTC time\r\n", NULL);
}

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"test", cmd_test},
  {"setup", cmd_setup_cc3000},
  {"smartcfg", cmd_smart_cc3000},
  {"ipconfig", cmd_ipconfig},
  {"ping", cmd_ping},
  {"sensor", cmd_getsensor},
  {"ls", cmd_listfiles},
  {"gettime", cmd_gettime},
  {"settime", cmd_settime},
  {NULL, NULL}
};

static const ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SD2,
  commands
};

/*===========================================================================*/
/* Initialization Routines                                                   */
/*===========================================================================*/

static void initI2CHw(void)
{
    i2cStart(&I2C_DRIVER, &i2cfg1);
    palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUDR_PULLUP);

}

static void initSerialHw(void)
{
  sdStart(&SERIAL_DRIVER, &serialcfg);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
}

static void initMMCSpiHw(void)
{
    /* Setup CC3000 SPI pins Chip Select*/
    palSetPad(MMC_CS_PORT, MMC_CS_PAD);
    palSetPadMode(MMC_CS_PORT, MMC_CS_PAD,
        PAL_MODE_OUTPUT_PUSHPULL |
        PAL_STM32_OSPEED_LOWEST);     /* 400 kHz */

    /* General SPI pin setting */
    palSetPadMode(MMC_SPI_PORT, MMC_SCK_PAD,
        PAL_MODE_ALTERNATE(5) |       /* SPI SCK */
        PAL_STM32_OTYPE_PUSHPULL |
        PAL_STM32_OSPEED_MID2);       /* 10 MHz */
    palSetPadMode(MMC_SPI_PORT, MMC_MISO_PAD,
        PAL_MODE_ALTERNATE(5));       /* SPI MISO */
    palSetPadMode(MMC_SPI_PORT, MMC_MOSI_PAD,
        PAL_MODE_ALTERNATE(5) |       /* SPI MOSI */
        PAL_STM32_OTYPE_PUSHPULL |
        PAL_STM32_OSPEED_MID2);       /* 10 MHz */

    mmcObjectInit(&MMCD1);
    mmcStart(&MMCD1, &mmccfg);

}

static void initCC3000SpiHw(void)
{
    /* Setup CC3000 SPI pins Chip Select*/
    palSetPad(CHIBIOS_CC3000_NSS_PORT, CHIBIOS_CC3000_NSS_PAD);
    palSetPadMode(CHIBIOS_CC3000_NSS_PORT, CHIBIOS_CC3000_NSS_PAD,
            PAL_MODE_OUTPUT_PUSHPULL |
            PAL_STM32_OSPEED_LOWEST);     /* 400 kHz */

    /* General SPI pin setting */
    palSetPadMode(CHIBIOS_CC3000_SPI_PORT, CHIBIOS_CC3000_SCK_PAD,
            PAL_MODE_ALTERNATE(5) |       /* SPI SCK */
            PAL_STM32_OTYPE_PUSHPULL |
            PAL_STM32_OSPEED_MID2);       /* 10 MHz */
    palSetPadMode(CHIBIOS_CC3000_SPI_PORT, CHIBIOS_CC3000_MISO_PAD,
            PAL_MODE_ALTERNATE(5));       /* SPI MISO */
    palSetPadMode(CHIBIOS_CC3000_SPI_PORT, CHIBIOS_CC3000_MOSI_PAD,
            PAL_MODE_ALTERNATE(5) |       /* SPI MOSI */
            PAL_STM32_OTYPE_PUSHPULL |
            PAL_STM32_OSPEED_MID2);       /* 10 MHz */

    /* Setup IRQ pin */
    palSetPadMode(CHIBIOS_CC3000_IRQ_PORT, CHIBIOS_CC3000_IRQ_PAD,
            PAL_MODE_INPUT_PULLUP);

    /* Setup WLAN EN pin.
     * With the pin low, we sleep here to make sure
     * CC3000 is off.
     **/
    palClearPad(CHIBIOS_CC3000_WLAN_EN_PORT, CHIBIOS_CC3000_WLAN_EN_PAD);
    palSetPadMode(CHIBIOS_CC3000_WLAN_EN_PORT, CHIBIOS_CC3000_WLAN_EN_PAD,
            PAL_MODE_OUTPUT_PUSHPULL |
            PAL_STM32_OSPEED_LOWEST);     /* 400 kHz */

    /* Initialize EXT and SPI drivers */
    extObjectInit(&EXTD1);
    spiObjectInit(&SPID1);


}

/* Start CC3000, must be called after EXT_DRIVER and
 * SPI_DRIVER are initialized
 **/
static void initCC3000Hw(void)
{
    chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "wlan_init ...");
    cc3000ChibiosWlanInit(&CC3K_SPI_DRIVER, &chCC3SpiConfig,
            &CC3K_EXT_DRIVER, &chExtConfig,
            0,0,0, ccprint);
    chprintf((BaseSequentialStream *)&SERIAL_DRIVER,"done!\r\n");

    chprintf((BaseSequentialStream *)&SERIAL_DRIVER,"wlan_start ...");
    wlan_start(0);
    chprintf((BaseSequentialStream *)&SERIAL_DRIVER,"done!\r\n");

}



/*===========================================================================*/
/* Threads                                                                   */
/*===========================================================================*/


/*
 * Red LED blinker thread, times are in milliseconds.
 */
/*
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palClearPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(500);
  }
}
*/

/*
 * Counter thread, times are in milliseconds.
 */
static WORKING_AREA(waThread2, 128);
static msg_t Thread2(void *arg) {

    (void)arg;
    uint8_t data[2], count = 0;
    msg_t status = RDY_OK;

    chRegSetThreadName("counter");

    // First set the direction registers
    data[0] = 0x00; // direction reg
    data[1] = 0x00; // all outputs
    i2cAcquireBus(&I2C_DRIVER);
    status = i2cMasterTransmitTimeout(&I2C_DRIVER, I2C_CNT_ADDR, data, 2, NULL, 0, 1000);
    i2cReleaseBus(&I2C_DRIVER);

    data[0] = 0x14; // output reg
    while (TRUE) {
        data[1] = count;
        i2cAcquireBus(&I2C_DRIVER);
        status = i2cMasterTransmitTimeout(&I2C_DRIVER, I2C_CNT_ADDR, data, 2, NULL, 0, 1000);
        i2cReleaseBus(&I2C_DRIVER);
        chThdSleepMilliseconds(100);
        count++;
    }

    // keep compiler happy
    return status;
}

/*===========================================================================*/
/* Main Function                                                             */
/*===========================================================================*/

/*
 * Application entry point.
 */
int main(void) {

  Thread *shelltp = NULL;


  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  initSerialHw();

  /*
   * Initialize GPT driver 3
   */
  gptStart(&GPT_DRIVER, &gpt3cfg);

  /*
   * Initialize the MMC over SPI hardware
   */
   chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "Initializing MMC Hardware...\r\n");
   initMMCSpiHw();
   chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "Mounting filesystem ...\r\n");
   /* Do an initial ls */
   if (mountFS())
   {
     chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "Listing files on the SD card ...\r\n");
     scan_files((BaseSequentialStream *)&SERIAL_DRIVER, rootpath);
   }




  /* Initialize I2C1 */
  initI2CHw();

  /* Initalize CC3000 Hardware */
  chprintf((BaseSequentialStream *)&SERIAL_DRIVER,"Initializing CC3000 SPI hardware ...");
  initCC3000SpiHw();
  chprintf((BaseSequentialStream *)&SERIAL_DRIVER,"done!\r\n");
  chprintf((BaseSequentialStream *)&SERIAL_DRIVER,"Initializing CC3000 ...\r\n");
  initCC3000Hw();

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);


  /*
   * Shell manager initialization.
   */
  shellInit();


  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and listen for events.
   */
  while (TRUE) {
      if (!shelltp)
          shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, NORMALPRIO);
      else if (chThdTerminated(shelltp)) {
          chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
          shelltp = NULL;           /* Triggers spawning of a new shell.        */
      }
      chThdSleepMilliseconds(500);
  }

}
