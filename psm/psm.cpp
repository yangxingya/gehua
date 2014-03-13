#include <cpplib/logger.h>
#include "psmcontext.h"
#ifdef _DEBUG
# include <iostream>
#endif // _DEBUG

static Logger g_logger;

int main(int argc, char **argv)
{
    PSMContext psmctx("psm.config", g_logger);
    psmctx.Start();

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
