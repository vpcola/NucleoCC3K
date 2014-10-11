#ifndef __HTTPTCPCONNECTION_H__
#define __HTTPTCPCONNECTION_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com
 * Date : Wed Oct 08 2014
 **/

#include "httptypes.h"
#include <string.h>


struct HttpConnectionParams
{
    bool isChunked;
    char host[MAX_HOST_LEN];
    char path[MAX_PATH_LEN];
    unsigned short port;
};

class HttpTcpConnection
{
    public:
        HttpTcpConnection();
        ~HttpTcpConnection();

        bool isOpen() { return _isOpen; }
        bool isConnected() { return _isConnected; }
        bool isChunked() { return _isChunked; }
        void setChunked(bool val) { _isChunked = val; }

        HTTP_RESULT open( HttpConnectionParams & param);
        HTTP_RESULT send_str( const char * str );
        HTTP_RESULT send_data( const char * buffer, size_t bufsiz);
        HTTP_RESULT receive_data(char * buffer, size_t & bufsiz);
        int recvUntil(char * buffer, size_t bufsiz, const char term);
        void consumeLine();
        HTTP_RESULT close();
        HTTP_CONTYPE getConType() { return HTTP_TCP; }
        // Reads a line from the stream, not including the terminating \r\n
        int readLine(char * buffer, size_t bufsiz);




    private:
        // Raw recv from socket or file
        HTTP_RESULT receive(char * buffer, size_t & bufsiz);
        // Read depending on content format
        virtual HTTP_RESULT chunkedRecv(char * buffer, size_t & bufsiz);
        // Reads the chunked header
        int readChunkHeader();

        int _sockfd;
        int _port;
        char _hostname[100];

        bool _isOpen;
        bool _isConnected;
        bool _isChunked;

};


#endif

