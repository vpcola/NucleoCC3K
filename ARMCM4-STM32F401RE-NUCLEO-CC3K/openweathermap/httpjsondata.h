#ifndef __HTTPJSONDATA_H__
#define __HTTPJSONDATA_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Tue Oct 07 2014
 **/

#include "httpdata.h"
#include "httpconnection.h"

class HttpJsonData : public HttpData
{
    public:

    HttpJsonData();
    ~HttpJsonData();

    // Virtual functions provided by HttpData
    virtual HTTP_RESULT sendHeader(HttpConnection * connection);
    virtual HTTP_RESULT handleData(HttpConnection * connection);
};



#endif

