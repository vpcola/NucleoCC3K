#include <cstdio>
#include "httpclient.h"
#include "httpjsondata.h"

int main(int argc, char * argv[])
{
    if (argc != 2)
        return 0;

    HttpClient test;
    HttpJsonData jsondatafile;

    test.connect(argv[1], HTTP_GET, &jsondatafile, 1000);

    return 0;
}
