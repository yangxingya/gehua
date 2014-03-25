#include <cpplib/logger.h>
#include <cpplib/thread.h>
#include <cpplib/interruptkeycatcher.h>
#include <cpplib/exception/exceptioncatcher.h>
#include "psmcontext.h"
#ifdef _DEBUG
# include <iostream>
#endif // _DEBUG

#ifdef _WIN32
#include <crtdbg.h> 
#endif

static Logger g_logger;

struct CustomCatchSystemException : public ExceptionCatcher
{
    virtual void WriteException( char * const s )
    {
        g_logger.Fatal(s);
    }
    virtual void OnExit()
    {
        g_logger.Flush();
        g_logger.SetBackgroundRunning(false);
        WriteException((char*)"Program exit abnormally!");
    }
};

int main(int argc, char **argv)
{
#ifdef _WIN32
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtDumpMemoryLeaks();
#endif

    // 安装异常捕捉对象
    ExceptionCatcher::Install( new CustomCatchSystemException() );

    // 初始化日志对象
    g_logger.SetLimit(10);
    g_logger.SetModule("psm");
    g_logger.SetPath("./log/psm");
    g_logger.SetLogLevel(LOGLEVEL_Trace);
    g_logger.SetOutputToScreen(true);
    g_logger.SetOutputToFile(true);
    g_logger.SetBackgroundRunning(true);

    PSMContext psmctx("psm.config", g_logger);
    psmctx.Start();

    // 开始循环等待命令输入并执行
    while (!InterruptKeyCatcher::Occurred())
    {
        Thread::Sleep(1000);
    }

    //
#ifdef _DEBUG
    using ::std::cin;
    using ::std::cout;
    using ::std::endl;
    cout << "Press any key(but not enter, space, tab and control key) to exit!!!" << endl;
    int xx;
    cin >> xx;
#endif

    g_logger.Flush();
    g_logger.SetBackgroundRunning(false);
    g_logger.Info("Program exit normally!");
    return 0;
}
