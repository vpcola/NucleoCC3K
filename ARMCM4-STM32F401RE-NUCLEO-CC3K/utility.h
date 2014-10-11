#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "hwdefs.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DBG(M, ...) chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "DEBUG %s:%d: " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define WARN(M, ...) chprintf((BaseSequentialStream *)&SERIAL_DRIVER, "DEBUG %s:%d: " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)


long int str_to_long(const char * s, char **end, int base);
/* void print_ip(uint32_t); */
void ccprint(const char * format, ...);
void hexdump(BaseSequentialStream *chp, const char *, size_t size);
void print_ip(BaseSequentialStream *chp, const char *);
void show_cc3_version(BaseSequentialStream *chp);
void show_cc3_dhcp_info(BaseSequentialStream *chp);
float u16tofloat_trick(uint16_t x);
float u16tofloat(uint16_t x);
int hex2decimal(const char * hex);

#ifdef __cplusplus
}
#endif
#endif
