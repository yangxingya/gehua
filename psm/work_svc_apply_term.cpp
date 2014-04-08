#include "work_def.h"
#include "psmcontext.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"

TermSvcApplyWork::TermSvcApplyWork(PtSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info )
{
    apply_type_             = SelfSvcApply;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = TermSvcApplyWork::Func_Begin;

    //work_name_              = "�ն�ҵ������-����ҵ��";
    work_name_              = "Terminal Business Apply ---> Same Screen Business";
    //�������á�
    shared_ptr<TermSession> sp_session_info(self_session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****Terminal Session be deleted****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

TermSvcApplyWork::TermSvcApplyWork(PtSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info, weak_ptr<TermSession> cross_session_info )
{
    apply_type_             = CorssSvcApply;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;
    cross_session_info_     = cross_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = TermSvcApplyWork::Func_Begin;

    //work_name_              = "�ն�ҵ������-����ҵ��";
    work_name_              = "Terminal Business Apply ---> Cross Screen Business";
    //�������á�
    shared_ptr<TermSession> sp_session_info(self_session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****Terminal Session be deleted****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

int TermSvcApplyWork::SendResponed( ByteStream &response_buf )
{
	shared_ptr<TermSession> session_info(self_session_info_.lock());
	if (!session_info) return -1;

	MutexLock lock(session_info->termconn_mtx_);

	if (session_info->term_conn_) {
		if ( session_info->term_conn_->Write(response_buf.GetBuffer(), response_buf.Size()) )
		{
			//TODO:��ʱĬ�Ϸ��ͳɹ�
			return 0;
		}
	}

    return -1;
}

TermSvcApplyWork::~TermSvcApplyWork()
{
    if ( pkg_ != NULL ) delete pkg_;
}

void TermSvcApplyWork::Func_Begin( Work *work )
{
    TermSvcApplyWork *svcapply_work  = (TermSvcApplyWork*)work;
    PSMContext       *psm_context    = (PSMContext*)work->user_ptr_;

    shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
    if (!session_info) {
        //todo:: loggggggg....
        return;
    }

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

		if ( psm_context->business_apply_svr_->IsPHONEControlSvc(service_name.c_str()) )
		{
			psm_context->logger_.Trace("%s ��ʼ����ҵ����������, ��ȡЭ��ͷ:%s ,��������ֻ�����ҵ����ֱ�ӷ��سɹ�Ӧ��.", svcapply_work->log_header_, service_name.c_str());

			PtSvcApplyResponse svcapply_response(RC_SUCCESS, 0);

			//��ȡ��ǰCA���������ն����еĻ������ն˻ỰID�������ն˷���ӳ��ָʾ������
			weak_ptr<TermSession> wp_stb_term_session = session_info->ca_session_->GetSTBTermSession();
			shared_ptr<TermSession> stb_term_session(wp_stb_term_session.lock());
			if ( stb_term_session != NULL )
			{
				svcapply_response.keymap_indicate_desc_.mapping_type_    = 0;
				svcapply_response.keymap_indicate_desc_.dest_session_id_ = stb_term_session->Id();
				svcapply_response.keymap_indicate_desc_.valid_           = true;

				svcapply_response.Add(svcapply_response.keymap_indicate_desc_);
			}

			ByteStream response_pkg = svcapply_response.Serialize();
			svcapply_work->SendResponed(response_pkg);

			TermSvcApplyWork::Func_End(work);
			return;
		}

		if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
		{
			psm_context->logger_.Warn("%s ��ʼ����ҵ����������, ��ȡЭ��ͷΪ��%s,��Э��ͷϵͳ��֧�֡�", svcapply_work->log_header_, service_name.c_str());

			ret_code = ST_RC_TERM_APPLY_SVC_ERROR;
			break;
		}
		
        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
        {
            if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
            {
                psm_context->logger_.Trace("%s ����ҵ�������BackURLΪ�գ�����Ϊ:%s", svcapply_work->log_header_, session_info->last_local_svc_url_.c_str());
                svcapply_work->pkg_->svc_self_apply_desc_.back_url_ = session_info->last_local_svc_url_;
            }
            else
            {
                string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_self_apply_desc_.back_url_.c_str());
                if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                {
                    session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_self_apply_desc_.back_url_;
                }
            }

            apply_url       = svcapply_work->pkg_->svc_self_apply_desc_.apply_url_.c_str();

            PB_SvcSelfApplyDescriptor svc_self_desc(svcapply_work->pkg_->svc_self_apply_desc_.session_id_, 
                                                    svcapply_work->pkg_->svc_self_apply_desc_.apply_url_, 
                                                    svcapply_work->pkg_->svc_self_apply_desc_.back_url_);
            apply_desc_buf = svc_self_desc.SerializeFull();
        }
        else
        {
            if ( svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.empty() )
            {
                psm_context->logger_.Trace("%s ��ʼ����ҵ����������, �ն�ҵ�����뷢�𷽵�BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_ = session_info->last_local_svc_url_;
            }
            else
            {
                string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_.c_str());
                if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                {
                    session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_;
                }
            }

            if ( svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.empty() )
            {
                shared_ptr<TermSession> cross_session_info(svcapply_work->cross_session_info_.lock());
                if (cross_session_info) {
                    psm_context->logger_.Trace("%s ��ʼ����ҵ����������, �ն�ҵ��������ַ���BackURLΪ�գ���Ҫ��Ϊ�ϸ�ҵ���URL...", svcapply_work->log_header_);
                    svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_ = cross_session_info->last_local_svc_url_;
                } else {
                    //todo:: loggggggg.......
                }
            }
            else
            {
                string service_name = svcapply_work->GetServiceName(svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.c_str());
                if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                {
                    //�������á�
                    shared_ptr<TermSession> cross_session_info(svcapply_work->cross_session_info_.lock());
                    if (cross_session_info)
                        session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_;
                }
            }

            apply_url       = svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_.c_str();

            PB_SvcCrossApplyDescriptor svc_cross_desc(svcapply_work->pkg_->svc_cross_apply_desc_.init_session_id_, 
                                                      svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_, 
                                                      svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_,
                                                      svcapply_work->pkg_->svc_cross_apply_desc_.show_session_id_, 
                                                      svcapply_work->pkg_->svc_cross_apply_desc_.show_apply_url_, 
                                                      svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_);
            apply_desc_buf = svc_cross_desc.SerializeFull();
        }

        svcapply_work->run_step_  = TermSvcApplyWork::SvcApply_Init_begin;
        svcapply_work->work_func_ = TermSvcApplyWork::Func_Inited;

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

    PtSvcApplyResponse svcapply_response(ret_code, 0);
    ByteStream response_pkg = svcapply_response.Serialize();
    svcapply_work->SendResponed(response_pkg);

    TermSvcApplyWork::Func_End(work);
}

void TermSvcApplyWork::Func_Inited( Work *work )
{
    TermSvcApplyWork *svcapply_work  = (TermSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = TermSvcApplyWork::SvcApply_Init_end;

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

        psm_context->logger_.Trace("%s ����ҵ����������, �������յ�SM��Ӧ��content=%s ", 
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

        
        shared_ptr<TermSession> self_session_info(svcapply_work->self_session_info_.lock());
        if (!self_session_info) {
            //todo:: loggggggg.....
            //psm_context->logger_.Warn("%s ����ҵ����������, �������յ�SM��Ӧ��,����ʧ�ܡ�", svcapply_work->log_header_);
            break;
        }

        PtSvcApplyResponse svcapply_response(RC_SUCCESS, 0);
        bool need_send_keymap_indicate = term_keymaping_indicate_desc.valid_;
        for ( vector<PB_SvcURLDescriptor>::iterator iter = svc_url_desc_list.begin(); iter != svc_url_desc_list.end(); iter++ )
        {
            if ( iter->valid_ )
            {
                if ( iter->session_id_ == self_session_info->Id() )
                {
                    //����backurl
                    if ( iter->back_url_.empty() )
                    {
                        iter->back_url_ = self_session_info->last_local_svc_url_;
                    }
                    else
                    {
                        string service_name = svcapply_work->GetServiceName(iter->back_url_.c_str());
                        if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                        {
                            self_session_info->last_local_svc_url_ = iter->back_url_;
                        }
                    }

                    //�����ն�ҵ����Ϣ
                    self_session_info->UpdateSessionInfo(*iter);

                    svcapply_response.svc_url_desc_.sm_session_id_ = iter->sm_session_id_;
                    svcapply_response.svc_url_desc_.url_           = iter->url_;
                    svcapply_response.svc_url_desc_.back_url_      = iter->back_url_;
                    svcapply_response.svc_url_desc_.valid_         = true;

                    if ( need_send_keymap_indicate && (term_keymaping_indicate_desc.dest_session_id_ == iter->session_id_) )
                    {
                        need_send_keymap_indicate      = false;
                    }
                }
                else
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
                                show_term_session->last_local_svc_url_ = iter->back_url_;
                            }
                        }

                        //�����ն�ҵ����Ϣ
                        show_term_session->UpdateSessionInfo(*iter);

                        // ֪ͨ���ֶ�ҵ���л�
                        PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
						
						//xxxxxx: add switch notify svc url descriptor.
						svcswitch_request->svc_url_desc_.sm_session_id_ = iter->sm_session_id_;
						svcswitch_request->svc_url_desc_.url_           = iter->url_;
						svcswitch_request->svc_url_desc_.back_url_      = iter->back_url_;
						svcswitch_request->svc_url_desc_.valid_         = true;

                        if ( need_send_keymap_indicate && (term_keymaping_indicate_desc.dest_session_id_ == iter->session_id_) )
                        {
                            need_send_keymap_indicate                = false;
                        }

                        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session, svcswitch_request);
                    }
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


        if ( svcapply_response.keymap_indicate_desc_.valid_ )    svcapply_response.Add(svcapply_response.keymap_indicate_desc_);
        if ( svcapply_response.svc_url_desc_.valid_ )            svcapply_response.Add(svcapply_response.svc_url_desc_);

        //������Ӧ��
        ByteStream response_pkg = svcapply_response.Serialize();

        psm_context->logger_.Trace("%s ����ҵ����������ɹ������ն˷���Ӧ�𡣳��ȣ�%d   ���ݣ�\n%s", 
                                svcapply_work->log_header_,
                                response_pkg.Size(),
                                response_pkg.DumpHex(16, true).c_str());

        svcapply_work->SendResponed(response_pkg);

        TermSvcApplyWork::Func_End(work);
        return;

    } while ( 0 );

    // ��������ֱ�Ӹ��ն�Ӧ��
    psm_context->logger_.Warn("%s ����ҵ����������ʧ�ܣ����ն˷���ʧ��Ӧ��", svcapply_work->log_header_);

    PtSvcApplyResponse svcapply_response(ret_code, 0);
    ByteStream response_pkg = svcapply_response.Serialize();
    svcapply_work->SendResponed(response_pkg);

    TermSvcApplyWork::Func_End(work);
}

void TermSvcApplyWork::Func_End( Work *work )
{
    TermSvcApplyWork *svcapply_work  = (TermSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    svcapply_work->run_step_ = TermSvcApplyWork::SvcApply_End;

    delete svcapply_work;
}

