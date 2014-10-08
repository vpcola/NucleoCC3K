#ifndef __HTTPTCPCONNECTION_H__
#define __HTTPTCPCONNECTION_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com
 * Date : Wed Oct 08 2014
 **/
#include "httpconnection.h"

class HttpTcpConnection : public HttpConnection
{
    public:
        HttpTcpConnection();
        virtual ~HttpTcpConnection();

        virtual HTTP_RESULT open( HttpConnectionParams & param);
        virtual HTTP_RESULT send( const char * buffer, size_t bufsiz);
        virtual HTTP_RESULT recv(char * buffer, size_t bufsiz, bool chunked = false);
        virtual int readLine(char * buffer, size_t bufsiz);
        virtual HTTP_RESULT close();
        virtual HTTP_CONTYPE getConType() { return HTTP_TCP; }
    private:
        int recvUntil(char * buffer, size_t bufsiz, const char term);
        HTTP_RESULT rawRecv(char * buffer, size_t bufsiz);
        HTTP_RESULT chunkedRecv(char * buffer, size_t bufsiz);
        void consumeLine();
        int readChunkHeader();


        int _sockfd;
        int _port;
        char _hostname[100];

};


#endif

