#ifndef WORK_DEF_H_
#define WORK_DEF_H_

#include <cpplib/workqueue.h>
#include <cpplib/stringtool.h>
#include <cpplib/timetool.h>
#include <protocol/protocol_v2_pb_message.h>
#include <protocol/protocol_v2_pt_message.h>
#include "tcpserver/busiconnection.h"
#include "tcpserver/termconnection.h"
#include "businesslogic/businesspool.h"
#include "work_svc_apply.h"

#define  RC_SUCCESS                    0x00000000	//成功

#define  PT_RC_MSG_FORMAT_ERROR        0x81010001	//消息格式验证失败
#define  PT_RC_MSG_MAC_ERROR           0x81010002	//消息MAC验证失败
#define  PT_RC_MSG_PASSWORD_ERROR      0x81010003	//消息MAC/用户密码验证失败
#define  PT_RC_MSG_ID_ERROR            0x81010004	//消息ID不支持
#define  PT_RC_ODCLIB_ERROR            0x81010021	//移植库版本不支持
#define  PT_RC_ODCLIB_EXPIRED          0x81010022	//移植库已过期
#define  PT_RC_USERINFO_FORMAT_ERROR   0x81010023	//用户信息格式不支持
#define  PT_RC_USER_CERT_ERROR         0x81010024	//用户证书无效
#define  PT_RC_USER_CERT_EXPIRED       0x81010025	//用户证书已过期
#define  PT_RC_TERM_TYPE_ERROR         0x81010026	//终端类型不支持
#define  PT_RC_TERM_APPLY_SVC_ERROR    0x81010027	//终端申请的业务不支持
#define  PT_RC_TERM_APPLY_SVC_INVALID  0x81010028	//终端申请的业务目标已失效
#define  PT_RC_TERM_SESSION_NOT_EXIST  0x81010029   //终端会话不存在

#define  ST_RC_TERM_APPLY_SVC_ERROR    0x81020021	//终端申请的业务不支持
#define  ST_RC_TERM_APPLY_SVC_INVALID  0x81020022	//终端申请的业务目标已失效
#define  ST_RC_APPLY_SVC_URL_ERROR     0x81020023	//申请的业务URL无效
#define  ST_RC_TERM_SVC_INIT_ERROR     0x81020024	//业务初始化失败

#define  SP_RC_TERM_APPLY_SVC_ERROR    0xA1020021	//终端申请的业务不支持
#define  SP_RC_TERM_APPLY_SVC_INVALID  0xA1020022	//终端申请的业务目标已失效
#define  SP_RC_APPLY_SVC_URL_ERROR     0xA1020023	//申请的业务URL无效
#define  SP_RC_TERM_SVC_INIT_ERROR     0xA1020024	//业务初始化失败

#define  PS_RC_MSG_FORMAT_ERROR        0xA1010001	//消息格式验证失败
#define  PS_RC_MSG_ID_ERROR            0xA1010002	//消息ID不支持
#define  PS_RC_SEQUENCENO_ERROR        0xA10A0021	//交易流水号错误


/**
 * @brief  处理器的工作项基础结构
 */
struct PSMWork : public DelayedWork
{
    double                work_start_time_;
    weak_ptr<TermSession> session_info_;

    string                work_name_;
    char                  log_header_[300]; 


    PSMWork()
    {
        work_start_time_ = timetool::get_up_time();
    }
};

/*************************************************************************/
/***************************以下为终端相关工作项定义** *******************/
/*************************************************************************/


/***************************用户登录请求处理工作任务**********************************/

struct TRequestWork_Login : public PSMWork
{
    PtLoginRequest *pkg_;

    enum LoginRunStep{
        Login_Begin = 0,
        Login_SendResponse,
        Login_NotifyStatusChanged,
        Login_End
    } run_step_;

    TRequestWork_Login(PtLoginRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context);
    ~TRequestWork_Login();

    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************用户退出请求处理工作任务**********************************/

struct TRequestWork_Logout : public PSMWork
{
    PtLogoutRequest  *pkg_;

    enum LogoutRunStep{
        Logout_Begin = 0,
        Logout_ReleaseSession,
        Logout_SendResponse,
        Logout_End
    } run_step_;

    TRequestWork_Logout(PtLogoutRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context);
    ~TRequestWork_Logout();

    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************终端心跳请求处理工作任务**********************************/

struct TRequestWork_Heartbeat : public PSMWork
{
    PtHeartbeatRequest  *pkg_;

    enum HeartbeatRunStep{
        Heartbeat_Begin = 0,
        Heartbeat_SendResponse,
        Heartbeat_End
    } run_step_;

    TRequestWork_Heartbeat(PtHeartbeatRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context);
    ~TRequestWork_Heartbeat();


    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************终端状态查询请求处理工作任务**********************************/

struct TRequestWork_StatusQuery : public PSMWork
{
    PtStatusQueryRequest  *pkg_;

    enum StatusQueryRunStep{
        StatusQuery_Begin = 0,
        StatusQuery_SendResponse,
        StatusQuery_End
    } run_step_;

    TRequestWork_StatusQuery(PtStatusQueryRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context);
    ~TRequestWork_StatusQuery();

    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************终端获取服务分组请求处理工作任务**********************************/

struct TRequestWork_GetSvcGroup : public PSMWork
{
    PtGetSvcGroupRequest  *pkg_;

