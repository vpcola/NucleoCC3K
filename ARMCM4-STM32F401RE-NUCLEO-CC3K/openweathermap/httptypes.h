#ifndef __HTTPTYPES_H__
#define __HTTPTYPES_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Tue Oct 07 2014
 **/

#define MAX_HOST_LEN 100
#define MAX_PATH_LEN  200

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

enum HTTP_TRANSFER_ENCODING{
  HTTP_TE_CHUNKED,
  HTTP_TE_COMPRESS,
  HTTP_TE_DEFLATE,
  HTTP_TE_GZIP,
  HTTP_TE_IDENTITY,
  HTTP_TE_UNKNOWN
};

#define DEBUG

#ifdef DEBUG
#define DBG(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define WARN(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define WARN(M, ...)
#endif

HTTP_CONTYPE getConType(const char * scheme);
HTTP_TRANSFER_ENCODING getTransferEncodingType(const char * str);


#endif

