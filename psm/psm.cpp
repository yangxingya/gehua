#include <cpplib/logger.h>
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
        WriteException("�����쳣�˳�");
    }
};

int main(int argc, char **argv)
{
#ifdef _WIN32
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtDumpMemoryLeaks();
#endif

    // ��װ�쳣��׽����
    ExceptionCatcher::Install( new CustomCatchSystemException() );

    // ��ʼ����־����
    g_logger.SetLimit(10);
    g_logger.SetModule("psm");
    g_logger.SetPath("./log/psm");
    g_logger.SetLogLevel(LOGLEVEL_Trace);
    g_logger.SetOutputToScreen(true);
    g_logger.SetOutputToFile(true);
    g_logger.SetBackgroundRunning(true);

    PSMContext psmctx("psm.config", g_logger);
    psmctx.Start();

    // ��ʼѭ���ȴ��������벢ִ��
    while (!InterruptKeyCatcher::Occurred())
    {
        Sleep(5000);
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
    return 0;
}
