#ifndef __HTTPDATA_H__
#define __HTTPDATA_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Tue Oct 07 2014
 **/

#include "ch.h"
#include "utility.h"
#include "httptcpconnection.h"

struct HttpResponseInfo
{
  unsigned int responseCode;
  char httpversion[20];
  char responseString[20];
  HDR_TRANSFER_ENCODING transferEncoding;
  HDR_TERM_TYPE termType;
  char contentType[100];
  int contentLength;

  void debug()
  {
    DBG("Response Header = [%s] [%d] [%s]\n",
        httpversion,
        responseCode,
        responseString);
    DBG("Server headers:\n");
    DBG("Content-Type: [%s]\n", contentType);
    DBG("Content Termination: [%d]\n", termType);
    DBG("Content Length: [%d]\n", contentLength);
    DBG("Transfer Encoding: [%d]\n", transferEncoding);

  }
};

class HttpData {
    public:

    virtual bool handleServerResponse(HttpResponseInfo & info)
    {
      if (info.responseCode == 200)
        return true;

      return false;
    }
    /* Initial request to send data to server */
    virtual HTTP_RESULT sendHeader(HttpTcpConnection * connection) { return HTTP_OK; }
    /* Process/Send or consume data after the header is read */
    virtual HTTP_RESULT handleData(HttpTcpConnection * connection, HttpResponseInfo & info) { return HTTP_OK; }

};



#endif

