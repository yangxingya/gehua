#ifndef WORK_SVC_APPLY_H_
#define WORK_SVC_APPLY_H_

#include <cpplib/timetool.h>
#include <cpplib/workqueue.h>
#include <protocol/protocol_v2_pb_message.h>
#include <protocol/protocol_v2_pt_message.h>
#include "businesslogic/businesspool.h"
#include "tcpserver/busiconnection.h"
#include "tcpserver/termconnection.h"


/**
 * @brief  HTTP请求信息
 */
struct HTTPAysnRequestInfo
{
public:
    void SetRequestURL(string &sm_addr, string &card_id);
    void SetRequestBody(ByteStream &svc_self_apply_desc, ByteStream &user_info_desc, ByteStream &terminal_info_desc, string &psm_addr);
    void SetRequestBody(ByteStream &svc_cross_apply_desc,ByteStream &user_info_desc, ByteStream &terminal_info_desc, ByteStream &user_info_desc2, ByteStream &terminal_info_desc2, string &psm_addr);
 
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

    SvcApplyRunStep             run_step_;
    bool                        apply_sucess_;
    AioConnection               *conn_;

    SvcApplyType                apply_type_;
    HTTPAysnRequestInfo         http_request_info_;
    weak_ptr<TermSession>       cross_session_info_;
    weak_ptr<TermSession>       self_session_info_;
    string                      work_name_;
    char                        log_header_[300]; 
    
    string  GetServiceName(const char *url);
    int     AddSendHttpRequestWork(string &service_name, ByteStream &svcapply_desc_buf);
    int     ParseHttpResponse(ByteStream &response_body, vector<PB_SvcURLDescriptor> &svc_url_desc_list, PB_KeyMapIndicateDescriptor &keymaping_indicate_desc);
};

/***************************终端业务申请请求处理工作任务**********************************/

/**
 * @brief  终端业务申请工作项定义
 */
struct TermSvcApplyWork : public SvcApplyWork
{
    TermSvcApplyWork(AioConnection *conn, PtSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info);
    TermSvcApplyWork(AioConnection *conn, PtSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info, weak_ptr<TermSession> cross_session_info);
    virtual ~TermSvcApplyWork();

    int SendResponed(ByteStream &response_buf);

    static void Func_Begin(Work *work);
    static void Func_Inited(Work *work);
    static void Func_End(Work *work);

public:
    PtSvcApplyRequest *pkg_;
};

/***************************SM终端业务申请请求处理工作任务**********************************/

/**
 * @brief  SM终端业务申请工作项定义
 */
struct SMSvcApplyWork : public SvcApplyWork
{
    SMSvcApplyWork(AioConnection *conn, PbSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info);

    SMSvcApplyWork(AioConnection *conn, PbSvcApplyRequest *pkg, weak_ptr<TermSession> self_session_info, weak_ptr<TermSession> cross_session_info);
    virtual ~SMSvcApplyWork();

    int SendResponed(unsigned int ret_code);

    // 当SM申请的业务为本地业务时，则调用该接口向终端发送业务切换通知
    void AddNotifySvcSwitchWork();

    static void Func_Begin(Work *work);
    static void Func_Inited( Work *work );
    static void Func_End(Work *work);

public:
    PbSvcApplyRequest *pkg_;
};

map<string, vector<string>> SplitString2MapList(const char *szString, char cGap1, char cGap2);

#endif