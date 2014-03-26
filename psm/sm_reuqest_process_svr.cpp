#include <cpplib/stringtool.h>
#include "sm_reuqest_process_svr.h"
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
    psm_context_->logger_.Trace("[SM�ն�״̬ͬ������] �յ����󣬿�ʼ���� ҵ��ID��%s   ��ˮ�ţ�" SFMT64U "  ͬ�����ն˸�����%d", pkg->sequence_no_desc_.business_id_.c_str(), pkg->sequence_no_desc_.sequence_no_ , pkg->term_status_desc_list_.size());

    UpdateTermStatus(pkg->term_status_desc_list_);

    //send responed.
    PbTermSyncResponse termsync_response;
    termsync_response.Add(pkg->sequence_no_desc_);
    if ( pkg->test_data_desc_.valid_ )
    {
        PB_TestDataDescriptor test_data_desc(pkg->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        termsync_response.Add(test_data_desc);
    }

    ByteStream bs = termsync_response.Serialize();

    psm_context_->logger_.Trace("[SM�ն�״̬ͬ������] ״̬ͬ����ɣ���SM����Ӧ��Ӧ������ȣ�%d  ���ݣ�\n%s", 
                                bs.Size(),
                                stringtool::to_hex_string((const char*)bs.GetBuffer(), bs.Size()).c_str());

    if ( !conn->Write(bs.GetBuffer(), bs.Size()) )
    {
        //TODO: ��ʱĬ�Ϸ��ͳɹ�
    }

    delete pkg;
}

void SMRequestProcessSvr::AddTermReportWork( BusiConnection *conn, PbTermReportRequest *pkg )
{
    psm_context_->logger_.Trace("[SM�ն�״̬�㱨����] �յ����󣬿�ʼ���� ҵ��ID��%s   ��ˮ�ţ�" SFMT64U "   �㱨���ն˸�����%d", pkg->sequence_no_desc_.business_id_.c_str(), pkg->sequence_no_desc_.sequence_no_ , pkg->term_status_desc_list_.size());

    int ret = UpdateTermStatus(pkg->term_status_desc_list_);
    if ( ret ) {//??????
    }

    //send responed.
    PbTermReportResponse termreport_response;
    termreport_response.Add(pkg->sequence_no_desc_);
    if ( pkg->test_data_desc_.valid_ )
    {
        PB_TestDataDescriptor test_data_desc(pkg->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        termreport_response.Add(test_data_desc);
    }

    ByteStream bs = termreport_response.Serialize();

    psm_context_->logger_.Trace("[SM�ն�״̬�㱨����] ״̬������ɣ���SM����Ӧ��Ӧ������ȣ�%d  ���ݣ�\n%s", 
                                bs.Size(),
                                stringtool::to_hex_string((const char*)bs.GetBuffer(), bs.Size()).c_str());

    if ( !conn->Write(bs.GetBuffer(), bs.Size()) )
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
    }

    delete pkg;
}

void SMRequestProcessSvr::AddSvcPChangeWork( BusiConnection *conn, PbSvcPChangeRequest *pkg )
{
    psm_context_->logger_.Trace("[SM�ն˲����������] �յ����󣬿�ʼ���� ҵ��ID��%s   ��ˮ�ţ�" SFMT64U "", pkg->sequence_no_desc_.business_id_.c_str(), pkg->sequence_no_desc_.sequence_no_);

    int ret = UpdateTermParam(pkg);
    if ( ret ) {//??????
    }

    //send responed.
    PbSvcPChangeResponse svcpchange_response;
    svcpchange_response.Add(pkg->sequence_no_desc_);
    if ( pkg->test_data_desc_.valid_ )
    {
        PB_TestDataDescriptor test_data_desc(pkg->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        svcpchange_response.Add(test_data_desc);
    }

    ByteStream bs = svcpchange_response.Serialize();

    psm_context_->logger_.Trace("[SM�ն˲����������] ���������ɣ���SM����Ӧ��Ӧ������ȣ�%d  ���ݣ�\n%s", 
                                bs.Size(),
                                stringtool::to_hex_string((const char*)bs.GetBuffer(), bs.Size()).c_str());

    if ( !conn->Write(bs.GetBuffer(), bs.Size()) )
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
    }

    delete pkg;
}

