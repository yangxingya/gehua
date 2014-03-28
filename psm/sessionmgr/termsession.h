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

struct UserInfo;
struct TermConnection;
struct PtLoginRequest;
struct CASession;
struct BusinessStatusInfo;

struct TermSession
{
public:
    TermSession(Logger &logger, PtLoginRequest *msg, TermConnection *tconn);
    bool valid() const { return valid_; }
public:

    CASession                 *ca_session_;
	Mutex                     termconn_mtx_;
    TermConnection            *term_conn_;

    BusinessType              curr_busi_type_;
    BusinessType              last_busi_type_;

    string                    last_local_svc_url_;
    PB_SvcURLDescriptor       curr_svc_url_desc_;

    ServiceGroup              service_grp_;
    UserInfo                  user_info_;

    PT_OdcLibDescriptor       odclib_desc_;        
    PT_UserInfoDescriptor     user_info_desc_;     
    PT_UserCertDataDescriptor cert_data_desc_;     
    PT_TerminalInfoDescriptor terminal_info_desc_; 

    PT_SessionInfoDescriptor  session_info_desc_;    //TODO:需要根据配置文件设置

    uint64_t Id() const { return id_; }
    caid_t CAId() const { return caid_;}
    void Id(uint64_t id) { id_ = id; terminal_info_desc_.session_id_ = id_; }

    void UpdateSessionInfo(PB_SvcURLDescriptor &url_desc);
    bool IsValidTermInfoDesc(PT_TerminalInfoDescriptor term_info_desc);

private:
    uint64_t    id_;
    caid_t      caid_;
    bool        valid_;
    Logger      &logger_;
};

#endif // !gethua_sessionmgr_terminal_session_h_
