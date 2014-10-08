#include "httptcpconnection.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
    uint32_t remoteHostIp;
    sockaddr_in dest;

    _port = param.port;

    // Open a socket
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        WARN("Error connecting to socket\n");
        return HTTP_CONNECT_ERROR;
    }

    memset(&dest, 0, sizeof(dest));
    if (gethostbyname(param.host, strlen(param.host), &remoteHostIp) < 0)
    {
        ccprint("Failed resolving address [%s] ...\r\n",
             param.host);
        chThdSleepMilliseconds(1000);
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(_port);
    dest.sin_addr.s_addr = htonl(remoteHostIp);

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

HTTP_RESULT HttpTcpConnection::recv(char * buffer, size_t bufsiz, bool chunked)
{
    // If the transfer encoding is chunked, we do a chunked
    // read
    if (chunked)
        return chunkedRecv(buffer, bufsiz);
    else
        return rawRecv(buffer, bufsiz);


}

HTTP_RESULT HttpTcpConnection::chunkedRecv(char * buffer, size_t bufsiz)
{
    HTTP_RESULT res = HTTP_OK;
    // Read the chunked header
    // <size in hex> <chunk ext>\r\n
    //<data... upto size>\r\n
    // 0\r\n
    // \r\n
    int totalrecv = 0, n;

    do {

        n = readChunkHeader();
        if (n < 0)
            return HTTP_RECV_ERROR;

        if (n == 0) // terminate chunk
        {
            // Read the training \r\n
            consumeLine();
            return res;
        }

        // terminate if we don't have enough
        // buffer to hold the data
        if ((totalrecv + n) > bufsiz)
            return HTTP_RECV_BUFF_ERROR;

        res = rawRecv(buffer + totalrecv, (size_t) n);
        if (res != HTTP_OK)
            return res;

        totalrecv += n;

    }while(totalrecv < bufsiz);

    return res;
}

// Reads untill the end of the line, returns a
// chunk header size if it sees it.
int HttpTcpConnection::readChunkHeader()
{
    char line[100];
    char * start = line;
    char * end = NULL;

    // Reads untill the end of the line,
    // resulting line buffer will be null
    // terminated without including new lines
    // \r\n
    if (readLine(line, sizeof(line)) > 0)
    {
        // Trim string, cut at first space
        end = strchr(line, ' ');
        if (end != NULL)
            line[end - start] = '\0'; // make null

        // convert the resulting hex string
        // to a number
        return strtol(line, 0, 16);
    }

    return HTTP_RECV_CHNK_ERROR;
}

int HttpTcpConnection::readLine(char * buffer, size_t bufsiz)
{
    int numread = recvUntil(buffer, bufsiz, '\n');

    // Just remove the \r
    if (numread > 0)
    {
        numread --;
        if (buffer[numread] == '\r')
        {
            // make it into a zero string
            buffer[numread] = '\0';

            return numread;
        }
    }

    return numread;

}



HTTP_RESULT HttpTcpConnection::rawRecv(char * buffer, size_t bufsiz)
{
    int totalrecv = 0, bytesleft = bufsiz, n;

    if (!buffer || (bufsiz == 0)) return HTTP_RECV_ERROR;

    while(totalrecv < bufsiz)
    {
        n = ::recv(_sockfd, buffer + totalrecv, bytesleft, 0);
        if (n < 0) break;
        totalrecv += n;
        bytesleft -= n;
    }

    if (n < 0)
        return HTTP_RECV_ERROR;
    else
        return HTTP_OK;
}

int HttpTcpConnection::recvUntil(char * buffer, size_t bufsiz, const char term)
{
    // Read one char at a time an evaluate
    int n, numread = 0;
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


