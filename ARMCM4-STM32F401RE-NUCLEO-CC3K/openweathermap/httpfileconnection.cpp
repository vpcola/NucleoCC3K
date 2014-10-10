#include "httpfileconnection.h"
#include "httptypes.h"

HttpFileConnection::HttpFileConnection()
    : _infp(NULL)
{
    DBG("File connection choosen!\n");
}

HttpFileConnection::~HttpFileConnection()
{
    if (_infp) fclose(_infp);
}

HTTP_RESULT HttpFileConnection::open( HttpConnectionParams & param)
{
#ifdef __linux__
    // We need to massage the path to work 
    // correctly under simulated linux env
    char tmpbuff[100];
    sprintf(tmpbuff, ".%s", param.path);
    strcpy(param.path, tmpbuff);
#endif

    DBG("Opening file path [%s]\n", param.path);
    // Open the file pointed to by the param url/path
    _infp = fopen(param.path, "r");
    if (!_infp)
    {
        WARN("Error opening file!\n");
        return HTTP_CONNECT_ERROR;
    }

    _isOpen = true;
    _isConnected = true;

    DBG("File now open!\n");

    return HTTP_OK;
}

HTTP_RESULT HttpFileConnection::send( const char * buffer, size_t bufsiz)
{
    DBG("Sending data to stdout!\n");

    // We send data to stdout
    printf("Sent Data[");
    fwrite((const void *) buffer, 1, bufsiz, stdout);
    printf("]\n");

    return HTTP_OK;
}

HTTP_RESULT HttpFileConnection::recv(char * buffer, size_t & bufsiz)
{
    size_t n;

    DBG("Calling recv for read size %d\n", bufsiz);

    if ((n = fread(buffer, 1, bufsiz, _infp)) != bufsiz)
    {
        if(feof(_infp)) {
            bufsiz = n;
            return HTTP_OK;
        }else
            // Error occured
            return HTTP_RECV_ERROR;
    }
    return HTTP_OK;
}

int HttpFileConnection::recvUntil(char * buffer, size_t bufsiz, const char term)
{
    // Read one char at a time an evaluate
    int n;
    unsigned int numread = 0;

    while(!feof(_infp))
    {
        n = fgetc(_infp);
        if ((char) n == term) break;

        buffer[numread] = n;
        numread ++;

        if (numread >= bufsiz) // prevent overflow
        {
            return HTTP_RECV_BUFF_ERROR;
        }

    };

    if (ferror(_infp)) return HTTP_RECV_ERROR;

    return numread;
}

void HttpFileConnection::consumeLine()
{
    int n = 0;

    while((n = fgetc(_infp)) != EOF)
    {
        if (n == '\n') return;
    };
}

HTTP_RESULT HttpFileConnection::close()
{
    if (_infp)
    {
        DBG("Closing output file\n");
        fclose(_infp);
        _infp = NULL;
    }
    return HTTP_OK;
}

