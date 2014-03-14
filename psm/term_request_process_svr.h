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
    void AddLoginRequestWork(TermConnection *conn, PtLoginRequest *pkg);

    /**
     * @brief ���һ���ն��˳�����������
     */
    void AddLogoutRequestWork(TermConnection *conn, PtLogoutRequest *pkg);

    /**
     * @brief ���һ���ն���������������
     */
    void AddHeartbeatWork(TermConnection *conn, PtHeartbeatRequest *pkg);
    
    /**
     * @brief ���һ���ն�ҵ����������������
     */
    void AddSvcApplyWork(TermConnection *conn, PtSvcApplyRequest *pkg);
    
    /**
     * @brief ���һ��ҵ��״̬��ѯ����������
     */
    void AddStatusQueryWork(TermConnection *conn, PtStatusQueryRequest *pkg);
    
    /**
     * @brief ���һ����ȡ���������������������
     */
    void AddGetSvrGroupWork(TermConnection *conn, PtGetSvcGroupRequest *pkg);

protected:

    void GetUserInfoParam(string &user_info_str, map<string, string> &out_param_map);
private:

    PSMContext  *psm_context_;
};

#endif 