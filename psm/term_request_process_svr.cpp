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

void TermRequestProcessSvr::AddLoginRequestWork( weak_ptr<TermSession> ts, PtLoginRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[�ն˵�¼����]******�ն˻Ự������******");
        return;
    }

    TRequestWork_Login *work = new TRequestWork_Login(pkg, sp_ts, psm_context_);
    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}

void TermRequestProcessSvr::AddLoginRequestWork( caid_t caid, PtLoginRequest *pkg )
{
    TRequestWork_Login *work = new TRequestWork_Login(pkg, caid, psm_context_);
    psm_context_->busi_pool_->AddWork(work, caid);
}

void TermRequestProcessSvr::AddLogoutRequestWork( weak_ptr<TermSession> ts, PtLogoutRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[�ն��˳�����]******�ն˻Ự������******");
        return;
    }

    TRequestWork_Logout *work = new TRequestWork_Logout(pkg, sp_ts, psm_context_);
    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}

void TermRequestProcessSvr::AddHeartbeatWork( weak_ptr<TermSession> ts, PtHeartbeatRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[�ն���������]******�ն˻Ự������******");
        return;
    }

    TRequestWork_Heartbeat *work = new TRequestWork_Heartbeat(pkg, sp_ts, psm_context_);
    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}

void TermRequestProcessSvr::AddSvcApplyWork( weak_ptr<TermSession> ts, PtSvcApplyRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[�ն�ҵ����������]******�ն˻Ự������******");
        return;
    }

    unsigned int ret_code = PT_RC_MSG_FORMAT_ERROR;
    do 
    {
        TermSvcApplyWork *svcapply_work = NULL;
        if ( pkg->svc_self_apply_desc_.valid_ && (sp_ts->Id() == pkg->svc_self_apply_desc_.session_id_) )
        {
            svcapply_work = new TermSvcApplyWork(pkg, sp_ts);
        }
        else if ( pkg->svc_cross_apply_desc_.valid_ && (sp_ts->Id() == pkg->svc_cross_apply_desc_.init_session_id_) )
        {
            weak_ptr<TermSession> wp_show_term_session = psm_context_->busi_pool_->FindTermSessionById(pkg->svc_cross_apply_desc_.show_session_id_);
            shared_ptr<TermSession> show_term_session(wp_show_term_session.lock());
            if ( show_term_session != NULL )
            {
                svcapply_work = new TermSvcApplyWork(pkg, sp_ts, show_term_session);
            }
            else
            {
                psm_context_->logger_.Warn("[�ն�ҵ����������][CAID=" SFMT64U "][SID=0x" SFMT64X "] ������ֶ˵ĻỰ[SID=0x" SFMT64X "]�����ڡ�",  
                                            sp_ts->CAId(), sp_ts->Id(),
                                            pkg->svc_cross_apply_desc_.show_session_id_);    

                ret_code = PT_RC_MSG_FORMAT_ERROR;
                break;
            }
        }
        else
        {
			uint64_t pkt_sid = pkg->svc_self_apply_desc_.session_id_;
			if (pkg->svc_cross_apply_desc_.valid_)
				pkt_sid = pkg->svc_cross_apply_desc_.show_session_id_;

            psm_context_->logger_.Warn("[�ն�ҵ����������][CAID=" SFMT64U "][SID=0x" SFMT64X "] ����������Ϸ�,������SID=0x"SFMT64X"",  
                                        sp_ts->CAId(), sp_ts->Id(), pkt_sid);    

            ret_code = PT_RC_MSG_FORMAT_ERROR;
            break;
        } 

        svcapply_work->user_ptr_    = psm_context_;
        svcapply_work->work_func_   = TermSvcApplyWork::Func_Begin;

        psm_context_->busi_pool_->AddWork(svcapply_work, sp_ts->CAId());
        return;
    } while (0);
    
    // ��������ֱ�Ӹ��ն�Ӧ��

    PtSvcApplyResponse svcapply_response(ret_code, 0);
    ByteStream response_pkg = svcapply_response.Serialize();

    psm_context_->logger_.Warn("[�ն�ҵ����������][CAID=" SFMT64U "][SID=" SFMT64X "] ����ҵ����������ʧ�ܣ����ն˷���ʧ��Ӧ�𡣳��ȣ�%d  ���ݣ�\n%s",  
                                sp_ts->CAId(), sp_ts->Id(),
                                response_pkg.Size(),
                                stringtool::to_hex_string((const char*)response_pkg.GetBuffer(),response_pkg.Size()).c_str());    
     
	bool writed = false;
	{
		MutexLock lock(sp_ts->termconn_mtx_);
		if (sp_ts->term_conn_)
			writed = sp_ts->term_conn_->Write(response_pkg.GetBuffer(), response_pkg.Size());
	}
    if (writed)
    {
        //TODO:��ʱĬ�Ϸ��ͳɹ�
    }

    delete pkg;
}

void TermRequestProcessSvr::AddStatusQueryWork( weak_ptr<TermSession> ts, PtStatusQueryRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[״̬��ѯ����]******�ն˻Ự������******");
        return;
    }

    TRequestWork_StatusQuery *work = new TRequestWork_StatusQuery(pkg, sp_ts, psm_context_);
    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}


void TermRequestProcessSvr::AddGetSvrGroupWork( weak_ptr<TermSession> ts, PtGetSvcGroupRequest *pkg )
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) {
        psm_context_->logger_.Warn("[��ȡ�����������]******�ն˻Ự������******");
        return;
    }

    TRequestWork_GetSvcGroup *work = new TRequestWork_GetSvcGroup(pkg, sp_ts, psm_context_);
    psm_context_->busi_pool_->AddWork(work, sp_ts->CAId());
}
