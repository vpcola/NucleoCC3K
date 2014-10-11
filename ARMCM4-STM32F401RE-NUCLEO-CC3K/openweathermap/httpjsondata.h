#ifndef __HTTPJSONDATA_H__
#define __HTTPJSONDATA_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Tue Oct 07 2014
 **/

#include "httpdata.h"
#include "jsmn.h"

class HttpJsonData : public HttpData
{
    public:

    HttpJsonData();
    ~HttpJsonData();

    // Virtual functions provided by HttpData
    virtual HTTP_RESULT sendHeader(HttpTcpConnection * connection);
    virtual HTTP_RESULT handleData(HttpTcpConnection * connection, HttpResponseInfo & info);

    private:

    int jsonGetVal(jsmntok_t * tokens,
                   const char * js,
                   const char * section,
                   const char * varname,
                   char * value, size_t len );

};



#endif

