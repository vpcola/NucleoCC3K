#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com 
 * Date : Wed Oct 08 2014
 **/
#include "httptypes.h"
#include <string.h>
#include "cc3000_chibios_api.h"
#include "socket.h"
#include "utility.h"

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
    virtual HTTP_RESULT recv(char * buffer, size_t bufsiz, bool chunked = false) = 0;
    // Reads a line from the stream, not including the terminating \r\n
    virtual int readLine(char * buffer, size_t bufsiz) = 0;
    virtual HTTP_RESULT close() = 0;
    virtual HTTP_CONTYPE getConType() = 0;

    HTTP_RESULT send( const char * str )
    {
        size_t len = strlen(str);
        return send(str, len);
    }

    protected:

    bool_t _isOpen;
    bool_t _isConnected;
};

#endif

