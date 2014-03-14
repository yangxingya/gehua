#include "business_apply_server.h"
#include "psmcontext.h"

BusinessApplySvr::BusinessApplySvr( PSMContext *psm_context, unsigned int send_thread_count )
{
    psm_context_        = psm_context;
    send_thread_count_  = send_thread_count;

    http_request_processor_ = new HttpRequstProcessor(psm_context_, send_thread_count_);
}

BusinessApplySvr::~BusinessApplySvr()
{
    delete http_request_processor_;
    http_request_processor_ = NULL;
}

int BusinessApplySvr::GetSMServiceAddr( string &apply_business_name, string &addr_str )
{
    SMSuppertSvrTable::iterator iter = sm_suppertsvr_table_.find(apply_business_name);

    if ( iter == sm_suppertsvr_table_.end() )
    {
        return -1;
    }

    addr_str = iter->second->sm_addr_;

    return 0;
}

bool BusinessApplySvr::IsPHONEControlSvc( const char *apply_business_name )
{
    return (strcmp(apply_business_name, BUSINESS_PHONE_Control) == 0);
}

void BusinessApplySvr::AddInitRequestWork(SvcApplyWork *work)
{
    http_request_processor_->AddHttpRequestWork(work);
}
