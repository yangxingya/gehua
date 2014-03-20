#include "work_def.h"
#include "psmcontext.h"
#include "cpplib/stringtool.h"
#include "cpplib/timetool.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"

SMSvcApplyWork::SMSvcApplyWork( AioConnection *conn, PbSvcApplyRequest *pkg, TermSession *self_session_info )
{
    apply_type_             = SelfSvcApply;
    conn_                   = conn;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = SMSvcApplyWork::Func_Begin;

    work_name_              = "SM�ն�ҵ������-����ҵ��";
    sprintf_s(log_header_, 300, "[%s][CAID=%I64d][Self_SID=%I64d]", work_name_.c_str(), self_session_info->CAId(), self_session_info->Id());
}

SMSvcApplyWork::SMSvcApplyWork( AioConnection *conn, PbSvcApplyRequest *pkg, TermSession *self_session_info, TermSession *cross_session_info )
{
    apply_type_             = CorssSvcApply;
    conn_                   = conn;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;
    cross_session_info_     = cross_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = SMSvcApplyWork::Func_Begin;

    work_name_              = "SM�ն�ҵ������-����ҵ��";
    sprintf_s(log_header_, 300, "[%s][CAID=%I64d][Self_SID=%I64d][Cross_SID=%I64d]", work_name_.c_str(), self_session_info->CAId(), self_session_info->Id(), cross_session_info->Id());
}

SMSvcApplyWork::~SMSvcApplyWork()
{
    if ( pkg_ != NULL ) delete pkg_;
}

