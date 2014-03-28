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

    work_name_              = "SM终端业务申请-本屏业务";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(self_session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
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

    work_name_              = "SM终端业务申请-跨屏业务";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(self_session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=" SFMT64U "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

SMSvcApplyWork::~SMSvcApplyWork()
{
    if ( pkg_ != NULL ) delete pkg_;
}

int SMSvcApplyWork::SendResponed( unsigned int ret_code )
{
    // 参数错误，直接给终端应答
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
        //TODO:暂时默认发送成功
        return 0;
    }

    return -1;
}

void SMSvcApplyWork::AddNotifySvcSwitchWork()
{
    PSMContext *psm_context = (PSMContext*)user_ptr_;

	//string service_name = svcapply_work->GetServiceName(pkg_->svc_self_apply_desc_.apply_url_);

    psm_context->logger_.Trace("%s 开始处理业务申请请求, 获取协议头:%s ,该协议头为SM不支持的本地协议头，创建通知终端业务切换工作任务...", log_header_, pkg_->svc_self_apply_desc_.apply_url_.c_str());
    if ( apply_type_ == TermSvcApplyWork::SelfSvcApply )
    {
        // 通知发起端业务切换
        PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
        svcswitch_request->svc_url_desc_.url_       = pkg_->svc_self_apply_desc_.apply_url_;
        svcswitch_request->svc_url_desc_.back_url_  = pkg_->svc_self_apply_desc_.back_url_;
        svcswitch_request->svc_url_desc_.valid_     = true;

        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(self_session_info_, svcswitch_request);
    }
    else
    {
        // 通知发起端业务切换
        PtSvcSwitchRequest *svcswitch_request_init = new PtSvcSwitchRequest;
        svcswitch_request_init->svc_url_desc_.url_       = pkg_->svc_cross_apply_desc_.init_apply_url_;
        svcswitch_request_init->svc_url_desc_.back_url_  = pkg_->svc_cross_apply_desc_.init_back_url_;
        svcswitch_request_init->svc_url_desc_.valid_     = true;

        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(self_session_info_, svcswitch_request_init);

        // 通知呈现端业务切换
        PtSvcSwitchRequest *svcswitch_request_show = new PtSvcSwitchRequest;
        svcswitch_request_show->svc_url_desc_.url_       = pkg_->svc_cross_apply_desc_.show_apply_url_;
        svcswitch_request_show->svc_url_desc_.back_url_  = pkg_->svc_cross_apply_desc_.show_back_url_;
        svcswitch_request_show->svc_url_desc_.valid_     = true;

        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(cross_session_info_, svcswitch_request_show);                
    }

    psm_context->logger_.Trace("%s 开始处理业务申请请求, 获取协议头:%s ,该协议头为SM不支持的本地协议头，创建通知终端业务切换工作任务完成。", log_header_, pkg_->svc_self_apply_desc_.apply_url_.c_str());
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
		psm_context->logger_.Trace("%s 开始处理业务申请请求, 获取协议头为：%s.", svcapply_work->log_header_, service_name.c_str());

		if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
		{ 
			// 当收到的业务申请请求url的协议头为非SM支持的协议头时，需要将该业务申请直接转发给对应终端，通知终端业务切换；            
			svcapply_work->AddNotifySvcSwitchWork();

			psm_context->logger_.Trace("%s 添加终端业务切换通知工作任务成功，向SM发送成功应答。", svcapply_work->log_header_);

			svcapply_work->SendResponed(RC_SUCCESS);
			TermSvcApplyWork::Func_End(work);
			return;
		}

        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
        {
            if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
            {
                //提升
                shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
                if (session_info) {
                    psm_context->logger_.Trace("%s 本屏业务申请的BackURL为空，补填为:%s", svcapply_work->log_header_, session_info->last_local_svc_url_.c_str());
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
                    //提升
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
                    psm_context->logger_.Trace("%s 开始处理业务申请请求, 终端业务申请发起方的BackURL为空，需要补为上个业务的URL...", svcapply_work->log_header_);
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
                    //提升
                    shared_ptr<TermSession> sp_session_info(svcapply_work->self_session_info_.lock());
                    if (sp_session_info)
                        sp_session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.init_back_url_;
                }
            }

            if ( svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_.empty() )
            {
                shared_ptr<TermSession> session_info(svcapply_work->cross_session_info_.lock());
                if (session_info) {
                    psm_context->logger_.Trace("%s 开始处理业务申请请求, 终端业务申请呈现方的BackURL为空，需要补为上个业务的URL...", svcapply_work->log_header_);
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
                    //提升
                    shared_ptr<TermSession> sp_session_info(svcapply_work->cross_session_info_.lock());
                    if (sp_session_info)
                        sp_session_info->last_local_svc_url_ = svcapply_work->pkg_->svc_cross_apply_desc_.show_back_url_;
                }
            }

            apply_url       = svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_.c_str();
            apply_desc_buf  = svcapply_work->pkg_->svc_cross_apply_desc_.SerializeFull();
        }

		/*
		psm_context->logger_.Trace("%s 检查BackURL，如果为空，则需要补填为上一个本地业务的URL...", svcapply_work->log_header_);

		if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
		{
			if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
			{
				psm_context->logger_.Trace("%s 本屏业务申请的BackURL为空，补填为:%s", svcapply_work->log_header_, svcapply_work->self_session_info_->last_local_svc_url_.c_str());
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
				psm_context->logger_.Trace("%s 开始处理业务申请请求, 终端业务申请发起方的BackURL为空，需要补为上个业务的URL...", svcapply_work->log_header_);
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
				psm_context->logger_.Trace("%s 开始处理业务申请请求, 终端业务申请呈现方的BackURL为空，需要补为上个业务的URL...", svcapply_work->log_header_);
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

        // 添加请求任务成功，等待回调Func_Inited即可
        return;
    } while ( 0 );

    // 参数错误，直接给终端应答
    psm_context->logger_.Warn("%s 处理业务申请请求失败，向终端发送失败应答。", svcapply_work->log_header_);

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
        //HTTP请求返回失败
        if ( svcapply_work->http_request_info_.request_result_ != HTTPAysnRequestInfo::OK )
        {
            psm_context->logger_.Warn("%s 处理业务申请请求, 接收SM应答失败。 ", svcapply_work->log_header_);

            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        psm_context->logger_.Trace("%s 处理业务申请请求, 接收到SM的应答，content=%s ", 
                                    svcapply_work->log_header_, 
                                    (char*)svcapply_work->http_request_info_.responed_body_.GetBuffer());

        //解析描述符
        vector<PB_SvcURLDescriptor>   svc_url_desc_list;
        PB_KeyMapIndicateDescriptor   sm_keymaping_indicate_desc;

        ret_code = svcapply_work->ParseHttpResponse(svcapply_work->http_request_info_.responed_body_, svc_url_desc_list, sm_keymaping_indicate_desc);
        if ( ret_code != RC_SUCCESS )
        {
            psm_context->logger_.Warn("%s 处理业务申请请求, 解析接收到SM的应答,解析失败。", svcapply_work->log_header_);

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
                    //更新backurl
                    if ( iter->back_url_.empty() )
                    {
                        iter->back_url_ = show_term_session->last_local_svc_url_;
                    }
                    else
                    {
                        string service_name = svcapply_work->GetServiceName(iter->back_url_.c_str());
                        if ( !psm_context->business_apply_svr_->IsValidServieName(service_name) )
                        {
                            //提升
                            shared_ptr<TermSession> session_info(svcapply_work->self_session_info_.lock());
                            if (session_info)
                                session_info->last_local_svc_url_ = iter->back_url_;
                        }
                    }

                    // 更新终端业务信息
                    show_term_session->UpdateSessionInfo(*iter);

                    // 通知呈现端业务切换
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

					psm_context->logger_.Warn("%s 处理业务申请请求, 添加业务切换通知工作任务。 \n目标终端SID：" SFMT64U "", svcapply_work->log_header_);

                    psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session, svcswitch_request);
                }
            }
            else
            {
                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }

        // 按键映射指示需要单独发送
        if ( need_send_keymap_indicate )
        {
            weak_ptr<TermSession> wp_show_term_session = psm_context->busi_pool_->FindTermSessionById(term_keymaping_indicate_desc.dest_session_id_);
            shared_ptr<TermSession> show_term_session(wp_show_term_session.lock());
            if ( show_term_session != NULL )
            {
                // 通知控制端业务切换
                PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                svcswitch_request->keymap_indicate_desc_ = term_keymaping_indicate_desc;

                psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session, svcswitch_request);
            }
        }
    } while ( 0 );

    // 参数错误，直接给终端应答
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
