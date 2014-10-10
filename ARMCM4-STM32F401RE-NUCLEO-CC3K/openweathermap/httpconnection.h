#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Wed Oct 08 2014
 **/
#include "httptypes.h"
#include <string.h>

#ifdef __linux__
#define bool_t bool
#define TRUE   true
#define FALSE  false
#endif

struct HttpConnectionParams
{
    bool_t isChunked;
    char host[MAX_HOST_LEN];
    char path[MAX_PATH_LEN];
    unsigned short port;
};

class HttpConnection
{
    public:
    HttpConnection() : _isOpen(FALSE), _isConnected(FALSE)
    {}
    virtual ~HttpConnection() {}

    bool_t isOpen() { return _isOpen; }
    bool_t isConnected() { return _isConnected; }

    virtual HTTP_RESULT open( HttpConnectionParams & param) = 0;
    virtual HTTP_RESULT send( const char * buffer, size_t bufsiz) = 0;
    virtual HTTP_RESULT recv(char * buffer, size_t & bufsiz) = 0;
    virtual int recvUntil(char * buffer, size_t bufsiz, const char term) = 0;
    virtual void consumeLine() = 0; // just reads untill '\n' char is reached
    virtual HTTP_RESULT close() = 0;
    virtual HTTP_CONTYPE getConType() = 0;


    HTTP_RESULT send( const char * str )
    {
        size_t len = strlen(str);
        return send(str, len);
    }

    // Common functions for dealing with chunked
    // data read.

    // Read depending on content format
    virtual HTTP_RESULT chunkedRecv(char * buffer, size_t & bufsiz);
    // Reads a line from the stream, not including the terminating \r\n
    virtual int readLine(char * buffer, size_t bufsiz);
    // Reads the chunked header
    int readChunkHeader();

    protected:

    bool_t _isOpen;
    bool_t _isConnected;
};

#endif

