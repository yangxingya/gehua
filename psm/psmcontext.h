
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
    Logger       &logger_;

    PSMContext(string const& cfgfile, Logger &logger)
        : term_server_("*:10001", 1)
        , busi_server_("*:10002", 1)
        , busi_pool_(logger, 4)
        , logger_(logger)
    {
        term_server_.psm_ctx_ = this;
        busi_server_.psm_ctx_ = this;
        busi_pool_.psm_ctx_ = this;

        term_server_.SetLogger(&logger);
        busi_server_.SetLogger(&logger);

        //get local tcp server ip address.
        ip_str_  = term_server_.ip_string();
        ip_addr_ = ip_cast(ip_str_);

        logger_.Info("PSM TerminalServer ip: %s", ip_str_.c_str()); 
    }

    bool Start()
    {
        if (!busi_pool_.Start()) {
            logger_.Error("PSM Business Pool start failure");

            //todo:: cleanup

            return false;
        }
        
        if (!busi_server_.Start()) {
            logger_.Error("PSM Business TCP Server: %s start failure", 
                busi_server_.Addr().c_str());

            //todo:: cleanup

            return false;
        }
        
        if (!term_server_.Start()) {
            logger_.Error("PSM Terminal TCP Server: %s start failure", 
                term_server_.Addr().c_str());

            //todo:: cleanup

            return false;
        }

        logger_.Info("PSM Business Pool start successful");
        logger_.Info("PSM Business TCP Server: %s start successful", 
                busi_server_.Addr().c_str());
        logger_.Info("PSM Terminal TCP Server: %s start successful", 
                term_server_.Addr().c_str());

        return true;
    }

    void Stop()
    {
        //todo:: delete all connection?
        term_server_.Stop();
        busi_server_.Stop();
        busi_pool_.Stop();
    }

    string ip_string() const { return ip_str_; }
    uint32_t ip_addr() const { return ip_addr_; }
private:
    
    string   ip_str_;
    uint32_t ip_addr_;

    PSMContext(PSMContext const&);
    PSMContext& operator=(PSMContext const&);
};

#endif // !gehua_psm_context_h_