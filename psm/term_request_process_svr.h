#ifndef TERM_REQUEST_PROCESS_SERVER_H_
#define TERM_REQUEST_PROCESS_SERVER_H_

#include "work_def.h"
#include "./tcpserver/termconnection.h"

struct PSMContext;

/*
 *	brief �ն������������
 */
class TermRequestProcessSvr
{
public:
    TermRequestProcessSvr(PSMContext* psm_context);
    ~TermRequestProcessSvr();

    /**
     * @brief ���һ���ն˵�¼����������
     */
    void AddLoginRequestWork(weak_ptr<TermSession> ts, PtLoginRequest *pkg);

    /**
     * @brief ���һ���ն��˳�����������
     */
    void AddLogoutRequestWork(weak_ptr<TermSession> ts, PtLogoutRequest *pkg);

    /**
     * @brief ���һ���ն���������������
     */
    void AddHeartbeatWork(weak_ptr<TermSession> ts, PtHeartbeatRequest *pkg);
    
    /**
     * @brief ���һ���ն�ҵ����������������
     */
    void AddSvcApplyWork(weak_ptr<TermSession> ts, PtSvcApplyRequest *pkg);
    
    /**
     * @brief ���һ��ҵ��״̬��ѯ����������
     */
    void AddStatusQueryWork(weak_ptr<TermSession> ts, PtStatusQueryRequest *pkg);
    
    /**
     * @brief ���һ����ȡ���������������������
     */
    void AddGetSvrGroupWork(weak_ptr<TermSession> ts, PtGetSvcGroupRequest *pkg);

protected:

    void GetUserInfoParam(string &user_info_str, map<string, string> &out_param_map);
private:

    PSMContext  *psm_context_;
};

#endif 