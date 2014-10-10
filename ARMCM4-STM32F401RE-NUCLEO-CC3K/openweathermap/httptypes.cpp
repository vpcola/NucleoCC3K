#include "httptypes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const char * schemes[] = {
    "http",         // HTTP_TCP
    "udp",          // HTTP_UDP
    "file",         // HTTP_FILE
    "https",        // HTTP_STCP
};
#define NUM_SCHEMES (sizeof(schemes) / sizeof(schemes[0]))

static const char * transferEncodings[] = {
    "chunked",
    "compress",
    "deflate",
    "gzip",
    "identity"
};
#define NUM_TE_TYPES (sizeof(transferEncodings) / sizeof(transferEncodings[0]))

static const char * connectionTerminations[] = {
    "close"
};
#define NUM_CT_TYPES (sizeof(connectionTerminations) / sizeof(connectionTerminations[0]))
    

HTTP_CONTYPE getConnectionType(const char * scheme)
{
    int i;
    HTTP_CONTYPE type = HTTP_TCP;

    for (i = 0; i < NUM_SCHEMES; i++)
    {
        if (strcmp(scheme, schemes[i]) == 0) 
            return (HTTP_CONTYPE) i;
    }

    return type;

}

HDR_TRANSFER_ENCODING getTEType(const char * str)
{
  int i;
  HDR_TRANSFER_ENCODING encoding = HDR_TE_UNKNOWN;
  for (i = 0; i < NUM_TE_TYPES; i++)
  {
      if (strcmp(str, transferEncodings[i]) == 0) 
          return (HDR_TRANSFER_ENCODING) i;
  }

  return encoding;
}

HDR_TERM_TYPE   getTRMType(const char * str)
{
    int i;
    HDR_TERM_TYPE ct = HDR_TRM_UNKNOWN;

    for (i = 0; i < NUM_CT_TYPES; i++)
        if (strcmp(str, connectionTerminations[i]) == 0) 
            return (HDR_TERM_TYPE) i;

    return ct;
}

char * strtrim(char * str)
{
    char *end;
    while(isspace(*str)) str++;

    if(*str == 0) // All spaces
        return str;

    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;

    *(end+1) = 0;

    return str;
}

