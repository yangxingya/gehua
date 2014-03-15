#include "term_request_process_svr.h"
#include "cpplib/stringtool.h"
#include "psmcontext.h"
#include "sessionmgr/termsession.h"
#include "sessionmgr/casession.h"


TermRequestProcessSvr::TermRequestProcessSvr( PSMContext* psm_context )
{
    psm_context_    = psm_context;
}

TermRequestProcessSvr::~TermRequestProcessSvr()
{

}

void TermRequestProcessSvr::GetUserInfoParam( string &user_info_str, map<string, string> &out_param_map )
{
    out_param_map = stringtool::to_string_map(user_info_str, '&', '=');
}

void TermRequestProcessSvr::AddLoginRequestWork( TermConnection *conn, PtLoginRequest *pkg )
{    
    TRequestWork_Login *work = new TRequestWork_Login;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_Login::Login_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TRequestWork_Login::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddLogoutRequestWork( TermConnection *conn, PtLogoutRequest *pkg )
{
    TRequestWork_Logout *work = new TRequestWork_Logout;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_Logout::Logout_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TRequestWork_Logout::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddHeartbeatWork( TermConnection *conn, PtHeartbeatRequest *pkg )
{
    TRequestWork_Heartbeat *work = new TRequestWork_Heartbeat;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_Heartbeat::Heartbeat_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TRequestWork_Heartbeat::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddSvcApplyWork( TermConnection *conn, PtSvcApplyRequest *pkg )
{
    unsigned int ret_code = PT_RC_MSG_FORMAT_ERROR;
    do 
    {
        TermSvcApplyWork *svcapply_work = NULL;
        if ( pkg->svc_self_apply_desc_.valid_ && (conn->term_session_->Id() == pkg->svc_self_apply_desc_.session_id_) )
        {
            svcapply_work = new TermSvcApplyWork(conn, pkg, conn->term_session_);
        }
        else if ( pkg->svc_cross_apply_desc_.valid_ && (conn->term_session_->Id() == pkg->svc_cross_apply_desc_.init_session_id_) )
        {
            TermSession *show_term_session = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.show_session_id_);
            if ( show_term_session != NULL )
            {
                svcapply_work = new TermSvcApplyWork(conn, pkg, conn->term_session_, show_term_session);
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

        svcapply_work->user_ptr_    = psm_context_;
        svcapply_work->work_func_   = TermSvcApplyWork::Func_Begin;

        psm_context_->busi_pool_->AddWork(svcapply_work, conn->term_session_->CAId());
        return;
    } while (0);
    
    // 参数错误，直接给终端应答
    TermSvcApplyWork::SendErrorResponed(conn, ret_code);

    delete pkg;
}

void TermRequestProcessSvr::AddStatusQueryWork( TermConnection *conn, PtStatusQueryRequest *pkg )
{
    TRequestWork_StatusQuery *work = new TRequestWork_StatusQuery;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_StatusQuery::StatusQuery_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TRequestWork_StatusQuery::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddGetSvrGroupWork( TermConnection *conn, PtGetSvcGroupRequest *pkg )
{
    TRequestWork_GetSvcGroup *work = new TRequestWork_GetSvcGroup;

    work->pkg_              = pkg;
    work->run_step_         = TRequestWork_GetSvcGroup::GetSvcGroup_Begin;
    work->session_info_     = conn->term_session_;
    work->work_func_        = TRequestWork_GetSvcGroup::Func_Begin;
    work->user_ptr_         = psm_context_;

    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}
