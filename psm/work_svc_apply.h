#ifndef WORK_SVC_APPLY_H_
#define WORK_SVC_APPLY_H_

#include "cpplib/workqueue.h"
#include "protocol/protocol_v2_pb_message.h"
#include "protocol/protocol_v2_pt_message.h"
#include "businesslogic/businesspool.h"
#include "cpplib/timetool.h"
#include "tcpserver/busiconnection.h"
#include "tcpserver/termconnection.h"


/**
 * @brief  HTTP请求信息
 */
struct HTTPAysnRequestInfo
{
public:
    void SetRequestURL(string &sm_addr, string &card_id)
    {
        ByteStream bs;
        bs.Add("http://");
        bs.Add(sm_addr);
        bs.Add("?userid=");
        bs.Add(card_id);
        bs.Add("&req=PSM_SERVICE_INIT_REQUEST");

        request_url_ = bs;

        sm_addr_ = sm_addr;
    }

    void SetRequestBody(ByteStream &svc_self_apply_desc, 
        ByteStream &user_info_desc, 
        ByteStream &terminal_info_desc,
        string &psm_addr)
    {
        ByteStream bs;
        ByteStream tmp_bs;

        bs.Add("\r\nSvcApply=");
        bs.Add(svc_self_apply_desc.DumpHex(svc_self_apply_desc.GetWritePtr(), false));

        bs.Add("\r\nUserInfo=");
        bs.Add(user_info_desc.DumpHex(user_info_desc.GetWritePtr(), false));

        bs.Add("\r\nTerminalInfo=");
        bs.Add(terminal_info_desc.DumpHex(terminal_info_desc.GetWritePtr(), false));

        bs.Add("\r\nPsm=");
        bs.Add(psm_addr);

        bs.Add("\r\n\0");

        request_body_ = bs;
    }

    void SetRequestBody(ByteStream &svc_cross_apply_desc, 
        ByteStream &user_info_desc, 
        ByteStream &terminal_info_desc,
        ByteStream &user_info_desc2, 
        ByteStream &terminal_info_desc2,
        string &psm_addr)
    {
        ByteStream bs;

        bs.Add("\r\nSvcApply=");
        bs.Add(svc_cross_apply_desc.DumpHex(svc_cross_apply_desc.GetWritePtr(), false));

        bs.Add("\r\nUserInfo=");
        bs.Add(user_info_desc.DumpHex(user_info_desc.GetWritePtr(), false));

        bs.Add("\r\nTerminalInfo=");
        bs.Add(terminal_info_desc.DumpHex(terminal_info_desc.GetWritePtr(), false));

        bs.Add("\r\nUserInfo2nd=");
        bs.Add(user_info_desc2.DumpHex(user_info_desc2.GetWritePtr(), false));

        bs.Add("\r\nTerminalInfo2nd=");
        bs.Add(terminal_info_desc2.DumpHex(terminal_info_desc2.GetWritePtr(), false));

        bs.Add("\r\nPsm=");
        bs.Add(psm_addr);

        bs.Add("\r\n\0");

        request_body_ = bs;
    }
 
public:
    ByteStream       request_url_;
    ByteStream       request_body_;
    ByteStream       responed_body_;

    string           sm_addr_;  // IP:Port

    enum RequestResult{
        OK = 0,
        ConnectFailed,
        SendFailed,
        RecvResponedFailed
    };
    RequestResult   request_result_;
};

/**
 * @brief  业务申请工作项定义
 */
struct SvcApplyWork : public Work
{
    enum SvcApplyRunStep{
        SvcApply_Begin = 0,
        SvcApply_Init_begin,
        SvcApply_Init_end,
        SvcApply_SvcSwitchNofity,
        SvcApply_SendResponse,
        SvcApply_End
    }; 

    enum SvcApplyType{
        SelfSvcApply = 0,
        CorssSvcApply
    };

    SvcApplyType                apply_type_;
    SvcApplyRunStep             run_step_;
    TermSession                 *cross_session_info_;
    TermSession                 *self_session_info_;
    HTTPAysnRequestInfo         http_request_info_;
    bool                        apply_sucess_;
    AioConnection               *conn_;
};

/**
 * @brief  终端业务申请工作项定义
 */
struct TermSvcApplyWork : public SvcApplyWork
{
    TermSvcApplyWork(AioConnection *conn, 
                 PtSvcApplyRequest *pkg, 
                 TermSession *self_session_info);

    TermSvcApplyWork(AioConnection *conn, 
                 PtSvcApplyRequest *pkg, 
                 TermSession *self_session_info, 
                 TermSession *cross_session_info);

    static int SendErrorResponed(AioConnection *conn, unsigned int ret_code);

    virtual ~TermSvcApplyWork();

    static void Func_Begin(Work *work);
    static void Func_Inited(Work *work);
    static void Func_End(Work *work);

public:
    PtSvcApplyRequest           *pkg_;
};

/***************************SM终端业务申请请求处理工作任务**********************************/

/**
 * @brief  终端业务申请工作项定义
 */
struct SMSvcApplyWork : public SvcApplyWork
{
    SMSvcApplyWork(AioConnection *conn, 
                 PbSvcApplyRequest *pkg, 
                 TermSession *self_session_info);

    SMSvcApplyWork(AioConnection *conn, 
                 PbSvcApplyRequest *pkg, 
                 TermSession *self_session_info, 
                 TermSession *cross_session_info);

    static int SendResponed(AioConnection *conn, PbSvcApplyRequest *pkg, unsigned int ret_code);

    virtual ~SMSvcApplyWork();

    static void Func_Begin(Work *work);
    static void Func_Inited( Work *work );
    static void Func_End(Work *work);

public:
    PbSvcApplyRequest           *pkg_;
};

#endif