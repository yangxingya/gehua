#include "work_def.h"
#include "psmcontext.h"
#include "./sessionmgr/casession.h"
#include "./sessionmgr/termsession.h"


TRequestWork_Login::TRequestWork_Login( PtLoginRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context )
{
    pkg_          = pkg;
    run_step_     = Login_Begin;
    session_info_ = session_info;
    work_func_    = TRequestWork_Login::Func_Begin;
    user_ptr_     = psm_context;

    work_name_    = "终端登录请求";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

TRequestWork_Login::TRequestWork_Login( PtLoginRequest *pkg, TermConnection *tconn, void *psm_context )
{
    pkg_          = pkg;
    tconn_        = tconn;
    run_step_     = Login_Begin;
    work_func_    = TRequestWork_Login::Func_Begin;
    user_ptr_     = psm_context;

    work_name_    = "终端登录请求";
    _snprintf(log_header_, 300, "[%s][CAID=no valided][Self_SID=no valided]", work_name_.c_str());
}

TRequestWork_Login::~TRequestWork_Login()
{
    if ( pkg_ != NULL )  delete pkg_;
}

TRequestWork_Logout::TRequestWork_Logout( PtLogoutRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context )
{
    pkg_          = pkg;
    run_step_     = Logout_Begin;
    session_info_ = session_info;
    work_func_    = TRequestWork_Logout::Func_Begin;
    user_ptr_     = psm_context;

    work_name_    = "终端退出请求";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

TRequestWork_Logout::~TRequestWork_Logout()
{
    if ( pkg_ != NULL )  delete pkg_;
}

TRequestWork_Heartbeat::TRequestWork_Heartbeat( PtHeartbeatRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context )
{
    pkg_              = pkg;
    run_step_         = Heartbeat_Begin;
    session_info_     = session_info;
    work_func_        = TRequestWork_Heartbeat::Func_Begin;
    user_ptr_         = psm_context;

    work_name_    = "终端心跳";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

TRequestWork_Heartbeat::~TRequestWork_Heartbeat()
{
    if ( pkg_ != NULL )  delete pkg_;
}

TRequestWork_StatusQuery::TRequestWork_StatusQuery( PtStatusQueryRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context )
{
    pkg_              = pkg;
    run_step_         = StatusQuery_Begin;
    session_info_     = session_info;
    work_func_        = TRequestWork_StatusQuery::Func_Begin;
    user_ptr_         = psm_context;

    work_name_    = "终端状态查询请求";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

TRequestWork_StatusQuery::~TRequestWork_StatusQuery()
{
    if ( pkg_ != NULL )  delete pkg_;
}

TRequestWork_GetSvcGroup::TRequestWork_GetSvcGroup( PtGetSvcGroupRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context )
{
    pkg_              = pkg;
    run_step_         = GetSvcGroup_Begin;
    session_info_     = session_info;
    work_func_        = TRequestWork_GetSvcGroup::Func_Begin;
    user_ptr_         = psm_context;

    work_name_    = "终端获取服务分组请求";
    //提升引用。
    shared_ptr<TermSession> sp_session_info(session_info_.lock());
    if (!sp_session_info)
        _snprintf(log_header_, 300, "[%s][****被删除的会话****]", work_name_.c_str());
    else 
        _snprintf(log_header_, 300, "[%s][CAID=" SFMT64U "][Self_SID=0x" SFMT64X "]", work_name_.c_str(), sp_session_info->CAId(), sp_session_info->Id());
}

TRequestWork_GetSvcGroup::~TRequestWork_GetSvcGroup()
{
    if ( pkg_ != NULL )  delete pkg_;
}
