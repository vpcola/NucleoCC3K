#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpclient.h"
#include "httptcpconnection.h"


const char * HttpClient::_thishost = "127.0.0.1";
static const char * methods[] = { "GET", "POST" };

HttpClient::HttpClient()
    : _sockfd(0)
{
}

HttpClient::~HttpClient()
{
}

HTTP_RESULT get(const char * url, char * result, size_t result_len, int timeout)
{
    return HTTP_OK;
}

HttpConnection * HttpClient::createConnection(const char * scheme)
{
   switch(getConType(scheme))
   {
       case HTTP_TCP : return new HttpTcpConnection();
       case HTTP_UDP :
       case HTTP_FILE :
       case HTTP_STCP:
            return NULL;
   }

   return NULL;
}

HTTP_RESULT HttpClient::connect(const char * url, HTTP_METHOD method, HttpData * handler)
{
    char scheme[8];
    HttpConnectionParams params;


    HTTP_RESULT res = parseURL(url, scheme, sizeof(scheme), params.host, sizeof(params.host), &(params.port), params.path, sizeof(params.path));
    if (res != HTTP_OK) 
        return res;

    // print the result
    DBG("scheme [%s]\n", scheme);
    DBG("host [%s]\n", params.host);
    DBG("port [%d]\n", params.port);
    DBG("path [%s]\n", params.path);

    // Instantiate a connection based on the scheme
    HttpConnection * _connection = createConnection(scheme);
    if (_connection)
    {
        // Open the connection
        res = _connection->open(params);
        if ( res != HTTP_OK)
            return res;

        // Send default headers to http server
        res = sendHeaders(_connection, params.path, method);
        if (res != HTTP_OK)
            return res;

        // Send any specific headers that the data
        // handler might have
        if (handler)
        {
            // Send specific headers used by the data handlers
            res = handler->sendHeader(_connection);
            if (res != HTTP_OK)
                return res;

        }

        // Send \r\n to terminate header sending
        res = _connection->send("\r\n");
        if (res != HTTP_OK)
            return res;

        // Now read the response header
        res = receiveHeaders(_connection, handler);
        if (res != HTTP_OK)
            return res;


        // Finally handle the response data
        if (handler)
            // Handle server response
            res = handler->handleData(_connection);
        else
        {
            // Just consume the data from the server
        }

        // Finally close the socket
        _connection->close();

        // And free up memory
        delete _connection;
    }


    return res;
}

HTTP_RESULT HttpClient::sendHeaders(HttpConnection * conn, const char * path, HTTP_METHOD method) 
{
    int numbytes;
    char buf[CHUNK_SIZE];

    if (!conn) return HTTP_CONNECT_ERROR;

    numbytes = sprintf(buf, "%s %s HTTP/1.1\r\nHost: %s\r\n", 
        methods[method],
        path, HttpClient::_thishost);


    return  conn->send(buf, numbytes);
    
}

HTTP_RESULT HttpClient::receiveHeaders(HttpConnection * con, HttpData  * handler)
{
    char line[100];
    HttpResponseInfo info;
    char * token;

    int numread;
    if (con && ((numread = con->readLine(line, sizeof(line))) > 0))
    {
        // first token is version
        token = strtok(line, " ");
        if (token)
          snprintf(info.httpversion, sizeof(info.httpversion),
                   "%s", token);
        // second token is number representing
        // response code
        token = strtok(NULL, " ");
        if (token)
          info.responseCode = atol(token);
        // third token is string representation
        // of the response code, we ignore this now
    }

    // We only determine the following
    // when parsing headers:
    // Content-Type
    // Content-Length
    // Transfer-Encoding
    // Connection
    while (con && ((numread = con->readLine(line, sizeof(line))) > 0))
    {
      parseHttpHeaders(line, info);
    }

    return HTTP_OK;
}

bool_t HttpClient::parseHttpHeaders(const char * line, HttpResponseInfo & info)
{
  char * sep_at;

  // Tokenize according to ':' char
  sep_at = strchr(line, ':');
  // First part is key
  if (sep_at != NULL)
  {
    *sep_at = '\0';
    if (stricmp(sep_at, "Transfer-Encoding") == 0)
      // Compare second string if "chunked"
      getTransferEncodingType(sep_at + 1);

    else if (stricmp(sep_at, "Connection") == 0)
    {

    }
    else if (stricmp(sep_at, "Content-Type") == 0)
    {

    }
    else if (stricmp(sep_at, "Content-Length") == 0)
    {

    }
    else
    {
      // Not matched
    }
  }

  return FALSE;
}

HTTP_RESULT HttpClient::parseURL(const char * url, char * scheme, size_t maxschemelen, 
            char * host, size_t maxhostlen, 
            uint16_t * port, 
            char * path, size_t maxpathlen)
{
    char * schemePtr = (char *) url;
    char * hostPtr = (char *) strstr(url, "://");
    if (hostPtr == NULL)
    {
        WARN("Could not find host!\r\n");
        return HTTP_PARSE_ERROR;
    }

    if (maxschemelen < (unsigned int) (hostPtr - schemePtr + 1))
    {
        WARN("Scheme str is too small (%d >= %d)\r\n", maxschemelen, hostPtr - schemePtr + 1);
        return HTTP_PARSE_ERROR;
    }

    memcpy(scheme, schemePtr, hostPtr - schemePtr);
    scheme[hostPtr-schemePtr] = '\0';

    hostPtr += 3; // move to chars after '//'
    size_t hostLen = 0;
    char * portPtr = strchr(hostPtr, ':');
    if (portPtr != NULL)
    {
        hostLen = portPtr - hostPtr;
        portPtr++;
        if (sscanf(portPtr, "%hu", port) != 1)
        {
            WARN("Could not convert/find port numbers!\r\n");
            return HTTP_PARSE_ERROR;
        }
    }
    else
    {
        *port = 80;
    }
    

    char * pathPtr = strchr(hostPtr,'/');
    if (hostLen == 0)
    {
        hostLen = pathPtr - hostPtr;
    }

    if (maxhostlen < hostLen + 1)
    {
        WARN("Host str is too small (%d >= %d)\r\n", maxhostlen, hostLen + 1);
        return HTTP_PARSE_ERROR;
    }

    memcpy(host, hostPtr, hostLen);
    host[hostLen] = '\0';

    size_t pathLen;
    char * fragmentPtr = strchr(hostPtr, '#');
    if (fragmentPtr != NULL)
    {
        pathLen = fragmentPtr - pathPtr;
    }else
    {
        pathLen = strlen(pathPtr);
    }

    if (maxpathlen < pathLen + 1)
    {
        WARN("Path str is too small (%d >= %d)\r\n", maxpathlen, pathLen + 1);
        return HTTP_PARSE_ERROR;
    }
    memcpy(path, pathPtr, pathLen);
    path[pathLen] = '\0';

    return HTTP_OK;

}

