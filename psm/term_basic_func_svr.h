#ifndef TERM_BASIC_FUNC_SERVER_H_
#define TERM_BASIC_FUNC_SERVER_H_

#include "work_def.h"
#include "./tcpserver/termconnection.h"

struct PSMContext;

/*
 *	brief �ն˻������ܷ�����
 */
class TermBasicFuncSvr
{
public:
    TermBasicFuncSvr(PSMContext *psm_context);
    ~TermBasicFuncSvr();

    /**
     * @brief ���һ����ֵת����������
     */
    void AddKeyTransmitWork(weak_ptr<TermSession> ts, PtKeyMappingRequest *pkg);

    /**
     * @brief ���һ��ҵ���л�֪ͨ��������
     */
    void AddSvcSwitchNotifyWork(weak_ptr<TermSession> ts, PtSvcSwitchRequest *pkg);
    void AddSvcSwitchNotifyWork(weak_ptr<TermSession> ts, PtSvcSwitchResponse *pkg);

    /**
     * @brief ���һ��״̬���֪ͨ��������
     */
    void AddStatusPChangeNotifyWork(weak_ptr<TermSession> ts, PtStatusNotifyRequest *pkg);
    void AddStatusPChangeNotifyWork(weak_ptr<TermSession> ts, PtStatusNotifyResponse *pkg);

    void NotifyAllTerminalStatusPChanged(CASession *ca_session, uint64_t ignore_session_id);

private:
    PSMContext *psm_context_;
};

#endif