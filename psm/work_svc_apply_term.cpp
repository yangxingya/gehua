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

    //work_name_              = "终端业务申请-本屏业务";
    work_name_              = "Terminal Business Apply ---> Same Screen Business";
    //提升引用。
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

    //work_name_              = "终端业务申请-跨屏业务";
    work_name_              = "Terminal Business Apply ---> Cross Screen Business";
    //提升引用。
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
			//TODO:暂时默认发送成功
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
		psm_context->logger_.Trace("%s 开始处理业务申请请求, 获取协议头为：%s.", svcapply_work->log_header_, service_name.c_str());

		if ( psm_context->business_apply_svr_->IsPHONEControlSvc(service_name.c_str()) )
		{
			psm_context->logger_.Trace("%s 开始处理业务申请请求, 获取协议头:%s ,申请的是手机外设业务，则直接返回成功应答.", svcapply_work->log_header_, service_name.c_str());

			PtSvcApplyResponse svcapply_response(RC_SUCCESS, 0);

			//获取当前CA卡关联的终端组中的机顶盒终端会话ID，并向终端发送映射指示描述符
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
			psm_context->logger_.Warn("%s 开始处理业务申请请求, 获取协议头为：%s,该协议头系统不支持。", svcapply_work->log_header_, service_name.c_str());

			ret_code = ST_RC_TERM_APPLY_SVC_ERROR;
			break;
		}
		
        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
        {
            if ( svcapply_work->pkg_->svc_self_apply_desc_.back_url_.empty() )
            {
                psm_context->logger_.Trace("%s 本屏业务申请的BackURL为空，补填为:%s", svcapply_work->log_header_, session_info->last_local_svc_url_.c_str());
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
                psm_context->logger_.Trace("%s 开始处理业务申请请求, 终端业务申请发起方的BackURL为空，需要补为上个业务的URL...", svcapply_work->log_header_);
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
                    psm_context->logger_.Trace("%s 开始处理业务申请请求, 终端业务申请呈现方的BackURL为空，需要补为上个业务的URL...", svcapply_work->log_header_);
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
                    //提升引用。
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

        // 添加请求任务成功，等待回调Func_Inited即可
        return;
    } while ( 0 );

    // 参数错误，直接给终端应答
    psm_context->logger_.Warn("%s 处理业务申请请求失败，向终端发送失败应答。", svcapply_work->log_header_);

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
        //HTTP请求返回失败
        if ( svcapply_work->http_request_info_.request_result_ != HTTPAysnRequestInfo::OK )
        {
            psm_context->logger_.Warn("%s 处理业务申请请求, 接收SM应答失败。 ", svcapply_work->log_header_);

            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        psm_context->logger_.Trace("%s 处理业务申请请求, 解析接收到SM的应答，content=%s ", 
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

        
        shared_ptr<TermSession> self_session_info(svcapply_work->self_session_info_.lock());
        if (!self_session_info) {
            //todo:: loggggggg.....
            //psm_context->logger_.Warn("%s 处理业务申请请求, 解析接收到SM的应答,解析失败。", svcapply_work->log_header_);
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
                    //更新backurl
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

                    //更新终端业务信息
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
                                show_term_session->last_local_svc_url_ = iter->back_url_;
                            }
                        }

                        //更新终端业务信息
                        show_term_session->UpdateSessionInfo(*iter);

                        // 通知呈现端业务切换
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


        if ( svcapply_response.keymap_indicate_desc_.valid_ )    svcapply_response.Add(svcapply_response.keymap_indicate_desc_);
        if ( svcapply_response.svc_url_desc_.valid_ )            svcapply_response.Add(svcapply_response.svc_url_desc_);

        //给发起方应答
        ByteStream response_pkg = svcapply_response.Serialize();

        psm_context->logger_.Trace("%s 处理业务申请请求成功，向终端发送应答。长度：%d   内容：\n%s", 
                                svcapply_work->log_header_,
                                response_pkg.Size(),
                                response_pkg.DumpHex(16, true).c_str());

        svcapply_work->SendResponed(response_pkg);

        TermSvcApplyWork::Func_End(work);
        return;

    } while ( 0 );

    // 参数错误，直接给终端应答
    psm_context->logger_.Warn("%s 处理业务申请请求失败，向终端发送失败应答。", svcapply_work->log_header_);

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

