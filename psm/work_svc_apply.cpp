#include "work_def.h"
#include "psmcontext.h"
#include "cpplib/stringtool.h"
#include "cpplib/timetool.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"

int SvcApplyWork::AddSendHttpRequestWork(string &service_name, ByteStream &svcapply_desc_buf)
{
    SvcApplyWork     *svcapply_work  = this;
    PSMContext       *psm_context    = (PSMContext*)svcapply_work->user_ptr_;

    unsigned int ret_code = RC_SUCCESS;

    if ( service_name.empty() )
    {
        psm_context->logger_.Trace("[%s] 开始处理业务申请请求, 协议头为空, 返回失败。", log_header_);

        ret_code = PT_RC_MSG_FORMAT_ERROR;
        return ret_code;
    }

    psm_context->logger_.Trace("%s 处理业务申请请求, 协议头为：%s, 通过协议头获取SM地址...", 
        log_header_, service_name.c_str());

    //获取该协议对应的SM地址
    if ( psm_context->business_apply_svr_->GetSMServiceAddr(service_name, svcapply_work->http_request_info_.sm_addr_) != 0 )
    {
        psm_context->logger_.Trace("%s 处理业务申请请求, 协议头为：%s, 通过协议头获取SM地址失败，返回失败。", 
            log_header_, service_name.c_str());

        ret_code = PT_RC_TERM_APPLY_SVC_ERROR;
        return ret_code;
    }

    psm_context->logger_.Trace("%s 处理业务申请请求, 协议头为：%s, 该协议头对应的SM地址：%s", 
        log_header_, service_name.c_str(), svcapply_work->http_request_info_.sm_addr_);

    string psm_addr = psm_context->busi_server_->Addr();

    //设置URL和BODY
    svcapply_work->http_request_info_.SetRequestURL(svcapply_work->http_request_info_.sm_addr_, svcapply_work->self_session_info_->user_info.card_id);

    if ( svcapply_work->apply_type_ == TermSvcApplyWork::SelfSvcApply )
    {           
        PB_TerminalInfoDescriptor terminal_info_desc(svcapply_work->self_session_info_->terminal_info_desc.terminal_class_,svcapply_work->self_session_info_->terminal_info_desc.terminal_sub_class_, svcapply_work->self_session_info_->user_info.card_id);
        PB_UserInfoDescriptor     user_info_desc(svcapply_work->self_session_info_->user_info_desc.user_info_);

        svcapply_work->http_request_info_.SetRequestBody(svcapply_desc_buf, 
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

        svcapply_work->http_request_info_.SetRequestBody(svcapply_desc_buf, 
            user_info_desc1.Serialize(), 
            terminal_info_desc1.Serialize(), 
            user_info_desc2.Serialize(),
            terminal_info_desc2.Serialize(),
            psm_addr);
    }

    psm_context->logger_.Trace("%s 处理业务申请请求, 协议头为：%s, 该协议头对应的SM地址：%s, 向SM发起业务初始化请求。\nURL=%s  \ncontent=%s", 
        log_header_, service_name.c_str(), psm_addr.c_str(),
        (char*)svcapply_work->http_request_info_.request_url_.GetBuffer(),
        (char*)svcapply_work->http_request_info_.request_body_.GetBuffer());

    psm_context->business_apply_svr_->AddInitRequestWork(svcapply_work);

    return RC_SUCCESS;
}

string SvcApplyWork::GetServiceName( const char *url )
{
    char        service_name[200]   = {0};
    const char  *tmp_str1           = url;
    const char  *tmp_str2           = strchr(tmp_str1, ':');
    if ( tmp_str2 != NULL )
    {
        unsigned int len = tmp_str2 - tmp_str1;
        strncpy_s(service_name, tmp_str1, len);
    }

    return service_name;
}

int SvcApplyWork::ParseHttpResponse( ByteStream &response_body, vector<PB_SvcURLDescriptor> &svc_url_desc_list, PB_KeyMapIndicateDescriptor &keymaping_indicate_desc )
{
    SvcApplyWork     *svcapply_work  = this;
    PSMContext       *psm_context    = (PSMContext*)svcapply_work->user_ptr_;
    
    //解析HTTP应答包
    char *respond_body_buff = (char*)svcapply_work->http_request_info_.responed_body_.GetBuffer();
    strlwr(respond_body_buff);
    map<string, vector<string>> param_map = SplitString2MapList(respond_body_buff, '\n', '=');

    //判断SM返回值
    if ( (param_map["returncode"].size() != 1) || (atoi(param_map["returncode"][0].c_str()) != RC_SUCCESS) )
    {
        // business init failed.
        return ST_RC_TERM_SVC_INIT_ERROR;
    }

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
        return PT_RC_MSG_FORMAT_ERROR;
    }

    return RC_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
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

void HTTPAysnRequestInfo::SetRequestURL( string &sm_addr, string &card_id )
{
    ByteStream bs;
    bs.Add("http://");
    bs.Add(sm_addr);
    bs.Add("/?userid=");
    bs.Add(card_id);
    bs.Add("&req=PSM_SERVICE_INIT_REQUEST");
    bs.PutUint8(0);

    request_url_ = bs;

    sm_addr_ = sm_addr;
}

void HTTPAysnRequestInfo::SetRequestBody( ByteStream &svc_self_apply_desc, ByteStream &user_info_desc, ByteStream &terminal_info_desc, string &psm_addr )
{
    ByteStream bs;
    ByteStream tmp_bs;

    bs.Add("\r\nSvcApply=");
    bs.Add(svc_self_apply_desc.DumpHex(svc_self_apply_desc.GetWritePtr(), false));

    bs.Add("\r\nUserInfo=");
    bs.Add(user_info_desc.DumpHex(user_info_desc.GetWritePtr(), false));

    bs.Add("\r\nTerminalInfo=");
    bs.Add(terminal_info_desc.DumpHex(terminal_info_desc.GetWritePtr(), false));

    bs.Add("\r\nPsm=");
    bs.Add(psm_addr);

    bs.Add("\r\n");
    bs.PutUint8(0);

    request_body_ = bs;
}

void HTTPAysnRequestInfo::SetRequestBody( ByteStream &svc_cross_apply_desc, ByteStream &user_info_desc, ByteStream &terminal_info_desc, ByteStream &user_info_desc2, ByteStream &terminal_info_desc2, string &psm_addr )
{
    ByteStream bs;

    bs.Add("\r\nSvcApply=");
    bs.Add(svc_cross_apply_desc.DumpHex(svc_cross_apply_desc.GetWritePtr(), false));

    bs.Add("\r\nUserInfo=");
    bs.Add(user_info_desc.DumpHex(user_info_desc.GetWritePtr(), false));

    bs.Add("\r\nTerminalInfo=");
    bs.Add(terminal_info_desc.DumpHex(terminal_info_desc.GetWritePtr(), false));

    bs.Add("\r\nUserInfo2nd=");
    bs.Add(user_info_desc2.DumpHex(user_info_desc2.GetWritePtr(), false));

    bs.Add("\r\nTerminalInfo2nd=");
    bs.Add(terminal_info_desc2.DumpHex(terminal_info_desc2.GetWritePtr(), false));

    bs.Add("\r\nPsm=");
    bs.Add(psm_addr);

    bs.Add("\r\n\0");

    request_body_ = bs;
}
