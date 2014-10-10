#ifndef __HTTPFILECONNECTION_H__
#define __HTTPFILECONNECTION_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Thu Oct 09 2014
 **/
#include "httpconnection.h"
#include <stdlib.h>
#include <stdio.h>

class HttpFileConnection : public HttpConnection
{
    public:
        HttpFileConnection();
        virtual ~HttpFileConnection();

        virtual HTTP_RESULT open( HttpConnectionParams & param);
        virtual HTTP_RESULT send( const char * buffer, size_t bufsiz);
        virtual HTTP_RESULT recv(char * buffer, size_t & bufsiz);
        virtual int recvUntil(char * buffer, size_t bufsiz, const char term);
        virtual void consumeLine();
        virtual HTTP_RESULT close();
        virtual HTTP_CONTYPE getConType() { return HTTP_FILE; }
    private:
        FILE * _infp;

};



#endif

