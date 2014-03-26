#ifndef TERM_REQUEST_PROCESS_SERVER_H_
#define TERM_REQUEST_PROCESS_SERVER_H_

#include "work_def.h"
#include "./tcpserver/termconnection.h"

struct PSMContext;

/*
 *	brief 终端请求处理服务类
 */
class TermRequestProcessSvr
{
public:
    TermRequestProcessSvr(PSMContext* psm_context);
    ~TermRequestProcessSvr();

    /**
     * @brief 添加一个终端登录请求工作任务
     */
    void AddLoginRequestWork(weak_ptr<TermSession> ts, PtLoginRequest *pkg);

    /**
     * @brief 添加一个终端退出请求工作任务
     */
    void AddLogoutRequestWork(weak_ptr<TermSession> ts, PtLogoutRequest *pkg);

    /**
     * @brief 添加一个终端心跳请求工作任务
     */
    void AddHeartbeatWork(weak_ptr<TermSession> ts, PtHeartbeatRequest *pkg);
    
    /**
     * @brief 添加一个终端业务申请请求工作任务
     */
    void AddSvcApplyWork(weak_ptr<TermSession> ts, PtSvcApplyRequest *pkg);
    
    /**
     * @brief 添加一个业务状态查询请求工作任务
     */
    void AddStatusQueryWork(weak_ptr<TermSession> ts, PtStatusQueryRequest *pkg);
    
    /**
     * @brief 添加一个获取服务分组申请请求工作任务
     */
    void AddGetSvrGroupWork(weak_ptr<TermSession> ts, PtGetSvcGroupRequest *pkg);

protected:

    void GetUserInfoParam(string &user_info_str, map<string, string> &out_param_map);
private:

    PSMContext  *psm_context_;
};

#endif 