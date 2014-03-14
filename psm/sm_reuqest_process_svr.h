#ifndef SM_REQUEST_PROCESS_SERVER_H_
#define SM_REQUEST_PROCESS_SERVER_H_

#include "work_def.h"
#include "tcpserver/busiconnection.h"

struct PSMContext;

/*
 *	brief SM请求处理服务类
 */
class SMRequestProcessSvr
{
public:
    SMRequestProcessSvr(PSMContext* psm_context);
    ~SMRequestProcessSvr();

    /**
     * @brief 添加一个SM业务业务状态同步请求工作任务
     */
    void AddTermSyncWork(BusiConnection *conn, PbTermSyncRequest *pkg);

    /**
     * @brief 添加一个SM业务参数通知请求工作任务
     */
    void AddSvcPChangeWork(BusiConnection *conn, PbSvcPChangeRequest *pkg);

    /**
     * @brief 添加一个SM业务状态汇报请求工作任务
     */
    void AddTermReportWork(BusiConnection *conn, PbTermReportRequest *pkg);

    /**
     * @brief 添加一个SM终端业务申请请求工作任务
     */
    void AddSvcApplyWork(BusiConnection *conn, PbSvcApplyRequest *pkg);  

protected:
    int UpdateTermStatus(list<PB_TerminalStatusDescriptor> &termstatus_desc_list);
    int UpdateTermParam(PbSvcPChangeRequest *pkg);

private:
    PSMContext  *psm_context_;
};

#endif