
/*
 * @brief:
 */

#if !defined gehua_psm_context_h_
#define gehua_psm_context_h_

#include "tcpserver/terminalserver.h"
#include "tcpserver/businessserver.h"
#include "businesslogic/businesspool.h"

namespace gehua {

using namespace tcpserver;
using namespace businesslogic;
using namespace sessionmgr;

struct PSMContext 
{
    TerminalServer term_server_;
    BusinessServer busi_server_;
    BusinessPool   busi_pool_;
};

} // namespace gehua

#endif // !gehua_psm_context_h_