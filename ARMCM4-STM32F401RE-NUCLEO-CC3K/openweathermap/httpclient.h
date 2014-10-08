#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

/**
 * Author : Cola Vergil
 * Email  : tkcov@svsqdcs01.telekurs.com
 * Date : Tue Oct 07 2014
 **/

#include <stdlib.h>
#include <stdint.h>

#include "httpdata.h"
#include "httptypes.h"
#include "httpconnection.h"

#define HTTP_TIMEOUT 1000
#define CHUNK_SIZE   128


class HttpClient {
    public:
    HttpClient();
    ~HttpClient();

    HTTP_RESULT get(const char * url, char * result, size_t result_len, int timeout = HTTP_TIMEOUT);
    HTTP_RESULT connect(const char * url, HTTP_METHOD method, HttpData * handler);

    private:
    
    int _sockfd;
    static const char * _thishost;

    static HttpConnection * createConnection(const char * scheme);

    bool_t parseHttpHeaders(const char * line, HttpResponseInfo & info);
    HTTP_RESULT parseURL(const char * url, char * scheme, size_t maxschemelen, 
            char * host, size_t maxhostlen, 
            uint16_t * port, 
            char * path, size_t maxpathlen);

    HTTP_RESULT sendHeaders(HttpConnection * con, const char * path, HTTP_METHOD method); 
    HTTP_RESULT receiveHeaders(HttpConnection * con, HttpData  * handler);

};

#endif

