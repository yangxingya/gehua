#include <cpplib/stringtool.h>
#include "term_request_process_svr.h"
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
    TRequestWork_Login *work = new TRequestWork_Login(pkg, conn->term_session_, psm_context_);
    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddLogoutRequestWork( TermConnection *conn, PtLogoutRequest *pkg )
{
    TRequestWork_Logout *work = new TRequestWork_Logout(pkg, conn->term_session_, psm_context_);
    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddHeartbeatWork( TermConnection *conn, PtHeartbeatRequest *pkg )
{
    TRequestWork_Heartbeat *work = new TRequestWork_Heartbeat(pkg, conn->term_session_, psm_context_);
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
                psm_context_->logger_.Warn("[终端业务申请请求][CAID=" SFMT64U "][SID=" SFMT64U "] 申请呈现端的会话[SID=" SFMT64U "]不存在。",  
                                            conn->term_session_->CAId(), conn->term_session_->Id(),
                                            pkg->svc_cross_apply_desc_.show_session_id_);    

                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }
        else
        {
            psm_context_->logger_.Warn("[终端业务申请请求][CAID=" SFMT64U "][SID=" SFMT64U "] 申请参数不合法。",  
                                        conn->term_session_->CAId(), conn->term_session_->Id());    

            ret_code = PT_RC_MSG_FORMAT_ERROR;
            break;
        } 

        svcapply_work->user_ptr_    = psm_context_;
        svcapply_work->work_func_   = TermSvcApplyWork::Func_Begin;

        psm_context_->busi_pool_->AddWork(svcapply_work, conn->term_session_->CAId());
        return;
    } while (0);
    
    // 参数错误，直接给终端应答

    PtSvcApplyResponse svcapply_response(ret_code, 0);
    ByteStream response_pkg = svcapply_response.Serialize();

    psm_context_->logger_.Warn("[终端业务申请请求][CAID=" SFMT64U "][SID=" SFMT64U "] 处理业务申请请求失败，向终端发送失败应答。长度：%d  内容：\n%s",  
                                conn->term_session_->CAId(), conn->term_session_->Id(),
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(),response_pkg.Size()).c_str());    
        
    if ( conn->Write(response_pkg.GetBuffer(), response_pkg.Size()) )
    {
        //TODO:暂时默认发送成功
    }

    delete pkg;
}

void TermRequestProcessSvr::AddStatusQueryWork( TermConnection *conn, PtStatusQueryRequest *pkg )
{
    TRequestWork_StatusQuery *work = new TRequestWork_StatusQuery(pkg, conn->term_session_, psm_context_);
    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}

void TermRequestProcessSvr::AddGetSvrGroupWork( TermConnection *conn, PtGetSvcGroupRequest *pkg )
{
    TRequestWork_GetSvcGroup *work = new TRequestWork_GetSvcGroup(pkg, conn->term_session_, psm_context_);
    psm_context_->busi_pool_->AddWork(work, conn->term_session_->CAId());
}
