#include "work_def.h"
#include "psmcontext.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"

SMSvcApplyWork::SMSvcApplyWork( BusiConnection *conn, PbSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info )
{
    apply_type_             = SelfSvcApply;
    pkg_                    = pkg;
	conn_                   = conn;
    self_session_info_      = self_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = SMSvcApplyWork::Func_Begin;

    work_name_              = "SM�ն�ҵ������-����ҵ��";
    //�������á�
    shared_ptr<TermSession> sp_session_info(self_session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****��ɾ���ĻỰ****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=" SFMT64U "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

SMSvcApplyWork::SMSvcApplyWork( BusiConnection *conn, PbSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info, weak_ptr<TermSession> cross_session_info )
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
    //�������á�
    shared_ptr<TermSession> sp_session_info(self_session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****��ɾ���ĻỰ****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=" SFMT64U "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
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

void SMSvcApplyWork::AddNotifySvcSwitchWork()
{
    PSMContext *psm_context = (PSMContext*)user_ptr_;

	//string service_name = svcapply_work->GetServiceName(pkg_->svc_self_apply_desc_.apply_url_);

    psm_context->logger_.Trace("%s ��ʼ����ҵ����������, ��ȡЭ��ͷ:%s ,��Э��ͷΪSM��֧�ֵı���Э��ͷ������֪ͨ�ն�ҵ���л���������...", log_header_, pkg_->svc_self_apply_desc_.apply_url_.c_str());
    if ( apply_type_ == TermSvcApplyWork::SelfSvcApply )
    {
        // ֪ͨ�����ҵ���л�
        PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
        svcswitch_request->svc_url_desc_.url_       = pkg_->svc_self_apply_desc_.apply_url_;
        svcswitch_request->svc_url_desc_.back_url_  = pkg_->svc_self_apply_desc_.back_url_;
        svcswitch_request->svc_url_desc_.valid_     = true;

        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(self_session_info_, svcswitch_request);
    }
    else
    {
        // ֪ͨ�����ҵ���л�
        PtSvcSwitchRequest *svcswitch_request_init = new PtSvcSwitchRequest;
        svcswitch_request_init->svc_url_desc_.url_       = pkg_->svc_cross_apply_desc_.init_apply_url_;
        svcswitch_request_init->svc_url_desc_.back_url_  = pkg_->svc_cross_apply_desc_.init_back_url_;
        svcswitch_request_init->svc_url_desc_.valid_     = true;

        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(self_session_info_, svcswitch_request_init);

        // ֪ͨ���ֶ�ҵ���л�
        PtSvcSwitchRequest *svcswitch_request_show = new PtSvcSwitchRequest;
        svcswitch_request_show->svc_url_desc_.url_       = pkg_->svc_cross_apply_desc_.show_apply_url_;
        svcswitch_request_show->svc_url_desc_.back_url_  = pkg_->svc_cross_apply_desc_.show_back_url_;
        svcswitch_request_show->svc_url_desc_.valid_     = true;

        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(cross_session_info_, svcswitch_request_show);                
    }

    psm_context->logger_.Trace("%s ��ʼ����ҵ����������, ��ȡЭ��ͷ:%s ,��Э��ͷΪSM��֧�ֵı���Э��ͷ������֪ͨ�ն�ҵ���л�����������ɡ�", log_header_, pkg_->svc_self_apply_desc_.apply_url_.c_str());
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
			apply_url = svcapply_work->pkg_->svc_self_apply_desc_.apply_url_.c_str();
		}
		else
		{
			apply_url = svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_.c_str();
		}

		string service_name = svcapply_work->GetServiceName(apply_url);
		psm_context->logger_.Trace("%s ��ʼ����ҵ����������, ��ȡЭ��ͷΪ��%s.", svcapply_work->log_header_, service_name.c_str());

		if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
		{ 
			// ���յ���ҵ����������url��Э��ͷΪ��SM֧�ֵ�Э��ͷʱ����Ҫ����ҵ������ֱ��ת������Ӧ�նˣ�֪ͨ�ն�ҵ���л���            
			svcapply_work->AddNotifySvcSwitchWork();

			psm_context->logger_.Trace("%s ����ն�ҵ���л�֪ͨ��������ɹ�����SM���ͳɹ�Ӧ��", svcapply_work->log_header_);

			svcapply_work->SendResponed(RC_SUCCESS);
			TermSvcApplyWork::Func_End(work);
			return;
		}

        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
        {
            if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
            {
                //����
                shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
                if (session_info) {
                    psm_context->logger_.Trace("%s ����ҵ�������BackURLΪ�գ�����Ϊ:%s", svcapply_work->log_header_, session_info->last_local_svc_url_.c_str());
                    svcapply_work->pkg_->svc_self_apply_desc_.back_url_ = session_info->last_local_svc_url_;
                } else {
                    //todo:: loggggggg...
                }
            }
            else
            {
                string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_self_apply_desc_.back_url_.c_str());
                if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                {
                    //����
                    shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
                    if (session_info)
                        session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_self_apply_desc_.back_url_;
                }
            }

            apply_url       = svcapply_work->pkg_->svc_self_apply_desc_.apply_url_.c_str();
            apply_desc_buf  = svcapply_work->pkg_->svc_self_apply_desc_.SerializeFull();
        }
        else
        {
            if ( svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.empty() )
            {
                shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
                if (session_info) {
                    psm_context->logger_.Trace("%s ��ʼ����ҵ����������, �ն�ҵ�����뷢�𷽵�BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                    svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_ = session_info->last_local_svc_url_;
                } else {
                    //todo:: logggggggg...
                }
            }
            else
            {
                string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.c_str());
                if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                {
                    //����
                    shared_ptr<TermSession> sp_session_info(svcapply_work->self_session_info_.lock());
                    if (sp_session_info)
                        sp_session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_;
                }
            }

            if ( svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.empty() )
            {
                shared_ptr<TermSession> session_info(svcapply_work->cross_session_info_.lock());
                if (session_info) {
                    psm_context->logger_.Trace("%s ��ʼ����ҵ����������, �ն�ҵ��������ַ���BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                    svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_ = session_info->last_local_svc_url_;
                } else {
                    //todo:: loggggggg...
                }
            }
            else
            {
                string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.c_str());
                if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                {
                    //����
                    shared_ptr<TermSession> sp_session_info(svcapply_work->cross_session_info_.lock());
                    if (sp_session_info)
                        sp_session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_;
                }
            }

            apply_url       = svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_.c_str();
            apply_desc_buf  = svcapply_work->pkg_->svc_cross_apply_desc_.SerializeFull();
        }

		/*
		psm_context->logger_.Trace("%s ���BackURL�����Ϊ�գ�����Ҫ����Ϊ��һ������ҵ���URL...", svcapply_work->log_header_);

		if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
		{
			if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
			{
				psm_context->logger_.Trace("%s ����ҵ�������BackURLΪ�գ�����Ϊ:%s", svcapply_work->log_header_, svcapply_work->self_session_info_->last_local_svc_url_.c_str());
				svcapply_work->pkg_->svc_self_apply_desc_.back_url_ = svcapply_work->self_session_info_->last_local_svc_url_;
			}
			else
			{
				string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_self_apply_desc_.back_url_.c_str());
				if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
				{
					svcapply_work->self_session_info_->last_local_svc_url_ = svcapply_work->pkg_->svc_self_apply_desc_.back_url_;
				}
			}

			apply_desc_buf  = svcapply_work->pkg_->svc_self_apply_desc_.SerializeFull();
		}
		else
		{
			if ( svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.empty() )
			{
				psm_context->logger_.Trace("%s ��ʼ����ҵ����������, �ն�ҵ�����뷢�𷽵�BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
				svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_ = svcapply_work->self_session_info_->last_local_svc_url_;
			}
			else
			{
				string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.c_str());
				if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
				{
					svcapply_work->self_session_info_->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_;
				}
			}

			if ( svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.empty() )
			{
				psm_context->logger_.Trace("%s ��ʼ����ҵ����������, �ն�ҵ��������ַ���BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
				svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_ = svcapply_work->cross_session_info_->last_local_svc_url_;
			}
			else
			{
				string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.c_str());
				if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
				{
					svcapply_work->cross_session_info_->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_;
				}
			}

			apply_desc_buf  = svcapply_work->pkg_->svc_cross_apply_desc_.SerializeFull();
		}
		*/

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
    psm_context->logger_.Warn("%s ����ҵ����������ʧ�ܣ����ն˷���ʧ��Ӧ��", svcapply_work->log_header_);

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
        PB_KeyMapIndicateDescriptor   sm_keymaping_indicate_desc;

        ret_code = svcapply_work->ParseHttpResponse(svcapply_work->http_request_info_.responed_body_, svc_url_desc_list, sm_keymaping_indicate_desc);
        if ( ret_code != RC_SUCCESS )
        {
            psm_context->logger_.Warn("%s ����ҵ����������, �������յ�SM��Ӧ��,����ʧ�ܡ�", svcapply_work->log_header_);

            break;
        }

        PT_KeyMapIndicateDescriptor   term_keymaping_indicate_desc;
        term_keymaping_indicate_desc.dest_session_id_        = sm_keymaping_indicate_desc.session_id_;
        term_keymaping_indicate_desc.dest_sm_session_id_     = sm_keymaping_indicate_desc.dest_sm_session_id_;
        term_keymaping_indicate_desc.mapping_protocol_       = sm_keymaping_indicate_desc.mapping_protocol_;
        term_keymaping_indicate_desc.mapping_server_ip_      = sm_keymaping_indicate_desc.mapping_server_ip_;
        term_keymaping_indicate_desc.mapping_server_port_    = sm_keymaping_indicate_desc.mapping_server_port_;
        term_keymaping_indicate_desc.mapping_type_           = 1;
        term_keymaping_indicate_desc.valid_                  = sm_keymaping_indicate_desc.valid_;

        bool need_send_keymap_indicate = sm_keymaping_indicate_desc.valid_;
        for ( vector<PB_SvcURLDescriptor>::iterator iter = svc_url_desc_list.begin(); iter != svc_url_desc_list.end(); iter++ )
        {
            if ( iter->valid_ )
            {                
                weak_ptr<TermSession> wp_show_term_session = psm_context->busi_pool_->FindTermSessionById(iter->session_id_);
                shared_ptr<TermSession> show_term_session(wp_show_term_session.lock());
                if ( show_term_session != NULL )
                {
                    //����backurl
                    if ( iter->back_url_.empty() )
                    {
                        iter->back_url_ = show_term_session->last_local_svc_url_;
                    }
                    else
                    {
                        string service_name = svcapply_work->GetServiceName(iter->back_url_.c_str());
                        if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                        {
                            //����
                            shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
                            if (session_info)
                                session_info->last_local_svc_url_ = iter->back_url_;
                        }
                    }

                    // �����ն�ҵ����Ϣ
                    show_term_session->UpdateSessionInfo(*iter);

                    // ֪ͨ���ֶ�ҵ���л�
                    PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
					svcswitch_request->svc_url_desc_.url_           = iter->url_;
					svcswitch_request->svc_url_desc_.back_url_      = iter->back_url_;
					svcswitch_request->svc_url_desc_.sm_session_id_ = iter->sm_session_id_;
					svcswitch_request->svc_url_desc_.valid_         = true;
                    //svcswitch_request->svc_url_desc_ = *iter;
                    if ( need_send_keymap_indicate && (sm_keymaping_indicate_desc.session_id_ == iter->session_id_) )
                    {                       
                        svcswitch_request->keymap_indicate_desc_ = term_keymaping_indicate_desc;
                        need_send_keymap_indicate                = false;
                    }

					psm_context->logger_.Warn("%s ����ҵ����������, ���ҵ���л�֪ͨ�������� \nĿ���ն�SID��" SFMT64U "", svcapply_work->log_header_);

                    psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session, svcswitch_request);
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
            weak_ptr<TermSession> wp_show_term_session = psm_context->busi_pool_->FindTermSessionById(term_keymaping_indicate_desc.dest_session_id_);
            shared_ptr<TermSession> show_term_session(wp_show_term_session.lock());
            if ( show_term_session != NULL )
            {
                // ֪ͨ���ƶ�ҵ���л�
                PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                svcswitch_request->keymap_indicate_desc_ = term_keymaping_indicate_desc;

                psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session, svcswitch_request);
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

    if ( psm_context ) {//??????
    }

    svcapply_work->run_step_ = SMSvcApplyWork::SvcApply_End;

    delete svcapply_work;
}
