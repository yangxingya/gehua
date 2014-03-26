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
    void AddKeyTransmitWork(weak_ptr<TermSession> ts, PtKeyMappingRequest *pkg);

    /**
     * @brief 添加一个业务切换通知工作任务
     */
    void AddSvcSwitchNotifyWork(weak_ptr<TermSession> ts, PtSvcSwitchRequest *pkg);
    void AddSvcSwitchNotifyWork(weak_ptr<TermSession> ts, PtSvcSwitchResponse *pkg);

    /**
     * @brief 添加一个状态变更通知工作任务
     */
    void AddStatusPChangeNotifyWork(weak_ptr<TermSession> ts, PtStatusNotifyRequest *pkg);
    void AddStatusPChangeNotifyWork(weak_ptr<TermSession> ts, PtStatusNotifyResponse *pkg);

    void NotifyAllTerminalStatusPChanged(CASession *ca_session, uint64_t ignore_session_id);

private:
    PSMContext *psm_context_;
};

#endif