int SMSvcApplyWork::SendResponed( unsigned int ret_code )
{
    // ��������ֱ�Ӹ��ն�Ӧ��
    PbSvcApplyResponse svcapply_response(ret_code, 0);
    if ( this->pkg_->sequence_no_desc_.valid_ )   svcapply_response.Add(this->pkg_->sequence_no_desc_);
    if ( this->pkg_->test_data_desc_.valid_ )
    {
        PB_TestDataDescriptor test_data_desc(this->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        svcapply_response.Add(test_data_desc);
    }

    ByteStream response_pkg = svcapply_response.Serialize();
    if ( this->conn_->Write(response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
        return 0;
    }

    return -1;
}

void SMSvcApplyWork::Func_Begin( Work *work )
{
    SMSvcApplyWork *svcapply_work  = (SMSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    unsigned int ret_code = RC_SUCCESS;
    do 
    {
        const char *apply_url = NULL;
        ByteStream apply_desc_buf;
        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
        {
            if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
            {
                psm_context->logger_.Trace("[%s] ��ʼ����ҵ����������, �ն�ҵ�������BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                svcapply_work->pkg_->svc_self_apply_desc_.back_url_ = svcapply_work->self_session_info_->GetBackURL(svcapply_work->pkg_->svc_self_apply_desc_.apply_url_);
            }

            apply_url       = svcapply_work->pkg_->svc_self_apply_desc_.apply_url_.c_str();
            apply_desc_buf  = svcapply_work->pkg_->svc_self_apply_desc_.Serialize();
        }
        else
        {
            if ( svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.empty() )
            {
                psm_context->logger_.Trace("[%s] ��ʼ����ҵ����������, �ն�ҵ�����뷢�𷽵�BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_ = svcapply_work->self_session_info_->GetBackURL(svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_);
            }
            if ( svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.empty() )
            {
                psm_context->logger_.Trace("[%s] ��ʼ����ҵ����������, �ն�ҵ��������ַ���BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_ = svcapply_work->cross_session_info_->GetBackURL(svcapply_work->pkg_->svc_cross_apply_desc_.show_apply_url_);
            }

            apply_url       = svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_.c_str();
            apply_desc_buf  = svcapply_work->pkg_->svc_cross_apply_desc_.Serialize();
        }

        psm_context->logger_.Trace("[%s] ��ʼ����ҵ����������, ��ȡЭ��ͷ...", svcapply_work->log_header_);

        string service_name = svcapply_work->GetServiceName(apply_url);

        if ( psm_context->business_apply_svr_->IsPHONEControlSvc(service_name.c_str()) )
        {
            psm_context->logger_.Trace("[%s] ��ʼ����ҵ����������, ��ȡЭ��ͷ:%s ,��������ֻ�����ҵ����ֱ�ӷ��سɹ�Ӧ��.", svcapply_work->log_header_, service_name.c_str());

            svcapply_work->SendResponed(RC_SUCCESS);
            TermSvcApplyWork::Func_End(work);
            return;
        }

        svcapply_work->run_step_  = SMSvcApplyWork::SvcApply_Init_begin;
        svcapply_work->work_func_ = SMSvcApplyWork::Func_Inited;

        ret_code = svcapply_work->AddSendHttpRequestWork(service_name, apply_desc_buf);
        if ( ret_code != RC_SUCCESS )
        {
            break;
        }

        // �����������ɹ����ȴ��ص�Func_Inited����
        return;
    } while ( 0 );

    // ��������ֱ�Ӹ��ն�Ӧ��
    psm_context->logger_.Trace("%s ����ҵ����������ʧ�ܣ����ն˷���ʧ��Ӧ��", svcapply_work->log_header_);

    svcapply_work->SendResponed(ret_code);

    SMSvcApplyWork::Func_End(work);
}

void SMSvcApplyWork::Func_Inited( Work *work )
{
    SMSvcApplyWork *svcapply_work  = (SMSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = SMSvcApplyWork::SvcApply_Init_end;

    unsigned int ret_code = RC_SUCCESS;

    do 
    {
        //HTTP���󷵻�ʧ��
        if ( svcapply_work->http_request_info_.request_result_ != HTTPAysnRequestInfo::OK )
        {
            psm_context->logger_.Warn("%s ����ҵ����������, ����SMӦ��ʧ�ܡ� ", svcapply_work->log_header_);

            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        psm_context->logger_.Trace("%s ����ҵ����������, ���յ�SM��Ӧ��content=%s ", 
                                    svcapply_work->log_header_, 
                                    (char*)svcapply_work->http_request_info_.responed_body_.GetBuffer());

        //����������
        vector<PB_SvcURLDescriptor>   svc_url_desc_list;
        PB_KeyMapIndicateDescriptor   keymaping_indicate_desc;

        ret_code = svcapply_work->ParseHttpResponse(svcapply_work->http_request_info_.responed_body_, svc_url_desc_list, keymaping_indicate_desc);
        if ( ret_code != RC_SUCCESS )
        {
            psm_context->logger_.Warn("%s ����ҵ����������, �������յ�SM��Ӧ��,����ʧ�ܡ�", svcapply_work->log_header_);

            break;
        }

        bool need_send_keymap_indicate = keymaping_indicate_desc.valid_;
        for ( vector<PB_SvcURLDescriptor>::iterator iter = svc_url_desc_list.begin(); iter != svc_url_desc_list.end(); iter++ )
        {
            if ( iter->valid_ )
            {                
                TermSession *show_term_session = psm_context->busi_pool_->FindTermSessionById(iter->session_id_);
                if ( show_term_session != NULL )
                {
                    //����backurl
                    show_term_session->UpdateBackURL(*iter);

                    // �����ն�ҵ����Ϣ
                    show_term_session->UpdateSessionInfo(*iter);

                    // ֪ͨ���ֶ�ҵ���л�
                    PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                    svcswitch_request->svc_url_desc_ = *iter;
                    if ( need_send_keymap_indicate && (keymaping_indicate_desc.session_id_ == iter->session_id_) )
                    {
                        svcswitch_request->keymap_indicate_desc_ = keymaping_indicate_desc;
                        need_send_keymap_indicate                = false;
                    }

                    psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session->term_conn, svcswitch_request);
                }
            }
            else
            {
                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }

        // ����ӳ��ָʾ��Ҫ��������
        if ( need_send_keymap_indicate )
        {
            TermSession *show_term_session = psm_context->busi_pool_->FindTermSessionById(keymaping_indicate_desc.session_id_);
            if ( show_term_session != NULL )
            {
                // ֪ͨ���ƶ�ҵ���л�
                PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                svcswitch_request->keymap_indicate_desc_ = keymaping_indicate_desc;

                psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session->term_conn, svcswitch_request);
            }
        }
    } while ( 0 );

    // ��������ֱ�Ӹ��ն�Ӧ��
    svcapply_work->SendResponed(ret_code);

    SMSvcApplyWork::Func_End(work);   
}

void SMSvcApplyWork::Func_End( Work *work )
{
    SMSvcApplyWork *svcapply_work  = (SMSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = SMSvcApplyWork::SvcApply_End;

    delete svcapply_work;
}
