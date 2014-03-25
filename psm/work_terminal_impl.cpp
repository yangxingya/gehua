#include "work_def.h"
#include "psmcontext.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"


void TRequestWork_Login::Func_Begin( Work *work )
{
    TRequestWork_Login *login_work  = (TRequestWork_Login*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    //TermSession *session_info       = login_work->session_info_;
    shared_ptr<TermSession> session_info(login_work->session_info_);
    if (!session_info) {
        //todo:: logggggg......
        return;
    }

    CASession   *ca_session_info    = session_info->ca_session_;
    
    //输出用户登录参数日志：
    psm_context->logger_.Trace("%s 登录请求参数如下：\n\
                                移植库描述符:\n\
                                  version=%s\n\
                                  expired_date=%0x\n\
                                用户信息描述符:\n\
                                  user_info=%s\n\
                                用户证书数据描述符:\n\
                                  user_cert_data=%s\n\
                                终端信息描述符:\n\
                                  terminal_class=%d\n\
                                  terminal_sub_class=%d\n\
                                  terminal_model=%s\n\
                                  terminal_name=%s\n\
                                  session_id=" SFMT64U "\n\
                                  business_status=%d\n\
                                  business_url=%s", 
                                login_work->log_header_,
                                login_work->pkg_->odclib_desc_.version_.c_str(),login_work->pkg_->odclib_desc_.expired_date_,
                                login_work->pkg_->user_info_desc_.user_info_.c_str(),
                                stringtool::to_hex_string((const char*)login_work->pkg_->cert_data_desc_.user_cert_data_.GetBuffer(), login_work->pkg_->cert_data_desc_.user_cert_data_.Size()).c_str(),
                                login_work->pkg_->terminal_info_desc_.terminal_class_, 
                                login_work->pkg_->terminal_info_desc_.terminal_sub_class_, 
                                login_work->pkg_->terminal_info_desc_.terminal_model_.c_str(),
                                login_work->pkg_->terminal_info_desc_.terminal_name_.c_str(),
                                login_work->pkg_->terminal_info_desc_.session_id_,
                                login_work->pkg_->terminal_info_desc_.business_status_,
                                login_work->pkg_->terminal_info_desc_.business_url_.c_str());

    login_work->run_step_ = TRequestWork_Login::Login_SendResponse;

    PtLoginResponse login_response;

    login_response.Add(session_info->session_info_desc_);
    if ( login_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(login_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        login_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = login_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答。应答包长度：%d  内容：\n%s", 
                                login_work->log_header_,
                                responed_pkg.Size(),
                                stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    if ( session_info->term_conn_->Write(responed_pkg.GetBuffer(), responed_pkg.Size()) )
    {
        // notify relate terminal status changed.
        psm_context->logger_.Trace("%s 向终端发送应答成功, 开始创建通知关联终端状态变更通知工作任务...", login_work->log_header_);

        login_work->run_step_ = TRequestWork_Login::Login_NotifyStatusChanged;
        psm_context->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session_info, session_info->Id());
    }
    else
    {
        //TODO: 暂时默认发送成功
    }

    TRequestWork_Login::Func_End(work);
}

void TRequestWork_Login::Func_End( Work *work )
{
    TRequestWork_Login *login_work  = (TRequestWork_Login*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    login_work->run_step_ = TRequestWork_Login::Login_End;

    delete login_work;
}

//////////////////////////////////////////////////////////////////////////

void TRequestWork_Logout::Func_Begin( Work *work )
{
    TRequestWork_Logout *logout_work  = (TRequestWork_Logout*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    shared_ptr<TermSession> session_info(logout_work->session_info_.lock());
    if (!session_info) {
        //todo:: logggggg....

        return;
    }

    //remove this session.
    logout_work->run_step_ = TRequestWork_Logout::Logout_ReleaseSession;

    CASession       *ca_session  = session_info->ca_session_;
    TermConnection  *term_conn   = session_info->term_conn_;

    caid_t    caid_id         = session_info->CAId();
    uint64_t  term_session_id = session_info->Id();

    if ( caid_id ) {//??????
    }

    psm_context->logger_.Trace("%s 删除会话，并通知其他关联终端状态变更...", logout_work->log_header_);

    if ( psm_context->busi_pool_->DelTermSession(logout_work->session_info_) > 0 )
    {
        // notify other terminal status changed.
        psm_context->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session, term_session_id);
    }

    //generate response package, and send responed.
    logout_work->run_step_ = TRequestWork_Logout::Logout_SendResponse;

    PtLogoutResponse logout_response;
    if ( logout_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(logout_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        logout_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = logout_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
                                logout_work->log_header_,
                                responed_pkg.Size(),
                                stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    if ( !term_conn->Write(responed_pkg.GetBuffer(), responed_pkg.Size()) )
    {
        // TODO:暂时默认发送成功
    }    

    TRequestWork_Logout::Func_End(work);
}

void TRequestWork_Logout::Func_End( Work *work )
{
    TRequestWork_Logout *logout_work  = (TRequestWork_Logout*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    logout_work->run_step_ = TRequestWork_Logout::Logout_End;

    delete logout_work;
}

//////////////////////////////////////////////////////////////////////////

void TRequestWork_Heartbeat::Func_Begin( Work *work )
{
    TRequestWork_Heartbeat *heartbeat_work  = (TRequestWork_Heartbeat*)work;
    PSMContext *psm_context                 = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    shared_ptr<TermSession> session_info(heartbeat_work->session_info_.lock());
    if (!session_info) {
        //todo:: logggggg....

        return;
    }

    session_info->term_conn_->last_heartbeat_time_ = timetool::get_up_time();

    //generate response package, and send responed.
    heartbeat_work->run_step_ = TRequestWork_Heartbeat::Heartbeat_SendResponse;

    PtHeartbeatResponse heartbeat_response;
    if ( heartbeat_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(heartbeat_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",session_info->term_conn_->last_heartbeat_time_));
        heartbeat_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = heartbeat_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
                                heartbeat_work->log_header_,
                                responed_pkg.Size(),
                                stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    if ( !session_info->term_conn_->Write((unsigned char*)responed_pkg.GetBuffer(), responed_pkg.GetWritePtr()) )
    {
        // TODO:暂时默认发送成功
    }

    TRequestWork_Heartbeat::Func_End(work);
}

void TRequestWork_Heartbeat::Func_End( Work *work )
{
    TRequestWork_Heartbeat *heartbeat_work  = (TRequestWork_Heartbeat*)work;
    PSMContext *psm_context                 = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    heartbeat_work->run_step_ = TRequestWork_Heartbeat::Heartbeat_End;

    delete heartbeat_work;
}

//////////////////////////////////////////////////////////////////////////


void TRequestWork_StatusQuery::Func_Begin( Work *work )
{
    TRequestWork_StatusQuery *statusquery_work  = (TRequestWork_StatusQuery*)work;
    PSMContext *psm_context                     = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    shared_ptr<TermSession> session_info(statusquery_work->session_info_.lock());
    if (!session_info) {
        //todo:: logggggg....

        return;
    }

    statusquery_work->run_step_ = TRequestWork_StatusQuery::StatusQuery_SendResponse;

    //get all relate terminal info.
    CASession *ca_session = session_info->ca_session_;

    PtStatusQueryResponse notifyquery_response;
    map<uint64_t, weak_ptr<TermSession> >::iterator iter = ca_session->terminal_session_map_.begin();
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        shared_ptr<TermSession> sp_session_info(iter->second.lock());
        if (sp_session_info)
            notifyquery_response.Add(sp_session_info->terminal_info_desc_);
    }
    if ( statusquery_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(statusquery_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        notifyquery_response.Add(test_data_desc);
    }

    ByteStream response_pkg = notifyquery_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
                                statusquery_work->log_header_,
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(), response_pkg.Size()).c_str());

    if ( !session_info->term_conn_->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:暂时默认发送成功
    }

    TRequestWork_StatusQuery::Func_End(work);
}

void TRequestWork_StatusQuery::Func_End( Work *work )
{
    TRequestWork_StatusQuery *statusquery_work  = (TRequestWork_StatusQuery*)work;
    PSMContext *psm_context                     = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    statusquery_work->run_step_ = TRequestWork_StatusQuery::StatusQuery_End;

    delete statusquery_work;
}

//////////////////////////////////////////////////////////////////////////


void TRequestWork_GetSvcGroup::Func_Begin( Work *work )
{
    TRequestWork_GetSvcGroup *getsvcgroup_work  = (TRequestWork_GetSvcGroup*)work;
    PSMContext *psm_context                     = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    shared_ptr<TermSession> session_info(getsvcgroup_work->session_info_.lock());
    if (!session_info) {
        //todo:: logggggg....

        return;
    }
    //get service group info.

    //send responed.
    getsvcgroup_work->run_step_ = TRequestWork_GetSvcGroup::GetSvcGroup_SendResponse;

    PtGetSvcGroupResponse getsvcgroup_response;
    getsvcgroup_response.sg_id_desc_.service_group_id_      = session_info->service_grp_.sg_id;
    getsvcgroup_response.sg_indicate_desc_.frequency_       = session_info->service_grp_.freq;
    getsvcgroup_response.sg_indicate_desc_.modulation_      = session_info->service_grp_.modulation;
    getsvcgroup_response.sg_indicate_desc_.symbol_rate_     = session_info->service_grp_.symbol_rate;
    getsvcgroup_response.Add(getsvcgroup_response.sg_id_desc_);
    getsvcgroup_response.Add(getsvcgroup_response.sg_indicate_desc_);

    if ( getsvcgroup_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(getsvcgroup_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        getsvcgroup_response.Add(test_data_desc);
    }

    ByteStream response_pkg = getsvcgroup_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
                                getsvcgroup_work->log_header_,
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(), response_pkg.Size()).c_str());


    if ( !session_info->term_conn_->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:暂时默认发送成功
    }

    TRequestWork_StatusQuery::Func_End(work);
}

void TRequestWork_GetSvcGroup::Func_End( Work *work )
{
    TRequestWork_GetSvcGroup *getsvcgroup_work  = (TRequestWork_GetSvcGroup*)work;
    PSMContext *psm_context                  = (PSMContext*)work->user_ptr_;

    if ( psm_context ) {//??????
    }

    getsvcgroup_work->run_step_ = TRequestWork_GetSvcGroup::GetSvcGroup_End;

    delete getsvcgroup_work;
}

//////////////////////////////////////////////////////////////////////////
