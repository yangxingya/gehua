#include "term_basic_func_svr.h"
#include "psmcontext.h"
#include "sessionmgr/termsession.h"
#include "sessionmgr/casession.h"


TermBasicFuncSvr::TermBasicFuncSvr( PSMContext *psm_context )
{
    psm_context_    = psm_context;
}


TermBasicFuncSvr::~TermBasicFuncSvr()
{

}


void TermBasicFuncSvr::AddKeyTransmitWork( weak_ptr<TermSession> ts, PtKeyMappingRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[键值映射请求]******终端会话不存在******");
        return;
    }

    TRequestWork_KeyMapping *work = new TRequestWork_KeyMapping;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_KeyMapping::KeyMapping_Begin;
    work->session_info_     = sp_ts;
    work->work_func_        = TRequestWork_KeyMapping::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}

void TermBasicFuncSvr::AddSvcSwitchNotifyWork( weak_ptr<TermSession> ts, PtSvcSwitchRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[业务切换通知]******终端会话不存在******");
        return;
    }

    TNotifyWork_SvcSwitch *work = new TNotifyWork_SvcSwitch;

    work->pkg_              = pkg;
    work->run_step_         = TNotifyWork_SvcSwitch::SvcSwitch_Begin;
    work->session_info_     = sp_ts;
    work->work_func_        = TNotifyWork_SvcSwitch::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}

void TermBasicFuncSvr::AddSvcSwitchNotifyWork( weak_ptr<TermSession> ts, PtSvcSwitchResponse *pkg )
{
    //TODO: 暂时不关注应答
    delete pkg;
    return;
}

void TermBasicFuncSvr::AddStatusPChangeNotifyWork( weak_ptr<TermSession> ts, PtStatusNotifyRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[状态参数变化通知]******终端会话不存在******");
        return;
    }

    TNotifyWork_StatusNotify *work = new TNotifyWork_StatusNotify;

    work->pkg_              = pkg;
    work->run_step_         = TNotifyWork_StatusNotify::StatusNotify_Begin;
    work->session_info_     = sp_ts;
    work->work_func_        = TNotifyWork_StatusNotify::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}

void TermBasicFuncSvr::AddStatusPChangeNotifyWork( weak_ptr<TermSession> ts, PtStatusNotifyResponse *pkg )
{
    //TODO: 暂时不关注应答
    delete pkg;
    return;
}

void TermBasicFuncSvr::NotifyAllTerminalStatusPChanged( CASession *ca_session, uint64_t ignore_session_id )
{
    PtStatusNotifyRequest notify_request;
    map<uint64_t, shared_ptr<TermSession> >::iterator iter = ca_session->terminal_session_map_.begin();
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        notify_request.terminal_list_desc_.push_back(iter->second->terminal_info_desc_);
    }

    // add notify work.
    for ( iter = ca_session->terminal_session_map_.begin(); iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        if ( ignore_session_id == iter->second->Id() )
        {
            continue;
        }

        PtStatusNotifyRequest *tmp = new PtStatusNotifyRequest;
        tmp->terminal_list_desc_ = notify_request.terminal_list_desc_;

        AddStatusPChangeNotifyWork(iter->second, tmp);
    }
}

/////////////////////////////////Work_Func////////////////////////////////////////////

void TRequestWork_KeyMapping::Func_Begin( Work *work )
{
    TRequestWork_KeyMapping *keymapping_work  = (TRequestWork_KeyMapping*)work;
    PSMContext *psm_context                   = (PSMContext*)work->user_ptr_;

    shared_ptr<TermSession> kmap_ts(keymapping_work->session_info_.lock());
    if (!kmap_ts) {
        psm_context->logger_.Warn("[键值映射请求]******终端会话不存在******");
        return;
    }

    keymapping_work->run_step_ = TRequestWork_KeyMapping::KepMapping_Transpond;

    //get target terminal info.
    map<uint64_t, shared_ptr<TermSession> >::iterator iter = kmap_ts->ca_session_->terminal_session_map_.find(keymapping_work->pkg_->key_mapping_desc_.dest_session_id_);
    if ( iter != kmap_ts->ca_session_->terminal_session_map_.end() )
    {
        //transpond to target terminal.
  
        PtKeyMappingResponse keymapping_response;
        keymapping_response.Add(keymapping_work->pkg_->key_mapping_desc_);

        ByteStream responed_pkg = keymapping_response.Serialize();

        psm_context->logger_.Trace("[键值映射请求][CAID=" SFMT64U "][SID=" SFMT64U "] 向指定终端[SID=" SFMT64U "]转发键值映射请求。 长度：%d  内容：\n%s", 
                                    kmap_ts->CAId(), kmap_ts->Id(),
                                    iter->second->Id(),
                                    responed_pkg.Size(),
                                    stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());


        iter->second->term_conn_->Write(responed_pkg.GetBuffer(), responed_pkg.Size());
    }

    TRequestWork_StatusQuery::Func_End(work);
}

void TRequestWork_KeyMapping::Func_End( Work *work )
{
    TRequestWork_KeyMapping *keymapping_work  = (TRequestWork_KeyMapping*)work;
    PSMContext *psm_context                   = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    keymapping_work->run_step_ = TRequestWork_KeyMapping::KeyMapping_End;

    delete keymapping_work;
}

