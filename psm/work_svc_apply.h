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

    void SetRequestBody(PT_SvcSelfApplyDescriptor &svc_self_apply_desc, 
        PT_UserInfoDescriptor &user_info_desc, 
        PT_TerminalInfoDescriptor &terminal_info_desc,
        string &psm_addr)
    {
        ByteStream bs;
        ByteStream tmp_bs;

        bs.Add("\r\nSvcApply=");
        tmp_bs = svc_self_apply_desc.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nUserInfo=");
        tmp_bs = user_info_desc.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nTerminalInfo=");
        tmp_bs = terminal_info_desc.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nPsm=");
        bs.Add(psm_addr);

        bs.Add("\r\n\0");

        request_body_ = bs;
    }

    void SetRequestBody(PT_SvcCrossApplyDescriptor &svc_cross_apply_desc, 
        PT_UserInfoDescriptor &user_info_desc, 
        PT_TerminalInfoDescriptor &terminal_info_desc,
        PT_UserInfoDescriptor &user_info_desc2, 
        PT_TerminalInfoDescriptor &terminal_info_desc2,
        string &psm_addr)
    {
        ByteStream bs;
        ByteStream tmp_bs;

        bs.Add("\r\nSvcApply=");
        tmp_bs = svc_cross_apply_desc.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nUserInfo=");
        tmp_bs = user_info_desc.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nTerminalInfo=");
        tmp_bs = terminal_info_desc.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nUserInfo2nd=");
        tmp_bs = user_info_desc2.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

        bs.Add("\r\nTerminalInfo2nd=");
        tmp_bs = terminal_info_desc2.Serialize();
        bs.Add(tmp_bs.DumpHex(tmp_bs.GetWritePtr(), false));

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
    SvcApplyWork(AioConnection *conn, 
                 PtSvcApplyRequest *pkg, 
                 TermSession *self_session_info)
    {
        apply_type_             = SelfSvcApply;
        conn_                   = conn;
        pkg_                    = pkg;
        self_session_info_      = self_session_info;

        run_step_               = SvcApply_Begin;
        apply_sucess_           = false;

        work_func_              = SvcApplyWork::Func_Begin;
    }

    SvcApplyWork(AioConnection *conn, 
                 PtSvcApplyRequest *pkg, 
                 TermSession *self_session_info, 
                 TermSession *cross_session_info)
    {
        apply_type_             = CorssSvcApply;
        conn_                   = conn;
        pkg_                    = pkg;
        self_session_info_      = self_session_info;
        cross_session_info_     = cross_session_info;

        run_step_               = SvcApply_Begin;
        apply_sucess_           = false;

        work_func_              = SvcApplyWork::Func_Begin;
    }

    static int SendErrorResponed(AioConnection *conn, unsigned int ret_code)
    {
        // 参数错误，直接给终端应答
        PtSvcApplyResponse response(ret_code, 0);
        ByteStream response_pkg = response.Serialize();
        if ( conn->Write((unsigned char*)response_pkg.GetBuffer(), response_pkg.Size()) )
        {
            //TODO:
            return 0;
        }

        return -1;
    }

    virtual ~SvcApplyWork()
    {
        if ( pkg_ != NULL ) delete pkg_;
    }

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

    static void Func_Begin(Work *work);
    static void Func_Inited( Work *work );
    static void Func_SendResponse(Work *work);
    static void Func_End(Work *work);

public:
    SvcApplyType                apply_type_;
    SvcApplyRunStep             run_step_;
    TermSession                 *cross_session_info_;
    TermSession                 *self_session_info_;
    HTTPAysnRequestInfo         http_request_info_;
    bool                        apply_sucess_;
    AioConnection               *conn_;
    PtSvcApplyRequest           *pkg_;
};

/***************************SM终端业务申请请求处理工作任务**********************************/

/**
 * @brief  SM终端业务申请请求处理工作项定义
 */
// struct SMRequestWork_SvcApply : public SvcApplyWork
// {
//     PbSvcApplyRequest   *pkg_;
//     BusiConnection      *busi_conn_;
// 
//     SMRequestWork_SvcApply()
//     {
//         pkg_                = NULL;
//         busi_conn_          = NULL;
//     }
//     ~SMRequestWork_SvcApply()
//     {
//         if ( pkg_ != NULL )  delete pkg_;
//     }
// 
//     static void Func_Begin(Work *work);
//     static void Func_SendResponse(Work *work);
//     static void Func_End(Work *work);
// };


#endif