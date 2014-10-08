#include "httpjsondata.h"

HttpJsonData::HttpJsonData()
{
}

HttpJsonData::~HttpJsonData()
{
}

HTTP_RESULT HttpJsonData::sendHeader(HttpConnection * connection)
{
    return HTTP_OK;
}

HTTP_RESULT HttpJsonData::handleData(HttpConnection * connection)
{
    return HTTP_OK;
}
