#ifndef __HTTPTYPES_H__
#define __HTTPTYPES_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Tue Oct 07 2014
 **/

#define MAX_HOST_LEN 100
#define MAX_PATH_LEN  200

#include <stdio.h>

enum HTTP_RESULT {
    HTTP_RECV_CHNK_ERROR = -7,
    HTTP_RECV_BUFF_ERROR = -6,
    HTTP_RECV_ERROR = -5,
    HTTP_SEND_ERROR = -4,
    HTTP_SENDHEADER_ERROR = -3,
    HTTP_CONNECT_ERROR = -2,
    HTTP_PARSE_ERROR = -1,
    HTTP_OK = 0
};

enum HTTP_CONTYPE {
    HTTP_TCP = 0,
    HTTP_UDP,
    HTTP_FILE,
    HTTP_STCP // Secure TCP (i.e. https)
};

enum HTTP_METHOD{
    HTTP_GET,
    HTTP_POST
};

enum HDR_TERM_TYPE
{
    HDR_TRM_CLOSE,
    HDR_TRM_UNKNOWN
};

enum HDR_TRANSFER_ENCODING{
  HDR_TE_CHUNKED,
  HDR_TE_COMPRESS,
  HDR_TE_DEFLATE,
  HDR_TE_GZIP,
  HDR_TE_IDENTITY,
  HDR_TE_UNKNOWN
};


#define DEBUG

#ifdef DEBUG
#define DBG(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define WARN(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define WARN(M, ...)
#endif

HTTP_CONTYPE getConnectionType(const char * scheme);
HDR_TRANSFER_ENCODING getTEType(const char * str);
HDR_TERM_TYPE   getTRMType(const char * str);

// TODO: Move this function to utility
char * strtrim(char * str);

#endif

