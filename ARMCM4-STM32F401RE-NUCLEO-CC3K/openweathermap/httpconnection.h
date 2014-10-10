#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

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

class HttpConnection
{
    public:
    HttpConnection() : _isOpen(false), _isConnected(false), _isChunked(false)
    {}
    virtual ~HttpConnection() {}

    bool isOpen() { return _isOpen; }
    bool isConnected() { return _isConnected; }
    bool isChunked() { return _isChunked; }
    void setChunked(bool val) { _isChunked = true; }

    virtual HTTP_RESULT open( HttpConnectionParams & param) = 0;
    virtual HTTP_RESULT send( const char * buffer, size_t bufsiz) = 0;
    HTTP_RESULT receive(char * buffer, size_t & buffer);
    virtual int recvUntil(char * buffer, size_t bufsiz, const char term) = 0;
    virtual void consumeLine() = 0; // just reads until '\n' char is reached
    virtual HTTP_RESULT close() = 0;
    virtual HTTP_CONTYPE getConType() = 0;


    HTTP_RESULT send( const char * str )
    {
        size_t len = strlen(str);
        return send(str, len);
    }


    // Reads a line from the stream, not including the terminating \r\n
    virtual int readLine(char * buffer, size_t bufsiz);

    protected:
    // Raw recv from socket or file
    virtual HTTP_RESULT recv(char * buffer, size_t & bufsiz) = 0;
    // Read depending on content format
    virtual HTTP_RESULT chunkedRecv(char * buffer, size_t & bufsiz);
    // Reads the chunked header
    int readChunkHeader();

    bool _isOpen;
    bool _isConnected;
    bool _isChunked;
};

#endif

