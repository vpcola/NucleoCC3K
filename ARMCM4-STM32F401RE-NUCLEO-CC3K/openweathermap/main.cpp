#include <cstdio>
#include "httpclient.h"
#include "httpjsondata.h"

int main(int argc, char * argv[])
{
    if (argc != 2)
        return 0;

    HttpClient test;
    HttpJsonData jsondatahandler;

    test.connect((const char *) argv[1], HTTP_GET, &jsondatahandler);

    return 0;
}
