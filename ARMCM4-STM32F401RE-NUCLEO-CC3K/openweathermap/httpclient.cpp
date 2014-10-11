#include <string.h>
#include "httpclient.h"
#include "ch.h"
#include "chprintf.h"
#include "utility.h"


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


HTTP_RESULT HttpClient::connect(const char * url, HTTP_METHOD method, HttpData * handler)
{

    char scheme[8];
    HttpConnectionParams params;
    HttpResponseInfo info;

    HTTP_RESULT res = parseURL(url, scheme, sizeof(scheme), params.host, sizeof(params.host), &(params.port), params.path, sizeof(params.path));
    if (res != HTTP_OK) 
        return res;

    // print the result
    DBG("scheme [%s]\n", scheme);
    DBG("host [%s]\n", params.host);
    DBG("port [%d]\n", params.port);
    DBG("path [%s]\n", params.path);

        // Open the connection
        res = _connection.open(params);
        if ( res != HTTP_OK)
            goto res_exit;

        // Send default headers to http server
        res = sendHeaders(&_connection, params.path, method);
        if (res != HTTP_OK)
            goto res_exit;

        // Send any specific headers that the data
        // handler might have
        if (handler)
        {
            // Send specific headers used by the data handlers
            res = handler->sendHeader(&_connection);
            if (res != HTTP_OK)
                goto res_exit;
        }

        // Send \r\n to terminate header sending
        res = _connection.send_str((const char *) "\r\n");
        if (res != HTTP_OK)
            goto res_exit;

        DBG("Sending header ends ...\n");
        DBG("Waiting for server response ...\n");

        // Now read the response header
        res = receiveHeaders(&_connection, info);
        if (res != HTTP_OK)
            goto res_exit;


        // Finally handle the response data
        if (handler && handler->handleServerResponse(info))
        {
            // Handle server response
            res = handler->handleData(&_connection, info);
        }

res_exit:
        // Finally close the socket
    _connection.close();

    return res;
}

HTTP_RESULT HttpClient::sendHeaders(HttpTcpConnection * conn, const char * path, HTTP_METHOD method)
{
    int numbytes;
    char buf[CHUNK_SIZE];

    if (!conn) return HTTP_CONNECT_ERROR;

    numbytes = chsnprintf(buf, sizeof(buf), "%s %s HTTP/1.1\r\nHost: %s\r\n",
        methods[method],
        path, HttpClient::_thishost);


    return  conn->send_data(buf, numbytes);
}

HTTP_RESULT HttpClient::receiveHeaders(HttpTcpConnection * con, HttpResponseInfo & info)
{

    char line[100];
    char * token;

    int numread;
    if (con && ((numread = con->readLine(line, sizeof(line))) > 0))
    {
        // first token is version
        token = strtok(line, " ");
        if (token)
          strcpy(info.httpversion, token);
        // second token is number representing
        // response code
        token = strtok(NULL, " ");
        if (token)
          info.responseCode = atol(token);
        // third token is string representation
        // of the response code, we ignore this now
        token = strtok(NULL, " ");
        if (token)
            strcpy(info.responseString, token);

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

    info.debug();

    return HTTP_OK;
}

bool HttpClient::parseHttpHeaders(char * line, HttpResponseInfo & info)
{

  char * sep_at;
  char * value;

  // Tokenize according to ':' char
  sep_at = strchr(line, ':');
  // First part is key
  if (sep_at != NULL)
  {
    *sep_at = '\0';
    value = strtrim(sep_at + 1);

    // DBG("Key [%s] Value [%s]\n", line, value);

    if (strcmp(line, "Transfer-Encoding") == 0)
    {
      // Compare second string if "chunked"
      info.transferEncoding = getTEType(value);
    }
    else if (strcmp(line, "Connection") == 0)
    {
      info.termType = getTRMType(value);
    }
    else if (strcmp(line, "Content-Type") == 0)
    {
      strcpy(info.contentType, value);
    }
    else if (strcmp(line, "Content-Length") == 0)
    {
      info.contentLength = atoi(value);
    }
    else
    {
      // Not matched
      WARN("Header fields skipped!\n");
    }
  }

  return true;
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
        *port = (uint16_t) atoi(portPtr);
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

