#include "httptypes.h"
#include <string.h>
#include <stdlib.h>

static const char * schemes[] = {
    "http",         // HTTP_TCP
    "udp",          // HTTP_UDP
    "file"          // HTTP_FILE
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

HTTP_CONTYPE getConType(const char * scheme)
{
    int i;
    HTTP_CONTYPE type = HTTP_TCP;

    for (i = 0; i < NUM_SCHEMES; i++)
        if (strstr(scheme, schemes[i]) != NULL) return (HTTP_CONTYPE) i;

    return type;

}

HTTP_TRANSFER_ENCODING getTransferEncodingType(const char * str)
{
  int i;
  HTTP_TRANSFER_ENCODING encoding = HTTP_TE_UNKNOWN;
  for (i = 0; i < NUM_TE_TYPES; i++)
      if (strstr(str, transferEncodings[i]) != NULL) return (HTTP_TRANSFER_ENCODING) i;

  return encoding;
}
