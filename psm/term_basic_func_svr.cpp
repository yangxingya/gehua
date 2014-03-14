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


void TermBasicFuncSvr::AddKeyTransmitWork( TermConnection *conn, PtKeyMappingRequest *pkg )
{
    TRequestWork_KeyMapping *work = new TRequestWork_KeyMapping;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_KeyMapping::KeyMapping_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TRequestWork_KeyMapping::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermBasicFuncSvr::AddSvcSwitchNotifyWork( TermConnection *conn, PtSvcSwitchRequest *pkg )
{
    TNotifyWork_SvcSwitch *work = new TNotifyWork_SvcSwitch;

    work->pkg_              = pkg;
    work->run_step_         = TNotifyWork_SvcSwitch::SvcSwitch_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TNotifyWork_SvcSwitch::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermBasicFuncSvr::AddSvcSwitchNotifyWork( TermConnection *conn, PtSvcSwitchResponse *pkg )
{
    //TODO:
    delete pkg;
    return;
}

void TermBasicFuncSvr::AddStatusPChangeNotifyWork( TermConnection *conn, PtStatusNotifyRequest *pkg )
{
    TNotifyWork_StatusNotify *work = new TNotifyWork_StatusNotify;

    work->pkg_              = pkg;
    work->run_step_         = TNotifyWork_StatusNotify::StatusNotify_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TNotifyWork_StatusNotify::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermBasicFuncSvr::AddStatusPChangeNotifyWork( TermConnection *conn, PtStatusNotifyResponse *pkg )
{
    //TODO:
    delete pkg;
    return;
}

void TermBasicFuncSvr::NotifyAllTerminalStatusPChanged( CASession *ca_session )
{
    PtStatusNotifyRequest notify_request;
    map<uint64_t, TermSession*>::iterator iter = ca_session->terminal_session_map_.begin();
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        notify_request.terminal_list_desc_.push_back(iter->second->terminal_info_desc);
    }

    // add notify work.
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        PtStatusNotifyRequest *tmp = new PtStatusNotifyRequest;
        tmp->terminal_list_desc_ = notify_request.terminal_list_desc_;        

        AddStatusPChangeNotifyWork(iter->second->term_conn, tmp);
    }
}

/////////////////////////////////Work_Func////////////////////////////////////////////

void TRequestWork_KeyMapping::Func_Begin( Work *work )
{
    TRequestWork_KeyMapping *keymapping_work  = (TRequestWork_KeyMapping*)work;
    PSMContext *psm_context                   = (PSMContext*)work->user_ptr_;

    keymapping_work->run_step_ = TRequestWork_KeyMapping::KepMapping_Transpond;

    //get target terminal info.
    map<uint64_t, TermSession*>::iterator iter = keymapping_work->session_info_->ca_session->terminal_session_map_.find(keymapping_work->pkg_->key_mapping_desc_.dest_session_id_);
    if ( iter != keymapping_work->session_info_->ca_session->terminal_session_map_.end() )
    {
        //transpond to target terminal.

        PtKeyMappingResponse keymapping_response;
        keymapping_response.key_mapping_desc_ = keymapping_work->pkg_->key_mapping_desc_;

        ByteStream responed_pkg = keymapping_response.Serialize();
        responed_pkg.Add(keymapping_response.key_mapping_desc_.Serialize());
        iter->second->term_conn->Write((unsigned char*)responed_pkg.GetBuffer(), responed_pkg.GetWritePtr());
    }

    TRequestWork_StatusQuery::Func_End(work);
}

void TRequestWork_KeyMapping::Func_End( Work *work )
{
    TRequestWork_KeyMapping *keymapping_work  = (TRequestWork_KeyMapping*)work;
    PSMContext *psm_context                   = (PSMContext*)work->user_ptr_;

    keymapping_work->run_step_ = TRequestWork_KeyMapping::KeyMapping_End;

    delete keymapping_work;
}

//////////////////////////////////////////////////////////////////////////

void TNotifyWork_SvcSwitch::Func_Begin( Work *work )
{
    TNotifyWork_SvcSwitch *svcswitch_work  = (TNotifyWork_SvcSwitch*)work;
    PSMContext *psm_context                = (PSMContext*)work->user_ptr_;

    // send notify to terminal.
    svcswitch_work->run_step_ = TNotifyWork_SvcSwitch::SvcSwitch_SendNotify;

    ByteStream bs = svcswitch_work->pkg_->Serialize();
    bs.Add(svcswitch_work->pkg_->svc_cross_url_desc_.Serialize());
    bs.Add(svcswitch_work->pkg_->svc_self_url_desc_.Serialize());
    bs.Add(svcswitch_work->pkg_->keymap_indicate_desc_.Serialize());
    bs.Add(svcswitch_work->pkg_->test_data_desc_.Serialize());
    if ( svcswitch_work->session_info_->term_conn->Write((unsigned char*)bs.GetBuffer(), bs.Size()) )
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

    // send notify to terminal.
    statusnotify_work->run_step_ = TNotifyWork_StatusNotify::StatusNotify_SendNotify;

    ByteStream bs = statusnotify_work->pkg_->Serialize();
    for ( list<PT_TerminalInfoDescriptor>::iterator iter = statusnotify_work->pkg_->terminal_list_desc_.begin();
        iter != statusnotify_work->pkg_->terminal_list_desc_.end(); iter++ )
    {
        bs.Add(iter->Serialize());
    }
    if ( statusnotify_work->session_info_->term_conn->Write((unsigned char*)bs.GetBuffer(), bs.Size()) )
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

    // if recvice responed failed.
    if ( !statusnotify_work->recv_responed_sucess_ )
    {

    }

    statusnotify_work->run_step_ = TNotifyWork_StatusNotify::StatusNotify_End;

    delete statusnotify_work;
}
