
/*
* @brief:
*/

#if !defined gehua_psm_context_h_
#define gehua_psm_context_h_

#include "tcpserver/termserver.h"
#include "tcpserver/busiserver.h"
#include "businesslogic/businesspool.h"
#include "term_request_process_svr.h"
#include "term_basic_func_svr.h"
#include "business_apply_server.h"
#include "sm_reuqest_process_svr.h"

struct PSMContext 
{
    TermServer              *term_server_;
    BusiServer              *busi_server_;
    BusinessPool            *busi_pool_;
    Logger                  &logger_;

    TermRequestProcessSvr   *term_request_process_svr_;
    TermBasicFuncSvr        *term_basic_func_svr_;
    BusinessApplySvr        *business_apply_svr_;
    SMRequestProcessSvr     *sm_request_process_svr_;

    PSMContext(string const& cfgfile, Logger &logger)
        : term_server_(0)
        , busi_server_(0)
        , busi_pool_(0)
        , logger_(logger)
    {
        int max_receiver_thread = 64;
        int max_sender_thread = 64;
        aio_mgr_ = new AioManager(max_receiver_thread, max_sender_thread);
        aio_mgr_->SetLogger(&logger);
        
        term_server_ = new TermServer("*:7687", 4);
        term_server_->AioTcpServer::SetLogger(&logger);
        term_server_->SetLogger(&logger);

        busi_server_ = new BusiServer("*:7688", 4);
        busi_server_->AioTcpServer::SetLogger(&logger);
        busi_server_->SetLogger(&logger);

        //get local tcp server ip address.
        ip_str_  = term_server_->ip_string();
        ip_addr_ = ip_cast(ip_str_);

        logger_.Info("PSM TerminalServer ip: %s", ip_str_.c_str()); 

        busi_pool_ = new BusinessPool(logger, 64);
        
        term_server_->psm_ctx_ = this;
        busi_server_->psm_ctx_ = this;
        busi_pool_->psm_ctx_   = this;

        term_request_process_svr_   = new TermRequestProcessSvr(this);
        term_basic_func_svr_        = new TermBasicFuncSvr(this);
        business_apply_svr_         = new BusinessApplySvr(this, 10);
        sm_request_process_svr_     = new SMRequestProcessSvr(this);      
    }

    ~PSMContext()
    {
        delete busi_pool_;
        delete term_server_;
        delete busi_server_;
        delete aio_mgr_;
        
        delete term_request_process_svr_;
        delete term_basic_func_svr_;
        delete business_apply_svr_;
        delete sm_request_process_svr_;
    }

    bool Start()
    {
        if (!busi_pool_->Start()) {
            logger_.Error("PSM Business Pool start failure");

            //todo:: cleanup

            return false;
        }
        
        if (!busi_server_->Start()) {
            logger_.Error("PSM Business TCP Server: %s start failure", 
                busi_server_->Addr().c_str());

            //todo:: cleanup

            return false;
        }
        
        if (!term_server_->Start()) {
            logger_.Error("PSM Terminal TCP Server: %s start failure", 
                term_server_->Addr().c_str());

            //todo:: cleanup

            return false;
        }

        aio_mgr_->Start();
        aio_mgr_->Attach(term_server_);
        aio_mgr_->Attach(busi_server_);

        logger_.Info("PSM Business Pool start successful");
        logger_.Info("PSM Business TCP Server: %s start successful", 
                busi_server_->Addr().c_str());
        logger_.Info("PSM Terminal TCP Server: %s start successful", 
                term_server_->Addr().c_str());

        return true;
    }

    void Stop()
    {
        aio_mgr_->Signal();
        aio_mgr_->Detach(term_server_);
        aio_mgr_->Detach(busi_server_);
        aio_mgr_->StopAndWait();
   
        //todo:: delete all connection?
        term_server_->Stop();
        busi_server_->Stop();
        busi_pool_->Stop();
    }

    string ip_string() const { return ip_str_; }
    uint32_t ip_addr() const { return ip_addr_; }
private:
    
    string     ip_str_;
    uint32_t   ip_addr_;
    AioManager *aio_mgr_;

    PSMContext(PSMContext const&);
    PSMContext& operator=(PSMContext const&);
};

#endif // !gehua_psm_context_h_