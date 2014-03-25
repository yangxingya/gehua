#include "business_apply_server.h"
#include "psmcontext.h"

BusinessApplySvr::BusinessApplySvr( PSMContext *psm_context, unsigned int send_thread_count )
{
    psm_context_        = psm_context;
    send_thread_count_  = send_thread_count;

    http_request_processor_ = new HttpRequstProcessor(psm_context_, send_thread_count_);

    char config_name[100];
    for ( unsigned int i = 1; i < 100; ++i )
    {
        _snprintf(config_name, 99, "sminfo%d", i);

        string sm_name      = psm_context_->config_->getOption(config_name, "name");
        string sm_addr      = psm_context_->config_->getOption(config_name, "addr");
        string sm_supported = psm_context_->config_->getOption(config_name, "supported");

        if ( !sm_name.empty() && !sm_addr.empty() && !sm_supported.empty() )
        {
            SMItemInfo *item = new SMItemInfo;
            item->sm_id_                    = i;
            item->sm_name_                  = sm_name;
            item->sm_addr_                  = sm_addr;
            item->suppert_business_list_    = stringtool::to_string_list(sm_supported.c_str(), '|', false);

            sm_item_list_.push_back(item);

            list<string>::iterator it = item->suppert_business_list_.begin();
            for ( ; it != item->suppert_business_list_.end(); it++ )
            {
                sm_suppertsvr_table_[it->c_str()] = item;
            }
        }
        else
        {
            break;
        }
    }
}

BusinessApplySvr::~BusinessApplySvr()
{
    delete http_request_processor_;
    http_request_processor_ = NULL;

    for ( vector<SMItemInfo*>::iterator iter = sm_item_list_.begin(); iter != sm_item_list_.end(); iter++ )
    {
        delete *iter;
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

bool BusinessApplySvr::IsValidServieName( string service_name )
{
    SMSuppertSvrTable::iterator iter = sm_suppertsvr_table_.find(service_name);

    if ( iter != sm_suppertsvr_table_.end() )
    {
        return true;
    }

    if ( IsPHONEControlSvc(service_name.c_str()) )
    {
        return true;
    }

    return false;
}