void SMRequestProcessSvr::AddSvcApplyWork( BusiConnection *conn, PbSvcApplyRequest *pkg )
{
    psm_context_->logger_.Trace("[SM�ն�ҵ����������] �յ����󣬿�ʼ���� ҵ��ID��%s   ��ˮ�ţ�" SFMT64U "", pkg->sequence_no_desc_.business_id_.c_str(), pkg->sequence_no_desc_.sequence_no_);

    unsigned int ret_code = PS_RC_MSG_FORMAT_ERROR;
    do 
    {
        SMSvcApplyWork *svcapply_work = NULL;

        weak_ptr<TermSession> self_session;
        weak_ptr<TermSession> cross_session;
        if ( pkg->svc_self_apply_desc_.valid_ )
        {
            self_session = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_self_apply_desc_.session_id_);
            shared_ptr<TermSession> self_ts(self_session.lock());
            if ( self_ts == NULL )
            {
                psm_context_->logger_.Warn("[SM�ն�ҵ����������] �յ����󣬿�ʼ���� ҵ��ID��%s   ��ˮ�ţ�" SFMT64U ". ����ҵ��ĻỰ[SID=" SFMT64U "]�����ڡ�", 
                                            pkg->sequence_no_desc_.business_id_.c_str(), pkg->sequence_no_desc_.sequence_no_,
                                            pkg->svc_self_apply_desc_.session_id_);
                ret_code = PS_RC_MSG_FORMAT_ERROR;
                break;
            }

            svcapply_work = new SMSvcApplyWork(conn, pkg, self_session);
        }
        else if ( pkg->svc_cross_apply_desc_.valid_ )
        {
            self_session   = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.init_session_id_);
            cross_session  = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.show_session_id_);
            shared_ptr<TermSession> self_ts(self_session.lock());
            shared_ptr<TermSession> cross_ts(cross_session.lock());
            if ( self_ts == NULL || cross_ts == NULL )
            {
                psm_context_->logger_.Warn("[SM�ն�ҵ����������] �յ����󣬿�ʼ���� ҵ��ID��%s   ��ˮ�ţ�" SFMT64U ". ����ҵ��ĻỰ[Self_SID=" SFMT64U "][Cross_SID=" SFMT64U "]�����ڡ�", 
                    pkg->sequence_no_desc_.business_id_.c_str(), pkg->sequence_no_desc_.sequence_no_,
                    pkg->svc_self_apply_desc_.session_id_,
                    pkg->svc_cross_apply_desc_.show_session_id_);

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

        shared_ptr<TermSession> self_ts(self_session.lock());
        if (self_ts) {
            psm_context_->busi_pool_->AddWork(svcapply_work, self_ts->CAId());
        } else {
            //todo logggggg.....
            // return ??????
        }
        return;
    } while (0);

    // ��������ֱ�Ӹ��ն�Ӧ��

    PbSvcApplyResponse svcapply_response(ret_code, 0);
    if ( pkg->sequence_no_desc_.valid_ )   svcapply_response.Add(pkg->sequence_no_desc_);
    if ( pkg->test_data_desc_.valid_ )
    {
        PB_TestDataDescriptor test_data_desc(pkg->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        svcapply_response.Add(test_data_desc);
    }

    ByteStream response_pkg = svcapply_response.Serialize();

    psm_context_->logger_.Warn("[SM�ն�ҵ����������] ����ʧ�ܣ���SM����ʧ��Ӧ�� ���ȣ�%d  ���ݣ�\n%s", 
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(),response_pkg.Size()).c_str());

    if ( conn->Write(response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
    }

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
            weak_ptr<TermSession> wp_term_session = psm_context_->busi_pool_->FindTermSessionById(*iter2);
            shared_ptr<TermSession> term_session(wp_term_session.lock());
            if ( term_session != NULL )
            {
                // if status changed, update business status.
                if ( term_session->terminal_info_desc_.business_status_ != iter->business_status_ )
                {
                    psm_context_->logger_.Trace("[״̬���֪ͨ][CAID=" SFMT64U "][SID=" SFMT64U "] �Ự��Ϣ���ͱ仯�����ն˷���״̬���֪ͨ��", term_session->CAId(), term_session->Id());

                    term_session->terminal_info_desc_.business_status_ = iter->business_status_;
                    psm_context_->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(term_session->ca_session_, 0);
                }
            }
        }
    }

    return 0;
}

int SMRequestProcessSvr::UpdateTermParam( PbSvcPChangeRequest *pkg )
{
    weak_ptr<TermSession> wp_term_session = psm_context_->busi_pool_->FindTermSessionById(pkg->key_map_indicate_desc_.session_id_);
    shared_ptr<TermSession> term_session(wp_term_session.lock());
    if ( term_session == NULL )
    {
        return -1;
    }

    CASession *ca_session = term_session->ca_session_;
    map<uint64_t, shared_ptr<TermSession> >::iterator iter = ca_session->terminal_session_map_.begin();
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        //ֻ����PhoneControlҵ���е��ն˷���ҵ���л�֪ͨ
        if ( iter->second->curr_busi_type_ == BSPhoneControl )
        {
            psm_context_->logger_.Trace("[ҵ���л�֪ͨ][CAID=" SFMT64U "][SID=" SFMT64U "] SM����ҵ���л�ָʾ�����ն˷���ҵ���л�֪ͨ��", iter->second->CAId(), iter->second->Id());

            PtSvcSwitchRequest *svcswitch_pkg = new PtSvcSwitchRequest;
            svcswitch_pkg->keymap_indicate_desc_ = pkg->key_map_indicate_desc_;
            psm_context_->term_basic_func_svr_->AddSvcSwitchNotifyWork(iter->second, svcswitch_pkg);
        }
    }

    return 0;
}
