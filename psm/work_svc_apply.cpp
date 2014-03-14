#include "work_def.h"
#include "psmcontext.h"
#include "cpplib/stringtool.h"
#include "cpplib/timetool.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"


void SvcApplyWork::Func_Begin( Work *work )
{
    SvcApplyWork *svcapply_work  = (SvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    unsigned int ret_code = RC_SUCCESS;
    do 
    {
        // 获取协议头
        char service_name[200] = {0};
        if ( svcapply_work->apply_type_ == SvcApplyWork::SelfSvcApply )
        {
            const char *tmp_str1 = svcapply_work->pkg_->svc_self_apply_desc_.apply_url_.c_str();
            const char *tmp_str2 = strchr(tmp_str1, ':');
            if ( tmp_str2 != NULL )
            {
                strcpy_s(service_name, tmp_str2 - tmp_str1, tmp_str1);
            }
            else
            {
                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }
        else
        {
            const char *tmp_str1 = svcapply_work->pkg_->svc_cross_apply_desc_.init_apply_url_.c_str();
            const char *tmp_str2 = strchr(tmp_str1, ':');
            if ( tmp_str2 != NULL )
            {
                strcpy_s(service_name, tmp_str2 - tmp_str1, tmp_str1);
            }
            else
            {
                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }

        //获取该协议对应的SM地址
        if ( psm_context->business_apply_svr_->GetSMServiceAddr(string(service_name), svcapply_work->http_request_info_.sm_addr_) != 0 )
        {
            ret_code = PT_RC_TERM_APPLY_SVC_ERROR;
            break;
        }

        string psm_addr = psm_context->busi_server_->Addr();

        //设置URL和BODY
        svcapply_work->http_request_info_.SetRequestURL(svcapply_work->http_request_info_.sm_addr_, svcapply_work->self_session_info_->user_info.card_id);
        if ( svcapply_work->apply_type_ == SvcApplyWork::SelfSvcApply )
        {           
            svcapply_work->http_request_info_.SetRequestBody(svcapply_work->pkg_->svc_self_apply_desc_, 
                                                             svcapply_work->self_session_info_->user_info_desc, 
                                                             svcapply_work->self_session_info_->terminal_info_desc, 
                                                             psm_addr);
        }
        else
        {
            svcapply_work->http_request_info_.SetRequestBody(svcapply_work->pkg_->svc_cross_apply_desc_, 
                                                             svcapply_work->self_session_info_->user_info_desc, 
                                                             svcapply_work->self_session_info_->terminal_info_desc, 
                                                             svcapply_work->cross_session_info_->user_info_desc,
                                                             svcapply_work->cross_session_info_->terminal_info_desc,
                                                             psm_addr);
        }

        svcapply_work->run_step_  = SvcApplyWork::SvcApply_Init_begin;
        svcapply_work->work_func_ = SvcApplyWork::Func_Inited;
        psm_context->business_apply_svr_->AddInitRequestWork(svcapply_work);
        return;
    } while ( 0 );

    // 参数错误，直接给终端应答
    SvcApplyWork::SendErrorResponed(svcapply_work->conn_, ret_code);
    SvcApplyWork::Func_End(work);
}

void SvcApplyWork::Func_Inited( Work *work )
{
    SvcApplyWork *svcapply_work  = (SvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = SvcApplyWork::SvcApply_Init_end;

    unsigned int ret_code = RC_SUCCESS;

    do 
    {
        //HTTP请求返回失败
        if ( svcapply_work->http_request_info_.request_result_ != HTTPAysnRequestInfo::OK )
        {
            // business init failed.
            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        //解析HTTP应答包
        char *respond_body_buff = (char*)svcapply_work->http_request_info_.responed_body_.GetBuffer();
        strlwr(respond_body_buff);
        map<string, string> param_map_ = stringtool::to_string_map(respond_body_buff, '\r', '=');

        //判断SM返回值
        unsigned int return_code = atoi(param_map_["returncode"].c_str());
        if ( return_code != RC_SUCCESS )
        {
            // business init failed.
            ret_code = return_code;
            break;
        }

        //解析描述符
        PB_SvcSelfURLDescriptor     svc_self_url_desc;
        PB_SvcCrossURLDescriptor    svc_cross_url_desc;
        PB_KeyMapIndicateDescriptor keymaping_indicate_desc;

        try
        {
            if ( param_map_["svcurl"].size() > 0 )
            {
                ByteStream svc_url;
                svc_url.PutHexString(param_map_["svcurl"]);

                Descriptor desc(svc_url);
                if ( desc.tag_ == TAGv2_PB_SvcSelfURLDescriptor )
                {
                    svc_self_url_desc = desc;
                }
                else if ( desc.tag_ == TAGv2_PB_SvcCrossURLDescriptor )
                {
                    svc_cross_url_desc = desc;
                }
                else
                {
                    ret_code = PT_RC_MSG_FORMAT_ERROR;
                    break;
                }
            }
            else
            {
                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }

            if ( param_map_["keymapindicate"].size() > 0 )
            {
                ByteStream keymap_indicate;
                keymap_indicate.PutHexString(param_map_["keymapindicate"]);
                keymaping_indicate_desc = Descriptor(keymap_indicate);
            }            
        }
        catch (...)
        {
            ret_code = PT_RC_MSG_FORMAT_ERROR;
            break;   	
        }

        PtSvcApplyResponse response(RC_SUCCESS, 0);

        if ( svc_cross_url_desc.valid_ )
        {
            //返回的是跨屏业务url描述符
            if ( svcapply_work->apply_type_ == SvcApplyWork::CorssSvcApply )
            {
                // 通知呈现端业务切换
                PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                svcswitch_request->svc_cross_url_desc_.sm_session_id_ = svc_cross_url_desc.show_sm_session_id_;
                svcswitch_request->svc_cross_url_desc_.url_           = svc_cross_url_desc.show_url_;
                svcswitch_request->svc_cross_url_desc_.back_url_      = svc_cross_url_desc.show_back_url_;
                svcswitch_request->svc_cross_url_desc_.valid_         = true;               

                psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(svcapply_work->cross_session_info_->term_conn, svcswitch_request);

                response.keymap_indicate_desc_             = keymaping_indicate_desc;
                response.svc_self_url_desc_.sm_session_id_ = svc_cross_url_desc.init_sm_session_id_;
                response.svc_self_url_desc_.url_           = svc_cross_url_desc.init_url_;
                response.svc_self_url_desc_.back_url_      = svc_cross_url_desc.init_back_url_;
                response.svc_self_url_desc_.valid_         = true;
            }
            else
            {
                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }
        else if ( svc_self_url_desc.valid_ )
        {
            response.keymap_indicate_desc_             = keymaping_indicate_desc;
            response.svc_self_url_desc_.sm_session_id_ = svc_self_url_desc.sm_session_id_;
            response.svc_self_url_desc_.url_           = svc_self_url_desc.url_;
            response.svc_self_url_desc_.back_url_      = svc_self_url_desc.back_url_;
            response.svc_self_url_desc_.valid_         = true;

        }
        else
        {
            ret_code = PT_RC_MSG_FORMAT_ERROR;
            break;
        }

        //给发起方应答
        ByteStream response_pkg = response.Serialize();
        if ( response.keymap_indicate_desc_.valid_ )    response_pkg.Add(response.keymap_indicate_desc_.Serialize());
        if ( response.svc_self_url_desc_.valid_ )       response_pkg.Add(response.svc_self_url_desc_.Serialize());

        if ( !svcapply_work->conn_->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
        {
            //TODO:
        }

        SvcApplyWork::Func_End(work);
        return;

    } while ( 0 );

    // 参数错误，直接给终端应答
    SvcApplyWork::SendErrorResponed(svcapply_work->conn_, ret_code);
    SvcApplyWork::Func_End(work);       
}

void SvcApplyWork::Func_End( Work *work )
{
    SvcApplyWork *svcapply_work  = (SvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = SvcApplyWork::SvcApply_End;

    delete svcapply_work;
}