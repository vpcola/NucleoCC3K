#include "httpconnection.h"
#include <stdlib.h>
#include <string.h>

HTTP_RESULT HttpConnection::chunkedRecv(char * buffer, size_t & bufsiz)
{
    HTTP_RESULT res = HTTP_OK;
    // Read the chunked header
    // <size in hex> <chunk ext>\r\n
    //<data... upto size>\r\n
    // 0\r\n
    // \r\n
    unsigned int totalrecv = 0;
    int n;

    do {

        n = readChunkHeader();
        if (n < 0)
            return HTTP_RECV_ERROR;

        if (n == 0) // terminate chunk
        {
            // Read the training \r\n
            bufsiz = totalrecv;

            consumeLine();
            return res;
        }

        // terminate if we don't have enough
        // buffer to hold the data
        if ((totalrecv + n) > bufsiz)
        {
            bufsiz = totalrecv;
            return HTTP_RECV_BUFF_ERROR;
        }

        res = recv(buffer + totalrecv, (size_t &) n);
        if (res != HTTP_OK)
        {
            bufsiz = totalrecv + n;
            return res;
        }

        totalrecv += n;

    }while(totalrecv < bufsiz);

    return res;
}

int HttpConnection::readLine(char * buffer, size_t bufsiz)
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

// Reads untill the end of the line, returns a
// chunk header size if it sees it.
int HttpConnection::readChunkHeader()
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


