#ifndef __HWDEFS_H__
#define __HWDEFS_H__

/* The SERIAL driver to use */
#define SERIAL_DRIVER SD2

/* The SPI driver used */
#define CC3K_SPI_DRIVER  SPID1
#define CC3K_SPI_PORT    GPIOA
#define CC3K_SCK_PAD     5
#define CC3K_MISO_PAD    6
#define CC3K_MOSI_PAD    7

#define CC3K_IRQ_PORT    GPIOA
#define CC3K_IRQ_PAD     10
#define CC3K_CS_PORT     GPIOB
#define CC3K_CS_PAD      6
#define CC3K_EN_PORT     GPIOA
#define CC3K_EN_PAD      8

/* The EXT driver used */
#define CC3K_EXT_DRIVER  EXTD1

#define MMC_SPI_DRIVER   SPID1
#define MMC_SPI_PORT     GPIOA
#define MMC_SCK_PAD      6
#define MMC_MISO_PAD     5
#define MMC_MOSI_PAD     7
#define MMC_CS_PORT      GPIOA
#define MMC_CS_PAD       9

/* Define the GPT driver used */
#define GPT_DRIVER  GPTD3

/* Define the I2C driver used */
#define I2C_DRIVER  I2CD1
/* I2C LED Blinkers */
#define I2C_CNT_ADDR 0x21

/* LED for connection notification */
#define CON_LED_PORT    GPIOB
#define CON_LED_PIN     4

/* LED for error setup */
#define ERROR_LED_PORT      GPIOC
#define ERROR_LED_PIN       0

#define SHELL_SWITCH_PORT   GPIOB
#define SHELL_SWITCH_PIN    10


/* Access point config - arguments to wlan_connect */
#define SSID                "dlink-B15C"
#define SSID_LEN            strlen(SSID)
#define SEC_TYPE            WLAN_SEC_WPA
#define KEY                 "fricu35860"
#define KEY_LEN             strlen(KEY)
#define BSSID               NULL
#define AES_KEY             "026FD991CD"

#endif
