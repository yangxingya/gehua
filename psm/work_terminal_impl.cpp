#include "work_def.h"
#include "psmcontext.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"


void TRequestWork_Login::Func_Begin( Work *work )
{
    TRequestWork_Login *login_work  = (TRequestWork_Login*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    //need valid login info.

    LoginError error = LoginOK;
    login_work->session_info_ = psm_context->busi_pool_->GenTermSession(login_work->pkg_, login_work->tconn_, &error);

    shared_ptr<TermSession> session_info(login_work->session_info_);
    if (!session_info) {
        PtLoginResponse login_response(error);

        //TODO:: need test data desc???
        if ( login_work->pkg_->test_data_desc_.valid_ )
        {
            PT_TestDataDescriptor test_data_desc;
            login_response.Add(test_data_desc);
        }

        ByteStream responed_pkg = login_response.Serialize();

        psm_context->logger_.Trace("%s Send Login Error Response Pkt, length: %d  context: \n\t%s", 
                    login_work->log_header_,
                    responed_pkg.Size(),
                    stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());


        tconn_->Write(responed_pkg.GetBuffer(), responed_pkg.Size());

        
        //TODO:: delete conn???
        tconn_->SetDirty();
        return;
    }

    CASession   *ca_session_info    = session_info->ca_session_;

    //输出用户登录参数日志：
    psm_context->logger_.Trace("%s Login Request Argument: \n\
                               OdcLib Desc:\n\
                               version=%s\n\
                               expired_date=%0x\n\
                               UserInfo Desc:\n\
                               user_info=%s\n\
                               UserCertData Desc:\n\
                               user_cert_data=%s\n\
                               TerminalInfo Desc:\n\
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
        PT_TestDataDescriptor test_data_desc;
        login_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = login_response.Serialize();

    psm_context->logger_.Trace("%s Send Terminal Login Response, length: %d  context: \n\t%s", 
        login_work->log_header_,
        responed_pkg.Size(),
        stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    bool writed = false;
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_)
            writed = session_info->term_conn_->Write(responed_pkg.GetBuffer(), responed_pkg.Size());
    }
    if (writed)
    {
        // notify relate terminal status changed.
        psm_context->logger_.Trace("%s Send Terminal Login OK Response Successful, Start Create Notify Terminal Relationed Status Changed Work...", login_work->log_header_);

        login_work->run_step_ = TRequestWork_Login::Login_NotifyStatusChanged;
        psm_context->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session_info, session_info->Id());
    }
    else
    {
        psm_context->logger_.Trace("%s Send Terminal Login OK Response Failed, Does It need send repeated??? ...", login_work->log_header_);
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

    CASession *ca_session  = session_info->ca_session_;

    caid_t    caid_id         = session_info->CAId();
    uint64_t  term_session_id = session_info->Id();

    if ( caid_id ) {//??????
    }

    //generate response package, and send responed.
    logout_work->run_step_ = TRequestWork_Logout::Logout_SendResponse;

    PtLogoutResponse logout_response;
    if ( logout_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc;
        logout_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = logout_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
        logout_work->log_header_,
        responed_pkg.Size(),
        stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    bool writed = false;
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_)
            writed = session_info->term_conn_->Write(responed_pkg.GetBuffer(), responed_pkg.Size());
    }
    if (!writed)
    {
        // TODO:暂时默认发送成功
    }

    psm_context->logger_.Trace("%s 删除会话，并通知其他关联终端状态变更...", logout_work->log_header_);

    psm_context->busi_pool_->RemoveFromTimer(session_info);
    if ( psm_context->busi_pool_->DelTermSession(session_info) > 0 )
    {
        // notify other terminal status changed.
        psm_context->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session, term_session_id);
    }

    //删除会话，关闭连接。
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_) {
            session_info->term_conn_->SetDirty();
            session_info->term_conn_ = NULL;
        }
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

    double curr_time = timetool::get_up_time();
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_)
            session_info->term_conn_->last_heartbeat_time_ = curr_time;
    }

    //generate response package, and send responed.
    heartbeat_work->run_step_ = TRequestWork_Heartbeat::Heartbeat_SendResponse;

    PtHeartbeatResponse heartbeat_response;
    if ( heartbeat_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc;
        heartbeat_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = heartbeat_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
        heartbeat_work->log_header_,
        responed_pkg.Size(),
        stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    bool writed = false;
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_)
            writed = session_info->term_conn_->Write((unsigned char*)responed_pkg.GetBuffer(), responed_pkg.GetWritePtr());
    }

    if (!writed)
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
    {
        MutexLock lock(ca_session->termsession_mtx_);
        map<uint64_t, shared_ptr<TermSession> >::iterator iter = ca_session->terminal_session_map_.begin();
        for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
        {
            notifyquery_response.Add(iter->second->terminal_info_desc_);
        }
    }
    if ( statusquery_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc;
        notifyquery_response.Add(test_data_desc);
    }

    ByteStream response_pkg = notifyquery_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
        statusquery_work->log_header_,
        response_pkg.Size(),
        stringtool::to_hex_string((const char*)response_pkg.GetBuffer(), response_pkg.Size()).c_str());

    bool writed = false;
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_)
            writed = session_info->term_conn_->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size());
    }
    if (!writed)
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
        PT_TestDataDescriptor test_data_desc;
        getsvcgroup_response.Add(test_data_desc);
    }

    ByteStream response_pkg = getsvcgroup_response.Serialize();

    psm_context->logger_.Trace("%s 向终端发送应答. 长度：%d  内容：\n%s", 
        getsvcgroup_work->log_header_,
        response_pkg.Size(),
        stringtool::to_hex_string((const char*)response_pkg.GetBuffer(), response_pkg.Size()).c_str());

    bool writed = false;
    {
        MutexLock lock(session_info->termconn_mtx_);
        if (session_info->term_conn_)
            writed = session_info->term_conn_->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size());
    }

    if (!writed)
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
