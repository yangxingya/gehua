
#include "fcgi/fcgi_config.h"  
#include <stdlib.h> 
#ifdef _WIN32  
# include <process.h>  
#else  
 extern char **environ;  
#endif  
#include "fcgi/fcgiapp.h" 

#include <string>
#include <vector>
#include "reqentry.h"

using ::std::string;
using ::std::vector;

static size_t getline(FCGX_Stream *in, vector<string> *out)
{
    size_t ret = 0;
    char buf[4096];
    while (FCGX_GetLine(buf, 4096, in)) {
        out->push_back(buf);
        ++ret;
    }

    return ret;
}

int main1()  
{  
    FCGX_Stream *in, *out, *err;  
    FCGX_ParamArray env;  
    int count = 0;  
  
    while (FCGX_Accept(&in, &out, &err, &env) >= 0) {  
        
        //get req header, and context.
        string q_str;
        vector<string> content;
        

        bool keepalive;
        do {

            content.clear();
            q_str = FCGX_GetParam("QUERY_STRING", env);
            getline(in, &content);

            RequestEntry reqety(q_str, content);
            if (!reqety.valid())
                break;

            string rescontent = reqety.MakeResponse();
        
            
            keepalive = reqety.ConnKeepAlive();

        } while (keepalive);

    } /* while */  

    return 0;  
} 

int main() 
{
    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    int count = 0;  
  
    while (FCGX_Accept_r(&request) >= 0) {

        //get req header, and context.
        string q_str;
        vector<string> content;

        bool keepalive = false;
        do {

            content.clear();
            q_str = FCGX_GetParam("QUERY_STRING", request.envp);
            getline(request.in, &content);

            RequestEntry reqety(q_str, content);
            if (!reqety.valid())
                break;

            string rescontent = reqety.MakeResponse();

            keepalive = reqety.ConnKeepAlive();
            request.keepConnection = keepalive; 

            string conn = "";
            if (!keepalive) 
                conn = "Connection: close\r\n";

            FCGX_FPrintF(request.out, 
                "Pragma: no-cache\r\n"			
                "Cache-Control: no-cache\r\n"
                "Content-Type: text/html\r\n"
                "%s"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                conn.c_str(),
                rescontent.length(),
                rescontent.c_str());
            FCGX_FFlush(request.out);
            FCGX_FClose(request.out);
        } while (false);  
    }
    //FCGX_Finish_r(&request);
    return 0;
}