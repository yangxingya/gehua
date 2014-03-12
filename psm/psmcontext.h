
/*
 * @brief:
 */

#if !defined gehua_psm_context_h_
#define gehua_psm_context_h_

#include "tcpserver/termserver.h"
#include "tcpserver/busiserver.h"
#include "businesslogic/businesspool.h"

struct PSMContext 
{
    TermServer   term_server_;
    BusiServer   busi_server_;
    BusinessPool busi_pool_;

    PSMContext(string const& cfgfile, Logger &logger)
      : term_server_("127.0.0.1:10001", 1)
      , busi_server_("127.0.0.1:10002", 1)
      , busi_pool_(logger, 4)
    {
      
    }

    bool Start()
    {
      return false;
    }

private:
    PSMContext(PSMContext const&);
    PSMContext& operator=(PSMContext const&);
};

#endif // !gehua_psm_context_h_