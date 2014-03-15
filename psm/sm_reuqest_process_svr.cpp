#include "sm_reuqest_process_svr.h"
#include "cpplib/stringtool.h"
#include "psmcontext.h"
#include "sessionmgr/termsession.h"
#include "sessionmgr/casession.h"

SMRequestProcessSvr::SMRequestProcessSvr( PSMContext* psm_context )
{
    psm_context_ = psm_context;
}

SMRequestProcessSvr::~SMRequestProcessSvr()
{

}

void SMRequestProcessSvr::AddTermSyncWork( BusiConnection *conn, PbTermSyncRequest *pkg )
{
    UpdateTermStatus(pkg->term_status_desc_list_);

    //send responed.
    PbTermSyncResponse termsync_response;
    termsync_response.sequence_no_desc_ = pkg->sequence_no_desc_;
    termsync_response.test_data_desc_   = pkg->test_data_desc_;

    ByteStream bs(termsync_response.Serialize());
    bs.Add(termsync_response.sequence_no_desc_.Serialize());
    bs.Add(termsync_response.test_data_desc_.Serialize());
    if ( !conn->Write((unsigned char*)bs.GetBuffer(), bs.GetWritePtr()) )
    {
        //TODO: 暂时默认发送成功
    }

    delete pkg;
}

void SMRequestProcessSvr::AddTermReportWork( BusiConnection *conn, PbTermReportRequest *pkg )
{
    int ret = UpdateTermStatus(pkg->term_status_desc_list_);

    //send responed.
    PbTermReportResponse termreport_response;
    termreport_response.sequence_no_desc_ = pkg->sequence_no_desc_;
    termreport_response.test_data_desc_   = pkg->test_data_desc_;

    ByteStream bs(termreport_response.Serialize());
    bs.Add(termreport_response.sequence_no_desc_.Serialize());
    if ( termreport_response.test_data_desc_.valid_ ) bs.Add(termreport_response.test_data_desc_.Serialize());
    if ( !conn->Write((unsigned char*)bs.GetBuffer(), bs.GetWritePtr()) )
    {
        //TODO:暂时默认发送成功
    }

    delete pkg;
}

void SMRequestProcessSvr::AddSvcPChangeWork( BusiConnection *conn, PbSvcPChangeRequest *pkg )
{
    int ret = UpdateTermParam(pkg);

    //send responed.
    PbSvcPChangeResponse svcpchange_response;
    svcpchange_response.sequence_no_desc_ = pkg->sequence_no_desc_;
    svcpchange_response.test_data_desc_   = pkg->test_data_desc_;

    ByteStream bs(svcpchange_response.Serialize());
    bs.Add(svcpchange_response.sequence_no_desc_.Serialize());
    bs.Add(svcpchange_response.test_data_desc_.Serialize());
    if ( !conn->Write((unsigned char*)bs.GetBuffer(), bs.GetWritePtr()) )
    {
        //TODO:暂时默认发送成功
    }

    delete pkg;
}

void SMRequestProcessSvr::AddSvcApplyWork( BusiConnection *conn, PbSvcApplyRequest *pkg )
{
    unsigned int ret_code = PS_RC_MSG_FORMAT_ERROR;
    do 
    {
        SMSvcApplyWork *svcapply_work = NULL;

        TermSession *self_session   = NULL;
        TermSession *cross_session  = NULL;
        if ( pkg->svc_self_apply_desc_.valid_ )
        {
            self_session = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_self_apply_desc_.session_id_);
            if ( self_session == NULL )
            {
                ret_code = PS_RC_MSG_FORMAT_ERROR;
                break;
            }

            svcapply_work = new SMSvcApplyWork(conn, pkg, self_session);
        }
        else if ( pkg->svc_cross_apply_desc_.valid_ )
        {
            self_session   = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.init_session_id_);
            cross_session  = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.show_session_id_);
            if ( self_session == NULL || cross_session == NULL )
            {
                ret_code = PS_RC_MSG_FORMAT_ERROR;
                break;
            }

            svcapply_work = new SMSvcApplyWork(conn, pkg, self_session, cross_session);
        }
        else
        {
            ret_code = PS_RC_MSG_FORMAT_ERROR;
            break;
        }        

        svcapply_work->user_ptr_    = psm_context_;
        svcapply_work->work_func_   = SMSvcApplyWork::Func_Begin;

        psm_context_->busi_pool_->AddWork(svcapply_work, self_session->CAId());
        return;
    } while (0);

    // 参数错误，直接给终端应答
    SMSvcApplyWork::SendResponed(conn, pkg, ret_code);

    delete pkg;
}

int SMRequestProcessSvr::UpdateTermStatus( list<PB_TerminalStatusDescriptor> &termstatus_desc_list )
{
    list<PB_TerminalStatusDescriptor>::iterator iter = termstatus_desc_list.begin();
    for ( ; iter != termstatus_desc_list.end(); iter++ )
    {
        vector<uint64_t>::iterator iter2 = iter->session_ids_.begin();
        for ( ; iter2 != iter->session_ids_.end(); iter2++ )
        {
            TermSession *term_session = psm_context_->busi_pool_->FindTermSessionById(*iter2);
            if ( term_session != NULL )
            {
                // if status changed, update business status.
                if ( term_session->terminal_info_desc.business_status_ != iter->business_status_ )
                {
                    term_session->terminal_info_desc.business_status_ = iter->business_status_;
                    psm_context_->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(term_session->ca_session);
                }
            }
        }
    }

    return 0;
}

int SMRequestProcessSvr::UpdateTermParam( PbSvcPChangeRequest *pkg )
{
    TermSession *term_session = psm_context_->busi_pool_->FindTermSessionById(pkg->key_map_indicate_desc_.session_id_);
    if ( term_session == NULL )
    {
        return -1;
    }

    CASession *ca_session = term_session->ca_session;
    map<uint64_t, TermSession*>::iterator iter = ca_session->terminal_session_map_.begin();
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        //只给在PhoneControl业务中的终端发送业务切换通知
        if ( iter->second->curr_busi_type == BSPhoneControl )
        {
            PtSvcSwitchRequest *svcswitch_pkg = new PtSvcSwitchRequest;
            svcswitch_pkg->keymap_indicate_desc_ = pkg->key_map_indicate_desc_;
            psm_context_->term_basic_func_svr_->AddSvcSwitchNotifyWork(iter->second->term_conn, svcswitch_pkg);
        }
    }

    return 0;
}
