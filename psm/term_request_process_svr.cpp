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
//     map<string, string> out_param_map;
//     GetUserInfoParam(pkg->user_info_desc_.user_info_, out_param_map);
//     string card_id = out_param_map["CardID"];
//     if ( card_id.empty() )
//     {
//         LOG_WARN("[TermRequestProcessSvr]CardID param is empty, PSM will ignore this TERM_LOGIN_REQUEST.");
//         return;
//     }
//     
//     TermSession *term_session_info = psm_context_->busi_pool_->GenTermSession(pkg, conn);
// 
//     term_session_info->terminal_info_.card_id_      = out_param_map["CardID"];
//     term_session_info->terminal_info_.user_id_      = out_param_map["UserID"];
//     term_session_info->terminal_info_.terminal_id_  = out_param_map["TerminalID"];
//     term_session_info->terminal_info_.mac_          = out_param_map["MAC"];
//     term_session_info->terminal_info_.licence_key_  = out_param_map["LicenceKey"];
//     
//     term_session_info->service_group_info_.sg_id_   = atoi(out_param_map["ServiceGroupID"]);
// 
//     term_session_info->terminal_info_.odc_version_      = pkg->odclib_desc_.version_;
//     term_session_info->terminal_info_.odc_expired_date_ = pkg->odclib_desc_.expired_date_;
// 
//     term_session_info->terminal_info_.terminal_class_           = pkg->terminal_info_desc_.terminal_class_;
//     term_session_info->terminal_info_.terminal_sub_class_       = pkg->terminal_info_desc_.terminal_sub_class_;
//     term_session_info->terminal_info_.terminal_model_           = pkg->terminal_info_desc_.terminal_model_;
//     term_session_info->terminal_info_.terminal_name_            = pkg->terminal_info_desc_.terminal_name_;
//     term_session_info->terminal_info_.terminal_rel_session_id_  = pkg->terminal_info_desc_.session_id_;
// 
//     term_session_info->curr_business_info_.business_status_ = pkg->terminal_info_desc_.business_status_;
//     term_session_info->curr_business_info_.business_url_    = pkg->terminal_info_desc_.business_url_;
//     term_session_info->curr_business_info_.back_url_stack_.push(pkg->terminal_info_desc_.business_url_);
// 
//     term_session_info->security_cert_info_.safe_user_cert_data_ = pkg->cert_data_desc_.user_cert_data_;
// 
//     term_session_info->net_connection_pt_ = conn;
//     conn->terminal_session_id = term_session_info->session_id_;
//     conn->terminal_session    = term_session_info;
//     conn->ca_session          = term_session_info->term_group_mgr_;
//     conn->caid                = term_session_info->term_group_mgr_->caid_;
    
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
        SvcApplyWork *svcapply_work = NULL;
        if ( pkg->svc_self_apply_desc_.valid_ && (conn->term_session_->Id() == pkg->svc_self_apply_desc_.session_id_) )
        {
            svcapply_work = new SvcApplyWork(conn, pkg, conn->term_session_);
        }
        else if ( pkg->svc_cross_apply_desc_.valid_ && (conn->term_session_->Id() == pkg->svc_cross_apply_desc_.init_session_id_) )
        {
            TermSession *show_term_session = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.show_session_id_);
            if ( show_term_session != NULL )
            {
                svcapply_work = new SvcApplyWork(conn, pkg, conn->term_session_, show_term_session);
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
        svcapply_work->work_func_   = SvcApplyWork::Func_Begin;

        psm_context_->busi_pool_->AddWork(svcapply_work, conn->term_session_->CAId());
        return;
    } while (0);
    
    // 参数错误，直接给终端应答
    SvcApplyWork::SendErrorResponed(conn, ret_code);

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
