#include "work_def.h"
#include "psmcontext.h"
#include "cpplib/stringtool.h"
#include "cpplib/timetool.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"


void TRequestWork_Login::Func_Begin( Work *work )
{
    TRequestWork_Login *login_work  = (TRequestWork_Login*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    TermSession *session_info       = login_work->session_info_;
    CASession   *ca_session_info    = login_work->session_info_->ca_session;
    
    // send response to terminal.

    login_work->run_step_ = TRequestWork_Login::Login_SendResponse;

    PtLoginResponse login_response;

    login_response.Add(login_work->session_info_->session_info_desc);
    if ( login_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(login_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        login_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = login_response.Serialize();

    psm_context->logger_.Trace("[�ն˵�¼����][CAID=%I64d][SID=%I64d] ���ն˷���Ӧ��Ӧ������ȣ�%d  ���ݣ�\n%s", 
                                session_info->CAId(), session_info->Id(),
                                responed_pkg.Size(),
                                stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    if ( session_info->term_conn->Write(responed_pkg.GetBuffer(), responed_pkg.Size()) )
    {
        // notify relate terminal status changed.
        psm_context->logger_.Trace("[�ն˵�¼����][CAID=%I64d][SID=%I64d] ���ն˷���Ӧ��ɹ�, ��ʼ����֪ͨ�����ն�״̬���֪ͨ��������...");

        login_work->run_step_ = TRequestWork_Login::Login_NotifyStatusChanged;
        //psm_context->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session_info);
    }
    else
    {
        //TODO: ��ʱĬ�Ϸ��ͳɹ�
    }

    TRequestWork_Login::Func_End(work);
}

void TRequestWork_Login::Func_End( Work *work )
{
    TRequestWork_Login *login_work  = (TRequestWork_Login*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    login_work->run_step_ = TRequestWork_Login::Login_End;

    delete login_work;
}

//////////////////////////////////////////////////////////////////////////

void TRequestWork_Logout::Func_Begin( Work *work )
{
    TRequestWork_Logout *logout_work  = (TRequestWork_Logout*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    //remove this session.
    logout_work->run_step_ = TRequestWork_Logout::Logout_ReleaseSession;

    CASession       *ca_session  = logout_work->session_info_->ca_session;
    TermConnection  *term_conn   = logout_work->session_info_->term_conn;

    uint64_t    caid_id         = logout_work->session_info_->CAId();
    uint64_t    term_session_id = logout_work->session_info_->Id();

    psm_context->logger_.Trace("[�ն��˳�����][CAID=%I64d][SID=%I64d] ɾ���Ự����֪ͨ���������ն�״̬���...", caid_id, term_session_id);

    if ( psm_context->busi_pool_->DelTermSession(logout_work->session_info_) > 0 )
    {
        // notify other terminal status changed.
        psm_context->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session);
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

    psm_context->logger_.Trace("[�ն��˳�����][CAID=%I64d][SID=%I64d] ���ն˷���Ӧ��. ���ȣ�%d  ���ݣ�\n%s", 
                                caid_id, term_session_id,
                                responed_pkg.Size(),
                                stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    if ( !term_conn->Write(responed_pkg.GetBuffer(), responed_pkg.Size()) )
    {
        // TODO:��ʱĬ�Ϸ��ͳɹ�
    }    

    TRequestWork_Logout::Func_End(work);
}

void TRequestWork_Logout::Func_End( Work *work )
{
    TRequestWork_Logout *logout_work  = (TRequestWork_Logout*)work;
    PSMContext *psm_context         = (PSMContext*)work->user_ptr_;

    logout_work->run_step_ = TRequestWork_Logout::Logout_End;

    delete logout_work;
}

//////////////////////////////////////////////////////////////////////////

void TRequestWork_Heartbeat::Func_Begin( Work *work )
{
    TRequestWork_Heartbeat *heartbeat_work  = (TRequestWork_Heartbeat*)work;
    PSMContext *psm_context                 = (PSMContext*)work->user_ptr_;

    heartbeat_work->session_info_->term_conn->last_heartbeat_time_ = timetool::get_up_time();

    //generate response package, and send responed.
    heartbeat_work->run_step_ = TRequestWork_Heartbeat::Heartbeat_SendResponse;

    PtHeartbeatResponse heartbeat_response;
    if ( heartbeat_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(heartbeat_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",heartbeat_work->session_info_->term_conn->last_heartbeat_time_));
        heartbeat_response.Add(test_data_desc);
    }

    ByteStream responed_pkg = heartbeat_response.Serialize();

    psm_context->logger_.Trace("[�ն���������][CAID=%I64d][SID=%I64d] ���ն˷���Ӧ��. ���ȣ�%d  ���ݣ�\n%s", 
                                heartbeat_work->session_info_->CAId(), heartbeat_work->session_info_->Id(),
                                responed_pkg.Size(),
                                stringtool::to_hex_string((const char*)responed_pkg.GetBuffer(), responed_pkg.Size()).c_str());

    if ( !heartbeat_work->session_info_->term_conn->Write((unsigned char*)responed_pkg.GetBuffer(), responed_pkg.GetWritePtr()) )
    {
        // TODO:��ʱĬ�Ϸ��ͳɹ�
    }

    TRequestWork_Heartbeat::Func_End(work);
}

void TRequestWork_Heartbeat::Func_End( Work *work )
{
    TRequestWork_Heartbeat *heartbeat_work  = (TRequestWork_Heartbeat*)work;
    PSMContext *psm_context                 = (PSMContext*)work->user_ptr_;

    heartbeat_work->run_step_ = TRequestWork_Heartbeat::Heartbeat_End;

    delete heartbeat_work;
}

//////////////////////////////////////////////////////////////////////////


void TRequestWork_StatusQuery::Func_Begin( Work *work )
{
    TRequestWork_StatusQuery *statusquery_work  = (TRequestWork_StatusQuery*)work;
    PSMContext *psm_context                     = (PSMContext*)work->user_ptr_;

    statusquery_work->run_step_ = TRequestWork_StatusQuery::StatusQuery_SendResponse;

    //get all relate terminal info.
    CASession *ca_session = statusquery_work->session_info_->ca_session;

    PtStatusQueryResponse notifyquery_response;
    map<uint64_t, TermSession*>::iterator iter = ca_session->terminal_session_map_.begin();
    for ( ; iter != ca_session->terminal_session_map_.end(); iter++ )
    {
        notifyquery_response.Add(iter->second->terminal_info_desc);
    }
    if ( statusquery_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(statusquery_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        notifyquery_response.Add(test_data_desc);
    }

    ByteStream response_pkg = notifyquery_response.Serialize();

    psm_context->logger_.Trace("[�ն�״̬��ѯ����][CAID=%I64d][SID=%I64d] ���ն˷���Ӧ��. ���ȣ�%d  ���ݣ�\n%s", 
                                statusquery_work->session_info_->CAId(), statusquery_work->session_info_->Id(),
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(), response_pkg.Size()).c_str());

    if ( !statusquery_work->session_info_->term_conn->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
    }

    TRequestWork_StatusQuery::Func_End(work);
}

void TRequestWork_StatusQuery::Func_End( Work *work )
{
    TRequestWork_StatusQuery *statusquery_work  = (TRequestWork_StatusQuery*)work;
    PSMContext *psm_context                     = (PSMContext*)work->user_ptr_;

    statusquery_work->run_step_ = TRequestWork_StatusQuery::StatusQuery_End;

    delete statusquery_work;
}

//////////////////////////////////////////////////////////////////////////


void TRequestWork_GetSvcGroup::Func_Begin( Work *work )
{
    TRequestWork_GetSvcGroup *getsvcgroup_work  = (TRequestWork_GetSvcGroup*)work;
    PSMContext *psm_context                     = (PSMContext*)work->user_ptr_;

    //get service group info.

    //send responed.
    getsvcgroup_work->run_step_ = TRequestWork_GetSvcGroup::GetSvcGroup_SendResponse;

    PtGetSvcGroupResponse getsvcgroup_response;
    getsvcgroup_response.sg_id_desc_.service_group_id_      = getsvcgroup_work->session_info_->service_grp.sg_id;
    getsvcgroup_response.sg_indicate_desc_.frequency_       = getsvcgroup_work->session_info_->service_grp.freq;
    getsvcgroup_response.sg_indicate_desc_.modulation_      = getsvcgroup_work->session_info_->service_grp.modulation;
    getsvcgroup_response.sg_indicate_desc_.symbol_rate_     = getsvcgroup_work->session_info_->service_grp.symbol_rate;
    getsvcgroup_response.Add(getsvcgroup_response.sg_id_desc_);
    getsvcgroup_response.Add(getsvcgroup_response.sg_indicate_desc_);

    if ( getsvcgroup_work->pkg_->test_data_desc_.valid_ )
    {
        PT_TestDataDescriptor test_data_desc(getsvcgroup_work->pkg_->test_data_desc_.request_str_, to_string("%.3lf",get_up_time()));
        getsvcgroup_response.Add(test_data_desc);
    }

    ByteStream response_pkg = getsvcgroup_response.Serialize();

    psm_context->logger_.Trace("[�ն˻�ȡ�����������][CAID=%I64d][SID=%I64d] ���ն˷���Ӧ��. ���ȣ�%d  ���ݣ�\n%s", 
                                getsvcgroup_work->session_info_->CAId(), getsvcgroup_work->session_info_->Id(),
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(), response_pkg.Size()).c_str());


    if ( !getsvcgroup_work->session_info_->term_conn->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
    }

    TRequestWork_StatusQuery::Func_End(work);
}

void TRequestWork_GetSvcGroup::Func_End( Work *work )
{
    TRequestWork_GetSvcGroup *getsvcgroup_work  = (TRequestWork_GetSvcGroup*)work;
    PSMContext *psm_context                  = (PSMContext*)work->user_ptr_;

    getsvcgroup_work->run_step_ = TRequestWork_GetSvcGroup::GetSvcGroup_End;

    delete getsvcgroup_work;
}

//////////////////////////////////////////////////////////////////////////