//////////////////////////////////////////////////////////////////////////

void TNotifyWork_SvcSwitch::Func_Begin( Work *work )
{
    TNotifyWork_SvcSwitch *svcswitch_work  = (TNotifyWork_SvcSwitch*)work;
    PSMContext *psm_context                = (PSMContext*)work->user_ptr_;

    shared_ptr<TermSession> session_info(svcswitch_work->session_info_.lock());
    if (!session_info) {
        psm_context->logger_.Warn("[业务切换通知]******终端会话不存在******");
        return;
    }

    if ( psm_context ) {//??????
    }

    // send notify to terminal.
    svcswitch_work->run_step_ = TNotifyWork_SvcSwitch::SvcSwitch_SendNotify;   

    if ( svcswitch_work->pkg_->svc_url_desc_.valid_ )           svcswitch_work->pkg_->Add(svcswitch_work->pkg_->svc_url_desc_);
    if ( svcswitch_work->pkg_->keymap_indicate_desc_.valid_ )   svcswitch_work->pkg_->Add(svcswitch_work->pkg_->keymap_indicate_desc_);

    ByteStream bs = svcswitch_work->pkg_->Serialize();

    psm_context->logger_.Trace("[业务切换通知][CAID=" SFMT64U "][SID=" SFMT64U "] 向终端发送业务切换通知。 长度：%d  内容：\n%s", 
                                session_info->CAId(), session_info->Id(),
                                bs.Size(),
                                stringtool::to_hex_string((const char*)bs.GetBuffer(), bs.Size()).c_str());

    if ( session_info->term_conn_->Write((unsigned char*)bs.GetBuffer(), bs.Size()) )
    {
        // add responed process work.
//         svcswitch_work->work_func_   = TNotifyWork_StatusNotify::Func_ProcessResponed; 
//         svcswitch_work->delay_time_  = 5000;
//         psm_context->busi_pool_->AddDelayedWork(work, svcswitch_work->session_info_->CAId());
//         return;
    }

    svcswitch_work->recv_responed_sucess_ = false;
    TNotifyWork_SvcSwitch::Func_End(work);
}

void TNotifyWork_SvcSwitch::Func_End( Work *work )
{
    TNotifyWork_SvcSwitch *svcswitch_work  = (TNotifyWork_SvcSwitch*)work;
    PSMContext *psm_context                = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    // if recvice responed failed.
    if ( !svcswitch_work->recv_responed_sucess_ )
    {

    }

    svcswitch_work->run_step_ = TNotifyWork_SvcSwitch::SvcSwitch_End;

    delete svcswitch_work;
}

//////////////////////////////////////////////////////////////////////////

void TNotifyWork_StatusNotify::Func_Begin( Work *work )
{
    TNotifyWork_StatusNotify *statusnotify_work  = (TNotifyWork_StatusNotify*)work;
    PSMContext *psm_context                      = (PSMContext*)work->user_ptr_;

    shared_ptr<TermSession> session_info(statusnotify_work->session_info_.lock());
    if (!session_info) {
        psm_context->logger_.Warn("[状态变更通知]******终端会话不存在******");
        return;
    }

    if ( psm_context ) {//??????
    }

    // send notify to terminal.
    statusnotify_work->run_step_ = TNotifyWork_StatusNotify::StatusNotify_SendNotify;

    for ( list<PT_TerminalInfoDescriptor>::iterator iter = statusnotify_work->pkg_->terminal_list_desc_.begin();
        iter != statusnotify_work->pkg_->terminal_list_desc_.end(); iter++ )
    {
        statusnotify_work->pkg_->Add(*iter);
    }

    ByteStream bs = statusnotify_work->pkg_->Serialize();

    psm_context->logger_.Trace("[状态变更通知][CAID=" SFMT64U "][SID=" SFMT64U "] 向终端发送状态变更通知。 长度：%d  内容：\n%s", 
                                session_info->CAId(), session_info->Id(),
                                bs.Size(),
                                stringtool::to_hex_string((const char*)bs.GetBuffer(), bs.Size()).c_str());

    if ( session_info->term_conn_->Write((unsigned char*)bs.GetBuffer(), bs.Size()) )
    {
        // and responed process work.
        //         statusnotify_work->work_func_   = TNotifyWork_StatusNotify::Func_ProcessResponed; 
        //         statusnotify_work->delay_time_  = 5000;
        //         psm_context->busi_pool_->AddDelayedWork(work, statusnotify_work->session_info_->CAId());
        //         return;
    }

    statusnotify_work->recv_responed_sucess_ = false;
    TNotifyWork_StatusNotify::Func_End(work);
}

void TNotifyWork_StatusNotify::Func_End( Work *work )
{
    TNotifyWork_StatusNotify *statusnotify_work  = (TNotifyWork_StatusNotify*)work;
    PSMContext *psm_context                      = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    // if recvice responed failed.
    if ( !statusnotify_work->recv_responed_sucess_ )
    {
    }

    statusnotify_work->run_step_ = TNotifyWork_StatusNotify::StatusNotify_End;

    delete statusnotify_work;
}
