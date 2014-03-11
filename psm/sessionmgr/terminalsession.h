/*
 * @brief: terminal session.
 */

#if !defined gehua_sessionmgr_terminal_session_h_
#define gehua_sessionmgr_terminal_session_h_

#include <stack>
#include <string>
#include "../comm-def.h"
#include "../bs-comm-def.h"

namespace gehua {
namespace sessionmgr {

using ::std::stack;
using ::std::string;

struct PtLoginRequest;
class TerminalConnection;
struct TerminalSession
{
public:
  TerminalSession(PtLoginRequest *msg, TerminalConnection *tconn);
  bool valid() const { return valid_; }
public:
  CASession *ca_session;
	TerminalConnection *terminal_conn;
	stack<string> back_url_stack;

	BusinessStatus curr_status;
	BusinessStatus last_status;

	int valid_status; // 0 valid failed, 1 valid successful.

	ServiceGroup service_grp;
  UserInfo user_info;

  PT_OdcLibDescriptor       odclib_desc;        
  PT_UserInfoDescriptor     user_info_desc;     
  PT_UserCertDataDescriptor cert_data_desc;     
  PT_TerminalInfoDescriptor terminal_info_desc; 

  uint64_t Id() const { return id_; }
  caid_t CAId() const { return caid_;
private:
  uint64_t id_;
  caid_t caid_;
  bool valid_;
};

} // namespace sessionmgr
} // namespace gehua

#endif // !gethua_sessionmgr_terminal_session_h_
