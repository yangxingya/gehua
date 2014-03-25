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
    void AddKeyTransmitWork(TermConnection *conn, PtKeyMappingRequest *pkg);

    /**
     * @brief ���һ��ҵ���л�֪ͨ��������
     */
    void AddSvcSwitchNotifyWork(TermConnection *conn, PtSvcSwitchRequest *pkg);
    void AddSvcSwitchNotifyWork(TermConnection *conn, PtSvcSwitchResponse *pkg);

    /**
     * @brief ���һ��״̬���֪ͨ��������
     */
    void AddStatusPChangeNotifyWork(TermConnection *conn, PtStatusNotifyRequest *pkg);
    void AddStatusPChangeNotifyWork(TermConnection *conn, PtStatusNotifyResponse *pkg);

    void NotifyAllTerminalStatusPChanged(CASession *ca_session, uint64_t ignore_session_id);

protected:
private:
    PSMContext *psm_context_;
};

#endif