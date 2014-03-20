/*
* @brief: terminal session.
*/

#if !defined gehua_sessionmgr_terminal_session_h_
#define gehua_sessionmgr_terminal_session_h_

#include <cpplib/logger.h>
#include <protocol/protocol_v2_common.h>
#include <protocol/protocol_v2_general.h>
#include <protocol/protocol_v2_cipher.h>
#include <protocol/protocol_v2_pt_common.h>
#include <protocol/protocol_v2_pt_descriptor.h>
#include <protocol/protocol_v2_pt.h>
#include <protocol/protocol_v2_pt_message.h>
#include <protocol/protocol_v2_pb_common.h>
#include <protocol/protocol_v2_pb_descriptor.h>
#include <protocol/protocol_v2_pb.h>
#include <protocol/protocol_v2_pb_message.h>
#include "../../common/widget.h"
#include "../bs-comm-def.h"
#include "../../common/auth/desc-common.h"

using ::std::stack;
using ::std::string;

struct UserInfo;
struct TermConnection;
struct PtLoginRequest;
struct CASession;
struct BusinessStatusInfo;
//struct PB_SvcURLDescriptor;

struct TermSession
{
public:
    TermSession(Logger &logger, PtLoginRequest *msg, TermConnection *tconn);
    bool valid() const { return valid_; }
public:
    CASession       *ca_session;
    Mutex            term_conn_mtx;
    TermConnection  *term_conn;

    BusinessType            curr_busi_type;
    BusinessType            last_busi_type;

    stack<string>          back_url_stack;
    PB_SvcURLDescriptor     curr_svc_url_desc;

    ServiceGroup    service_grp;
    UserInfo        user_info;

    PT_OdcLibDescriptor       odclib_desc;        
    PT_UserInfoDescriptor     user_info_desc;     
    PT_UserCertDataDescriptor cert_data_desc;     
    PT_TerminalInfoDescriptor terminal_info_desc; 

    PT_SessionInfoDescriptor  session_info_desc;    //TODO:需要根据配置文件设置

    uint64_t Id() const { return id_; }
    caid_t CAId() const { return caid_;}
    void Id(uint64_t id) { id_ = id; terminal_info_desc.session_id_ = id_; }

    void    UpdateSessionInfo(PB_SvcURLDescriptor &url_desc);
    string  GetBackURL(string &curr_apply_url);
    void    UpdateBackURL(PB_SvcURLDescriptor &url_desc);

private:
    uint64_t    id_;
    caid_t      caid_;
    bool        valid_;
    Logger      &logger_;
};

#endif // !gethua_sessionmgr_terminal_session_h_
