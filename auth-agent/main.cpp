/*
 * threaded.c -- A simple multi-threaded FastCGI application.
 */

#ifndef lint
static const char rcsid[] = "$Id: threaded.c,v 1.9 2001/11/20 03:23:21 robs Exp $";
#endif /* not lint */

#include <fcgi_config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcgiapp.h>
#if defined _WIN32
#include <process.h>
#else
#include <pthread.h>
#endif
#include <string>
#include <vector>
#include "reqentry.h"
#include <cpplib/mutex.h>
#include <cpplib/logger.h>
#include <cpplib/interruptkeycatcher.h>
#include <cpplib/thread.h>
#include "../common/config/config.h"

using ::std::string;
using ::std::vector;

static Logger g_logger;

#define THREAD_COUNT 20

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

#if defined _WIN32
#define APICALL __stdcall
#else
#define APICALL
#endif 

static unsigned APICALL accept_proc(void *para)
{
    int rc;

    string psmaddr = *(string*)para;

    //FCGX_Stream     *in  = 0;
    //FCGX_Stream     *out = 0;
    //FCGX_Stream     *err = 0;
    //FCGX_ParamArray envp = 0;

    FCGX_Request request;

    FCGX_InitRequest(&request, 0, 0);

    for (;;) {
        static Mutex accept_mutex;

        // lock accept.
        {
            MutexLock lock(accept_mutex);
            /* Some platforms require accept() serialization, some don't.. */
            rc = FCGX_Accept_r(&request);
            //rc = FCGX_Accept(&in, &out, &err, &envp);
        }

        // error.
        if (rc < 0) {
            g_logger.Warn("接收出错，退出");
            break;
        }

        string q_str;
        string methods;
        vector<string> content;

        bool keepalive = false;
        do {
            content.clear();
            q_str = FCGX_GetParam("QUERY_STRING", request.envp);
            methods = FCGX_GetParam("REQUEST_METHOD", request.envp);

            getline(request.in, &content);

            RequestEntry reqety(g_logger, q_str, content, psmaddr);
            if (!reqety.valid())
                break;

            string rescontent = reqety.MakeResponse();

            keepalive = reqety.ConnKeepAlive();
            request.keepConnection = false; 
            if (methods == "GET")
                request.keepConnection = 0;

            string conn = "";
            if (!keepalive) 
                conn = "Connection: close\r\n";

            int writed = FCGX_FPrintF(request.out, 
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
            g_logger.Info("FastCGI Printf return code: %d context: \n\t%s!", writed, rescontent.c_str());
            //FCGX_FFlush(request.out);
            //FCGX_FClose(request.out);
        } while (false); 

        FCGX_Finish_r(&request);
    }

    return 0;
}


static void* pthread_accept_proc(void *argv)
{
    accept_proc(argv);
    return 0;
}

int main(void)
{ 
    // 初始化日志对象
    g_logger.SetLimit(10);
    g_logger.SetModule("auth-agent");
    g_logger.SetPath("./log/auth-agent");
#if _DEBUG
    g_logger.SetLogLevel(LOGLEVEL_Trace);
#else // _DEBUG
    g_logger.SetLogLevel(LOGLEVEL_Info);
#endif // ! _DEBUG
    g_logger.SetOutputToScreen(true);
    g_logger.SetOutputToFile(true);
    g_logger.SetBackgroundRunning(true);

    Config cfg(g_logger, "psm.config");

    FCGX_Init();

    g_logger.Info("PSM-HTTP Server Started!");

    string str_auth_thr_cnt = cfg.getOption("perf", "auth_thread_cnt");
    string term_srv_addr    = cfg.getOption("alladdr", "login_addr");

    int auth_thr_cnt = THREAD_COUNT;
    g_logger.Info("[PSM-Http Server][option] Accept thread count: %s", str_auth_thr_cnt.c_str());
    g_logger.Info("[PSM-Http Server][option] Sitch to PSM Term Server Address: %s", term_srv_addr.c_str());
    if (!str_auth_thr_cnt.empty()) {
        auth_thr_cnt = atoi(str_auth_thr_cnt.c_str());
        if (auth_thr_cnt < 0 || auth_thr_cnt > 20000)
            auth_thr_cnt = THREAD_COUNT;
    }
    g_logger.Info("[PSM-Http Server] Accept thread real count: %d", auth_thr_cnt);

    for (int i = 1; i < THREAD_COUNT; i++) {
#if defined _WIN32
        _beginthreadex(0, 0, accept_proc, &term_srv_addr, 0, 0);
#else
        pthread_t pt;
        pthread_create(&pt, 0, pthread_accept_proc, &term_srv_addr);
#endif
    }

    // 开始循环等待命令输入并执行
    while (!InterruptKeyCatcher::Occurred())
    {
        Thread::Sleep(5000);
    }

    return 0;
}

