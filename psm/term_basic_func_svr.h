#ifndef TERM_BASIC_FUNC_SERVER_H_
#define TERM_BASIC_FUNC_SERVER_H_

#include "work_def.h"
#include "./tcpserver/termconnection.h"

struct PSMContext;

/*
 *	brief 终端基础功能服务类
 */
class TermBasicFuncSvr
{
public:
    TermBasicFuncSvr(PSMContext *psm_context);
    ~TermBasicFuncSvr();

    /**
     * @brief 添加一个键值转发工作任务
     */
    void AddKeyTransmitWork(TermConnection *conn, PtKeyMappingRequest *pkg);

    /**
     * @brief 添加一个业务切换通知工作任务
     */
    void AddSvcSwitchNotifyWork(TermConnection *conn, PtSvcSwitchRequest *pkg);
    void AddSvcSwitchNotifyWork(TermConnection *conn, PtSvcSwitchResponse *pkg);

    /**
     * @brief 添加一个状态变更通知工作任务
     */
    void AddStatusPChangeNotifyWork(TermConnection *conn, PtStatusNotifyRequest *pkg);
    void AddStatusPChangeNotifyWork(TermConnection *conn, PtStatusNotifyResponse *pkg);

    void NotifyAllTerminalStatusPChanged(CASession *ca_session, uint64_t ignore_session_id);

protected:
private:
    PSMContext *psm_context_;
};

#endif