#include "httptcpconnection.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include "cc3000_chibios_api.h"
#include "socket.h"
#include "utility.h"
#endif



HttpTcpConnection::HttpTcpConnection()
    : _sockfd(0), _port(0)
{
   memset(_hostname, 0, sizeof(_hostname));
}

HttpTcpConnection::~HttpTcpConnection()
{
}

HTTP_RESULT HttpTcpConnection::open( HttpConnectionParams & param)
{
    sockaddr_in dest;
#ifdef __linux__
    struct hostent * he;
    struct in_addr **addr_list, *p;
#else
    uint32_t remoteHostIp;
#endif

    _port = param.port;

    // Open a socket
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        WARN("Error connecting to socket\n");
        return HTTP_CONNECT_ERROR;
    }

    memset(&dest, 0, sizeof(dest));
#ifdef __linux__
    if ((he = gethostbyname(param.host)) == NULL)
    {
        WARN("Error resolving host [%s]\n", param.host);
        return HTTP_CONNECT_ERROR;
    }

    addr_list = (struct in_addr **) he->h_addr_list;
#else
    if (gethostbyname(param.host, strlen(param.host), &remoteHostIp) < 0)
    {
        WARN("Failed resolving address [%s] ...\r\n",
             param.host);
        chThdSleepMilliseconds(1000);
    }
#endif

    dest.sin_family = AF_INET;
    dest.sin_port = htons(_port);
#ifdef __linux__
    // Only get the first match
    dest.sin_addr.s_addr = addr_list[0]->s_addr;
#else
    dest.sin_addr.s_addr = htonl(remoteHostIp);
#endif

    // Make the actual connection
    if (::connect(_sockfd, (sockaddr *) &dest, sizeof(dest)) != 0)
    {
        WARN("Error connecting to host address\n");
        return HTTP_CONNECT_ERROR;
    }

    // Successfully open and connected
    _isOpen = TRUE;
    _isConnected = FALSE;

    return HTTP_OK;

}


HTTP_RESULT HttpTcpConnection::send( const char * buffer, size_t bufsiz)
{
    unsigned int totalsent = 0, bytesleft = bufsiz;
    int n;

    // Sanity checks here
    if (!buffer || (bufsiz == 0)) return HTTP_SEND_ERROR;

    while(totalsent < bufsiz)
    {
        n = ::send(_sockfd, buffer + totalsent, bytesleft, 0);
        if (n < 0) break;
        totalsent += n;
        bytesleft -= n;
    }

    if (n < 0)
        return HTTP_SEND_ERROR;
    else
        return HTTP_OK;
}


HTTP_RESULT HttpTcpConnection::recv(char * buffer, size_t & bufsiz)
{
    unsigned int totalrecv = 0, bytesleft = bufsiz;
    int n;

    if (!buffer || (bufsiz == 0)) return HTTP_RECV_ERROR;

    while(totalrecv < bufsiz)
    {
        n = ::recv(_sockfd, buffer + totalrecv, bytesleft, 0);
        if (n < 0) break;
        totalrecv += n;
        bytesleft -= n;
    }

    bufsiz = totalrecv;

    if (n < 0)
        return HTTP_RECV_ERROR;
    else
        return HTTP_OK;
}

int HttpTcpConnection::recvUntil(char * buffer, size_t bufsiz, const char term)
{
    // Read one char at a time an evaluate
    int n;
    unsigned int numread = 0;
    char c;

    while (( n = ::recv(_sockfd, &c, 1, 0)) > 0)
    {
        if ( c == term )
            break;

        buffer[numread] = c;
        numread ++;

        if (numread >= bufsiz) // prevent overflow
        {
            return HTTP_RECV_BUFF_ERROR;
        }
    }

    if (n < 0)
        return HTTP_RECV_ERROR;
    else
        return numread;

}

void HttpTcpConnection::consumeLine()
{
    char c;
    int n = 0;

    do {
        if((n = ::recv(_sockfd, &c, 1, 0)) <= 0) break;
    }while(c != '\n');
}

HTTP_RESULT HttpTcpConnection::close()
{
    // ::close(_sockfd);

    return HTTP_OK;
}


