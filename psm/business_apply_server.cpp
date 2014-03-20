#include "business_apply_server.h"
#include "psmcontext.h"

BusinessApplySvr::BusinessApplySvr( PSMContext *psm_context, unsigned int send_thread_count )
{
    psm_context_        = psm_context;
    send_thread_count_  = send_thread_count;

    http_request_processor_ = new HttpRequstProcessor(psm_context_, send_thread_count_);

    SMItemInfo *item = new SMItemInfo;
    item->sm_id_        = BSGame;
    item->sm_name_      = SERVICE_CloudGame;
    item->sm_addr_      = "127.0.0.1:80";
    item->suppert_business_list_.insert(SERVICE_CloudGame);
    sm_suppertsvr_table_[SERVICE_CloudGame] = item;

    item = new SMItemInfo;
    item->sm_id_        = BSVOD;
    item->sm_name_      = SERVICE_SVOD;
    item->sm_addr_      = "127.0.0.1:80";
    item->suppert_business_list_.insert(SERVICE_SVOD);
    sm_suppertsvr_table_[SERVICE_SVOD] = item;
}

BusinessApplySvr::~BusinessApplySvr()
{
    delete http_request_processor_;
    http_request_processor_ = NULL;

    for ( SMSuppertSvrTable::iterator iter = sm_suppertsvr_table_.begin(); iter != sm_suppertsvr_table_.end(); iter++ )
    {
        delete (iter->second);
    }
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
    return (strcmp(apply_business_name, SERVICE_PhoneControl) == 0);
}

void BusinessApplySvr::AddInitRequestWork(SvcApplyWork *work)
{
    http_request_processor_->AddHttpRequestWork(work);
}

BusinessType BusinessApplySvr::GetBusinessType( string service_name )
{
    if (service_name.compare(SERVICE_CloudGame) == 0)
    {    
        return BSGame;
    }
    if (service_name.compare(SERVICE_SVOD) == 0)
    {    
        return BSVOD;
    }
    if (service_name.compare(SERVICE_PhoneControl) == 0)
    {    
        return BSPhoneControl;
    }    

    return BSPending;
}