    enum GetSvcGroupRunStep{
        GetSvcGroup_Begin = 0,
        GetSvcGroup_SendResponse,
        GetSvcGroup_End
    } run_step_;

    TRequestWork_GetSvcGroup(PtGetSvcGroupRequest *pkg, weak_ptr<TermSession> session_info, void *psm_context);
    ~TRequestWork_GetSvcGroup();

    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************终端键值映射请求处理工作任务**********************************/

struct TRequestWork_KeyMapping : public PSMWork
{
    PtKeyMappingRequest  *pkg_;

    enum KeyMappingRunStep{
        KeyMapping_Begin = 0,
        KeyMapping_GetTargetInfo,
        KepMapping_Transpond,
        KeyMapping_End
    } run_step_;

    TRequestWork_KeyMapping()
    {
        pkg_ = NULL;
        run_step_ = KeyMapping_Begin;
    }
    ~TRequestWork_KeyMapping()
    {
        if ( pkg_ != NULL )  delete pkg_;
    }
    
    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************终端业务切换通知处理工作任务**********************************/

struct TNotifyWork_SvcSwitch : public PSMWork
{
    PtSvcSwitchRequest  *pkg_;

    enum SvcSwitchRunStep{
        SvcSwitch_Begin = 0,
        SvcSwitch_SendNotify,
        SvcSwitch_End
    } run_step_;

    bool recv_responed_sucess_;

    TNotifyWork_SvcSwitch()
    {
        pkg_ = NULL;
        recv_responed_sucess_ = false;
        run_step_ = SvcSwitch_Begin;
    }
    ~TNotifyWork_SvcSwitch()
    {
        if ( pkg_ != NULL )  delete pkg_;
    }

    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};

/***************************终端状态变更通知处理工作任务**********************************/

struct TNotifyWork_StatusNotify : public PSMWork
{
    PtStatusNotifyRequest  *pkg_;

    enum StatusNotifyRunStep{
        StatusNotify_Begin = 0,
        StatusNotify_SendNotify,
        StatusNotify_End
    } run_step_;

    bool recv_responed_sucess_;

    TNotifyWork_StatusNotify()
    {
        pkg_ = NULL;
        recv_responed_sucess_ = false;
        run_step_ = StatusNotify_Begin;
    }
    ~TNotifyWork_StatusNotify()
    {
        if ( pkg_ != NULL )  delete pkg_;
    }

    static void Func_Begin(Work *work);
    static void Func_End(Work *work);
};


/*************************************************************************/
/***************************以下为SM相关工作项定义************************/
/*************************************************************************/


/***************************SM业务状态同步请求处理工作任务**********************************/

struct SMRequestWork_TermSync : public PSMWork
{
    PbTermSyncRequest  *pkg_;
    BusiConnection  *busi_conn_;

    enum TermSyncRunStep{
        TermSync_Begin = 0,
        TermSync_Sync,
        TermSync_SendResponse,
        TermSync_End
    } run_step_;    

    SMRequestWork_TermSync()
    {
        pkg_        = NULL;
        busi_conn_  = NULL;
        run_step_   = TermSync_Begin;
    }
    ~SMRequestWork_TermSync()
    {
        if ( pkg_ != NULL )  delete pkg_;
    }

    static void Func_Begin(Work *work);
    static void Func_Sync(Work *work);
    static void Func_SendResponse(Work *work);
    static void Func_End(Work *work);
};

/***************************SM业务状态汇报请求处理工作任务**********************************/

struct SMRequestWork_TermReport : public PSMWork
{
    PbTermReportRequest  *pkg_;
    BusiConnection  *busi_conn_;

    enum TermReportRunStep{
        TermReport_Begin = 0,
        TermReport_UpdateStatus,
        TermReport_SendResponse,
        TermReport_End
    } run_step_;

    SMRequestWork_TermReport()
    {
        pkg_        = NULL;
        busi_conn_  = NULL;
        run_step_   = TermReport_Begin;
    }
    ~SMRequestWork_TermReport()
    {
        if ( pkg_ != NULL )  delete pkg_;
    }

    static void Func_Begin(Work *work);
    static void Func_UpdateStatus(Work *work);
    static void Func_SendResponse(Work *work);
    static void Func_End(Work *work);
};

/***************************SM业务参数通知请求处理工作任务**********************************/

struct SMRequestWork_SvcPChange : public PSMWork
{
    PbSvcPChangeRequest  *pkg_;
    BusiConnection  *busi_conn_;

    enum SvcPChangeRunStep{
        SvcPChange_Begin = 0,
        SvcPChange_UpdateParam,
        SvcPChange_SendResponse,
        SvcPChange_End
    } run_step_;

    SMRequestWork_SvcPChange()
    {
        pkg_        = NULL;
        busi_conn_  = NULL;
        run_step_   = SvcPChange_Begin;
    }
    ~SMRequestWork_SvcPChange()
    {
        if ( pkg_ != NULL )  delete pkg_;
    }

    static void Func_Begin(Work *work);
    static void Func_UpdateParam(Work *work);
    static void Func_SendResponse(Work *work);
    static void Func_End(Work *work);
};

#endif