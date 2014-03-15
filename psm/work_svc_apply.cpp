#include "work_def.h"
#include "psmcontext.h"
#include "cpplib/stringtool.h"
#include "cpplib/timetool.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"

map<string, vector<string>> SplitString2MapList(const char *szString, char cGap1, char cGap2);

TermSvcApplyWork::TermSvcApplyWork( AioConnection *conn, PtSvcApplyRequest *pkg, TermSession *self_session_info )
{
    apply_type_             = SelfSvcApply;
    conn_                   = conn;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = TermSvcApplyWork::Func_Begin;
}

TermSvcApplyWork::TermSvcApplyWork( AioConnection *conn, PtSvcApplyRequest *pkg, TermSession *self_session_info, TermSession *cross_session_info )
{
    apply_type_             = CorssSvcApply;
    conn_                   = conn;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;
    cross_session_info_     = cross_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = TermSvcApplyWork::Func_Begin;
}

int TermSvcApplyWork::SendErrorResponed( AioConnection *conn, unsigned int ret_code )
{
    // 参数错误，直接给终端应答
    PtSvcApplyResponse response(ret_code, 0);
    ByteStream response_pkg = response.Serialize();
    if ( conn->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:暂时默认发送成功
        return 0;
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

    unsigned int ret_code = RC_SUCCESS;
    do 
    {
        // 获取协议头
        char service_name[200] = {0};
        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
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
        if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
        {           
            PB_TerminalInfoDescriptor terminal_info_desc(svcapply_work->self_session_info_->terminal_info_desc.terminal_class_,svcapply_work->self_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->self_session_info_->user_info.card_id);
            PB_UserInfoDescriptor     user_info_desc(svcapply_work->self_session_info_->user_info_desc.user_info_);

            svcapply_work->http_request_info_.SetRequestBody(svcapply_work->pkg_->svc_self_apply_desc_.Serialize(), 
                                                             user_info_desc.Serialize(), 
                                                             terminal_info_desc.Serialize(), 
                                                             psm_addr);
        }
        else
        {
            PB_TerminalInfoDescriptor terminal_info_desc1(svcapply_work->self_session_info_->terminal_info_desc.terminal_class_, svcapply_work->self_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->self_session_info_->user_info.card_id);
            PB_UserInfoDescriptor     user_info_desc1(svcapply_work->self_session_info_->user_info_desc.user_info_);
            PB_TerminalInfoDescriptor terminal_info_desc2(svcapply_work->cross_session_info_->terminal_info_desc.terminal_class_, svcapply_work->cross_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->cross_session_info_->user_info.card_id);
            PB_UserInfoDescriptor     user_info_desc2(svcapply_work->cross_session_info_->user_info_desc.user_info_);

            svcapply_work->http_request_info_.SetRequestBody(svcapply_work->pkg_->svc_cross_apply_desc_.Serialize(), 
                                                             user_info_desc1.Serialize(), 
                                                             terminal_info_desc1.Serialize(), 
                                                             user_info_desc2.Serialize(),
                                                             terminal_info_desc2.Serialize(),
                                                             psm_addr);
        }

        svcapply_work->run_step_  = TermSvcApplyWork::SvcApply_Init_begin;
        svcapply_work->work_func_ = TermSvcApplyWork::Func_Inited;
        psm_context->business_apply_svr_->AddInitRequestWork(svcapply_work);
        return;
    } while ( 0 );

    // 参数错误，直接给终端应答
    TermSvcApplyWork::SendErrorResponed(svcapply_work->conn_, ret_code);
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
            // business init failed.
            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        //解析HTTP应答包
        char *respond_body_buff = (char*)svcapply_work->http_request_info_.responed_body_.GetBuffer();
        strlwr(respond_body_buff);
        map<string, vector<string>> param_map = SplitString2MapList(respond_body_buff, '\n', '=');

        //判断SM返回值
        if ( (param_map["returncode"].size() != 1) || (atoi(param_map["returncode"][0].c_str()) != RC_SUCCESS) )
        {
            // business init failed.
            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        //解析描述符
        vector<PB_SvcURLDescriptor>     svc_url_desc_list;
        PB_KeyMapIndicateDescriptor   keymaping_indicate_desc;

        try
        {
            for ( unsigned int i = 0; i < param_map["svcurl"].size(); ++i )
            {
                ByteStream svc_url;
                svc_url.PutHexString(param_map["svcurl"][i]);
                PB_SvcURLDescriptor desc;
                desc = Descriptor(svc_url);
                svc_url_desc_list.push_back(desc);
            }

            if ( param_map["keymapindicate"].size() > 0 )
            {
                ByteStream keymap_indicate;
                keymap_indicate.PutHexString(param_map["keymapindicate"][0]);
                keymaping_indicate_desc = Descriptor(keymap_indicate);
            }
        }
        catch (...)
        {
            ret_code = PT_RC_MSG_FORMAT_ERROR;
            break;   	
        }

        PtSvcApplyResponse response(RC_SUCCESS, 0);
        bool need_send_keymap_indicate = keymaping_indicate_desc.valid_;
        for ( vector<PB_SvcURLDescriptor>::iterator iter = svc_url_desc_list.begin(); iter != svc_url_desc_list.end(); iter++ )
        {
            if ( iter->valid_ )
            {
                if ( iter->session_id_ == svcapply_work->self_session_info_->Id() )
                {
                    // 如果返回的BackURL为空，则需要设置为上一个业务的URL
                    if ( iter->back_url_.empty() ) 
                    {
                        //TODO:
                    }

                    svcapply_work->self_session_info_->UpdateSessionInfo(*iter);

                    response.svc_url_desc_ = *iter;
                    if ( need_send_keymap_indicate && (keymaping_indicate_desc.session_id_ == iter->session_id_) )
                    {
                        response.keymap_indicate_desc_ = keymaping_indicate_desc;
                        need_send_keymap_indicate      = false;
                    }

                    // 更新终端业务信息
                    //svcapply_work->self_session_info_->terminal_info_desc.session_id_   = iter->sm_session_id_;
                    svcapply_work->self_session_info_->terminal_info_desc.business_url_ = iter->url_;
                    //svcapply_work->self_session_info_->terminal_info_desc.business_status_;
                }
                else
                {
                    TermSession *show_term_session = psm_context->busi_pool_->FindTermSessionById(iter->session_id_);
                    if ( show_term_session != NULL )
                    {
                        // 如果返回的BackURL为空，则需要设置为上一个业务的URL
                        if ( iter->back_url_.empty() ) 
                        {
                            //TODO:
                        }

                        show_term_session->UpdateSessionInfo(*iter);

                        // 通知呈现端业务切换
                        PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                        svcswitch_request->svc_url_desc_ = *iter;
                        if ( need_send_keymap_indicate && (keymaping_indicate_desc.session_id_ == iter->session_id_) )
                        {
                            svcswitch_request->keymap_indicate_desc_ = keymaping_indicate_desc;
                            need_send_keymap_indicate                = false;
                        }

                        // 更新终端业务信息
                        //show_term_session->terminal_info_desc.session_id_   = iter->sm_session_id_;
                        show_term_session->terminal_info_desc.business_url_ = iter->url_;
                        //show_term_session->terminal_info_desc.business_status_;

                        psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session->term_conn, svcswitch_request);
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
            TermSession *show_term_session = psm_context->busi_pool_->FindTermSessionById(keymaping_indicate_desc.session_id_);
            if ( show_term_session != NULL )
            {
                // 通知控制端业务切换
                PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                svcswitch_request->keymap_indicate_desc_ = keymaping_indicate_desc;

                psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session->term_conn, svcswitch_request);
            }
        }

        //给发起方应答
        ByteStream response_pkg = response.Serialize();
        if ( response.keymap_indicate_desc_.valid_ )    response_pkg.Add(response.keymap_indicate_desc_.Serialize());
        if ( response.svc_url_desc_.valid_ )            response_pkg.Add(response.svc_url_desc_.Serialize());

        if ( !svcapply_work->conn_->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
        {
            //TODO:暂时默认发送成功
        }

        TermSvcApplyWork::Func_End(work);
        return;

    } while ( 0 );

    // 参数错误，直接给终端应答
    TermSvcApplyWork::SendErrorResponed(svcapply_work->conn_, ret_code);
    TermSvcApplyWork::Func_End(work);       
}

void TermSvcApplyWork::Func_End( Work *work )
{
    TermSvcApplyWork *svcapply_work  = (TermSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = TermSvcApplyWork::SvcApply_End;

    delete svcapply_work;
}

//////////////////////////////////////////////////////////////////////////

map<string, vector<string>> SplitString2MapList( const char *szString, char cGap1, char cGap2 )
{
    map<string, vector<string>>    ret_map;
    list<string>                 str_list = stringtool::to_string_list(szString,cGap1);
    list<string>::iterator       it       = str_list.begin();

    if ( str_list.size() != 0 ) {
        for ( ; it != str_list.end(); ++it ) {
            list<string> str_pair = to_string_list(*it,cGap2);

            if ( str_pair.size() == 1 ) {
                list<string>::iterator it_temp  = str_pair.begin();
                list<string>::iterator it_key   = it_temp;
                ret_map[(*it_key)].push_back("");
            }
            else if ( str_pair.size() ==2 ) {
                list<string>::iterator it_temp  = str_pair.begin();
                list<string>::iterator it_key   = it_temp++;
                list<string>::iterator it_value = it_temp;
                ret_map[(*it_key)].push_back(*it_value);
            }            
        }
    }
    return ret_map;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



SMSvcApplyWork::SMSvcApplyWork( AioConnection *conn, PbSvcApplyRequest *pkg, TermSession *self_session_info )
{
    apply_type_             = SelfSvcApply;
    conn_                   = conn;
    pkg_                    = pkg;
    self_session_info_      = self_session_info;

    run_step_               = SvcApply_Begin;
    apply_sucess_           = false;

    work_func_              = SMSvcApplyWork::Func_Begin;
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
}

SMSvcApplyWork::~SMSvcApplyWork()
{
    if ( pkg_ != NULL ) delete pkg_;
}

int SMSvcApplyWork::SendResponed( AioConnection *conn, PbSvcApplyRequest *pkg, unsigned int ret_code )
{
    // 参数错误，直接给终端应答
    PbSvcApplyResponse response;
    response.msg_return_code_desc_ = MsgReturnCodeDescriptor(response.msg_id_, ret_code, 0);
    response.sequence_no_desc_     = pkg->sequence_no_desc_;

    ByteStream response_pkg = response.Serialize();
    if ( response.msg_return_code_desc_.valid_ ) response_pkg.Add(response.msg_return_code_desc_.Serialize());
    if ( response.sequence_no_desc_.valid_ )     response_pkg.Add(response.sequence_no_desc_.Serialize());

    if ( conn->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:暂时默认发送成功
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
        // 获取协议头
        char service_name[200] = {0};
        if ( svcapply_work->apply_type_ == SMSvcApplyWork::SelfSvcApply )
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
        if ( svcapply_work->apply_type_ == SMSvcApplyWork::SelfSvcApply )
        {           
            PB_TerminalInfoDescriptor terminal_info_desc(svcapply_work->self_session_info_->terminal_info_desc.terminal_class_,svcapply_work->self_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->self_session_info_->user_info.card_id);
            PB_UserInfoDescriptor     user_info_desc(svcapply_work->self_session_info_->user_info_desc.user_info_);

            svcapply_work->http_request_info_.SetRequestBody(svcapply_work->pkg_->svc_self_apply_desc_.Serialize(), 
                user_info_desc.Serialize(), 
                terminal_info_desc.Serialize(), 
                psm_addr);
        }
        else
        {
            PB_TerminalInfoDescriptor terminal_info_desc1(svcapply_work->self_session_info_->terminal_info_desc.terminal_class_, svcapply_work->self_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->self_session_info_->user_info.card_id);
            PB_UserInfoDescriptor     user_info_desc1(svcapply_work->self_session_info_->user_info_desc.user_info_);
            PB_TerminalInfoDescriptor terminal_info_desc2(svcapply_work->cross_session_info_->terminal_info_desc.terminal_class_, svcapply_work->cross_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->cross_session_info_->user_info.card_id);
            PB_UserInfoDescriptor     user_info_desc2(svcapply_work->cross_session_info_->user_info_desc.user_info_);

            svcapply_work->http_request_info_.SetRequestBody(svcapply_work->pkg_->svc_cross_apply_desc_.Serialize(), 
                user_info_desc1.Serialize(), 
                terminal_info_desc1.Serialize(), 
                user_info_desc2.Serialize(),
                terminal_info_desc2.Serialize(),
                psm_addr);
        }

        svcapply_work->run_step_  = SMSvcApplyWork::SvcApply_Init_begin;
        svcapply_work->work_func_ = SMSvcApplyWork::Func_Inited;
        psm_context->business_apply_svr_->AddInitRequestWork(svcapply_work);
        return;
    } while ( 0 );

    // 参数错误，直接给终端应答
    SMSvcApplyWork::SendResponed(svcapply_work->conn_, svcapply_work->pkg_, ret_code);
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
            // business init failed.
            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        //解析HTTP应答包
        char *respond_body_buff = (char*)svcapply_work->http_request_info_.responed_body_.GetBuffer();
        strlwr(respond_body_buff);
        map<string, vector<string>> param_map = SplitString2MapList(respond_body_buff, '\n', '=');

        //判断SM返回值
        if ( (param_map["returncode"].size() != 1) || (atoi(param_map["returncode"][0].c_str()) != RC_SUCCESS) )
        {
            // business init failed.
            ret_code = ST_RC_TERM_SVC_INIT_ERROR;
            break;
        }

        //解析描述符
        vector<PB_SvcURLDescriptor>     svc_url_desc_list;
        PB_KeyMapIndicateDescriptor   keymaping_indicate_desc;

        try
        {
            for ( unsigned int i = 0; i < param_map["svcurl"].size(); ++i )
            {
                ByteStream svc_url;
                svc_url.PutHexString(param_map["svcurl"][i]);
                PB_SvcURLDescriptor desc;
                desc = Descriptor(svc_url);
                svc_url_desc_list.push_back(desc);
            }

            if ( param_map["keymapindicate"].size() > 0 )
            {
                ByteStream keymap_indicate;
                keymap_indicate.PutHexString(param_map["keymapindicate"][0]);
                keymaping_indicate_desc = Descriptor(keymap_indicate);
            }
        }
        catch (...)
        {
            ret_code = PT_RC_MSG_FORMAT_ERROR;
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
                    // 如果返回的BackURL为空，则需要设置为上一个业务的URL
                    if ( iter->back_url_.empty() ) 
                    {
                        //TODO:
                    }

                    show_term_session->UpdateSessionInfo(*iter);

                    // 通知呈现端业务切换
                    PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                    svcswitch_request->svc_url_desc_ = *iter;
                    if ( need_send_keymap_indicate && (keymaping_indicate_desc.session_id_ == iter->session_id_) )
                    {
                        svcswitch_request->keymap_indicate_desc_ = keymaping_indicate_desc;
                        need_send_keymap_indicate                = false;
                    }

                    // 更新终端业务信息
                    //show_term_session->terminal_info_desc.session_id_   = iter->sm_session_id_;
                    show_term_session->terminal_info_desc.business_url_ = iter->url_;
                    //show_term_session->terminal_info_desc.business_status_;

                    psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session->term_conn, svcswitch_request);
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
            TermSession *show_term_session = psm_context->busi_pool_->FindTermSessionById(keymaping_indicate_desc.session_id_);
            if ( show_term_session != NULL )
            {
                // 通知控制端业务切换
                PtSvcSwitchRequest *svcswitch_request = new PtSvcSwitchRequest;
                svcswitch_request->keymap_indicate_desc_ = keymaping_indicate_desc;

                psm_context->term_basic_func_svr_->AddSvcSwitchNotifyWork(show_term_session->term_conn, svcswitch_request);
            }
        }

        SendResponed(svcapply_work->conn_, svcapply_work->pkg_, RC_SUCCESS);

        SMSvcApplyWork::Func_End(work);
        return;

    } while ( 0 );

    // 参数错误，直接给终端应答
    SMSvcApplyWork::SendResponed(svcapply_work->conn_, svcapply_work->pkg_, ret_code);
    SMSvcApplyWork::Func_End(work);   
}

void SMSvcApplyWork::Func_End( Work *work )
{
    SMSvcApplyWork *svcapply_work  = (SMSvcApplyWork*)work;
    PSMContext *psm_context      = (PSMContext*)work->user_ptr_;

    svcapply_work->run_step_ = SMSvcApplyWork::SvcApply_End;

    delete svcapply_work;
}

