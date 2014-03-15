

#include "fcgi/fcgiapp.h"
#include "fcgi/fcgi_stdio.h"
#include <stdlib.h>

int main1() 
{
    //
    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {
        Params *param = request.paramsPtr;
    }

    return 0;
}

int main()
{
    int count = 0;
    while (FCGI_Accept() >= 0) {
        //get env is null, fastcgi server env can't
        // transfer to fcgi programm.
        char *srv = getenv("SERVER_NAME");
        if (srv == 0) srv = "";
        printf("Content-type: text/html\r\n"
            "\r\n"
            "<title>FastCGI Hello! (C, fcgi_stdio library)</title>"
            "<h1>FastCGI Hello! (C, fcgi_stdio library)</h1>"
            "Request number %d running on host <i>%s</i>\n",
            ++count, srv);
    }
    return 0;
}