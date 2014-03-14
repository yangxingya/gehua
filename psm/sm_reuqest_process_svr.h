#ifndef SM_REQUEST_PROCESS_SERVER_H_
#define SM_REQUEST_PROCESS_SERVER_H_

#include "work_def.h"
#include "tcpserver/busiconnection.h"

struct PSMContext;

/*
 *	brief SM�����������
 */
class SMRequestProcessSvr
{
public:
    SMRequestProcessSvr(PSMContext* psm_context);
    ~SMRequestProcessSvr();

    /**
     * @brief ���һ��SMҵ��ҵ��״̬ͬ������������
     */
    void AddTermSyncWork(BusiConnection *conn, PbTermSyncRequest *pkg);

    /**
     * @brief ���һ��SMҵ�����֪ͨ����������
     */
    void AddSvcPChangeWork(BusiConnection *conn, PbSvcPChangeRequest *pkg);

    /**
     * @brief ���һ��SMҵ��״̬�㱨����������
     */
    void AddTermReportWork(BusiConnection *conn, PbTermReportRequest *pkg);

    /**
     * @brief ���һ��SM�ն�ҵ����������������
     */
    void AddSvcApplyWork(BusiConnection *conn, PbSvcApplyRequest *pkg);  

protected:
    int UpdateTermStatus(list<PB_TerminalStatusDescriptor> &termstatus_desc_list);
    int UpdateTermParam(PbSvcPChangeRequest *pkg);

private:
    PSMContext  *psm_context_;
};

#endif