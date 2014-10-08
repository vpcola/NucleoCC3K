#ifndef __HTTPDATA_H__
#define __HTTPDATA_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Tue Oct 07 2014
 **/
#include "httptypes.h"
#include "httpconnection.h"

struct HttpResponseInfo
{
  unsigned int responseCode;
  char httpversion[20];
  HTTP_TRANSFER_ENCODING transferEncoding;

    //CONTENT_TYPE contentType;
};

class HttpData {
    public:

    /* Initial request to send data to server */
    virtual HTTP_RESULT sendHeader(HttpConnection * connection) { return HTTP_OK; }
    /* Process/Send or consume data after the header is read */
    virtual HTTP_RESULT handleData(HttpConnection * connection) { return HTTP_OK; }
};



#endif

