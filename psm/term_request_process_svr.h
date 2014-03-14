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
    void AddLoginRequestWork(TermConnection *conn, PtLoginRequest *pkg);

    /**
     * @brief 添加一个终端退出请求工作任务
     */
    void AddLogoutRequestWork(TermConnection *conn, PtLogoutRequest *pkg);

    /**
     * @brief 添加一个终端心跳请求工作任务
     */
    void AddHeartbeatWork(TermConnection *conn, PtHeartbeatRequest *pkg);
    
    /**
     * @brief 添加一个终端业务申请请求工作任务
     */
    void AddSvcApplyWork(TermConnection *conn, PtSvcApplyRequest *pkg);
    
    /**
     * @brief 添加一个业务状态查询请求工作任务
     */
    void AddStatusQueryWork(TermConnection *conn, PtStatusQueryRequest *pkg);
    
    /**
     * @brief 添加一个获取服务分组申请请求工作任务
     */
    void AddGetSvrGroupWork(TermConnection *conn, PtGetSvcGroupRequest *pkg);

protected:

    void GetUserInfoParam(string &user_info_str, map<string, string> &out_param_map);
private:

    PSMContext  *psm_context_;
};

#endif 