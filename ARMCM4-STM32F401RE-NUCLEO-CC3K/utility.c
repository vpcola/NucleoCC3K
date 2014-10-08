#include "ch.h"
#include "hal.h"
#include "utility.h"
#include "nvmem.h"
#include "shell.h"
#include "netapp.h"
#include "hwdefs.h"
#include <string.h>

float u16tofloat_trick(uint16_t x)
{
    union { float f; uint32_t i; } u; u.f = 16777216.0f; u.i |= x;
    return u.f - 16777216.0f; // optionally: (u.f - 16777216.0f) * (65536.0f / 65535.0f)
}

float u16tofloat(uint16_t x)
{
    return (float)x * (1.0f / 65535.0f);
}


long int str_to_long(const char * s, char **end, int base)
{
    long res = 0;

    if (!base)
        base = 10;
    while (*s) {
        if (end)
            *end = (void *)s;
        /* we only use it for cmdline: avoid alpha digits */
        if (base > 10 || *s < '0' || *s >= '0' + base) {
            return res;
        }
        res = res * base + *(s++) - '0';
    }
    return res;
}

void  ccprint(const char * format, ...)
{
    va_list args;

    va_start( args, format );
    chprintf((BaseSequentialStream *) &SERIAL_DRIVER, format, args );
    va_end(args);
}

void hexdump(BaseSequentialStream *chp, const char *dta, size_t size)
{
    size_t i;
    for(i = 0; i < size; i++)
        chprintf(chp, "%02X ", dta[i]);
    chprintf(chp,"\r\n");
}

void print_ip(BaseSequentialStream *chp, const char * bytes)
{
    chprintf(chp, "%d.%d.%d.%d\r\n", bytes[3], bytes[2], bytes[1], bytes[0]); 
}

void show_cc3_version(BaseSequentialStream *chp)
{
    uint8_t patchVer[2];
    nvmem_read_sp_version(patchVer);
    chprintf(chp, "CC3000 Patch Version : %d.%d\r\n", patchVer[0], patchVer[1]);
}

void show_cc3_dhcp_info(BaseSequentialStream *chp)
{
    tNetappIpconfigRetArgs ipConfig;

    netapp_ipconfig(&ipConfig);
    chprintf(chp,"-------- Connection Info -------\r\n");
    chprintf(chp,"IP : "); print_ip(chp, (const char *)ipConfig.aucIP);
    chprintf(chp,"NetMask : "); print_ip(chp, (const char *)ipConfig.aucSubnetMask);
    chprintf(chp,"Default Gateway : "); print_ip(chp, (const char *)ipConfig.aucDefaultGateway);
    chprintf(chp,"DHCP Server : "); print_ip(chp, (const char *)ipConfig.aucDHCPServer);
    chprintf(chp,"DNS Server : "); print_ip(chp, (const char *)ipConfig.aucDNSServer);
    chprintf(chp,"--------------------------------\r\n");
}

