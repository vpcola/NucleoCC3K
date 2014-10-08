#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "chprintf.h"

/* The serial driver used for all chprintf
 * functions
 **/

#ifdef __cplusplus
extern "C" {
#endif

long int str_to_long(const char * s, char **end, int base);

/* void print_ip(uint32_t); */
void ccprint(const char * format, ...);
void hexdump(BaseSequentialStream *chp, const char *, size_t size);
void print_ip(BaseSequentialStream *chp, const char *);
void show_cc3_version(BaseSequentialStream *chp);
void show_cc3_dhcp_info(BaseSequentialStream *chp);
float u16tofloat_trick(uint16_t x);
float u16tofloat(uint16_t x);

#ifdef __cplusplus
}
#endif
#endif
