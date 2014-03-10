/*
 * @brief: terminal session.
 */

#if !defined gehua_sessionmgr_terminal_session_h_
#define gehua_sessionmgr_terminal_session_h_

#include <stack>
#include <string>
#include "../bs-comm-def.h"

namespace gehua {
namespace sessionmgr {

using ::std::stack;
using ::std::string;

class TerminalConnection;
struct TerminalSession
{
	caid_t caid;
	uint64_t id;
	TerminalConnection *terminal_conn;
	stack<string> back_url_stack;

	BusinessStatus curr_status;
	BusinessStatus last_status;

	TerminalClass terminal_class;
	int terminal_subclass;

	int valid_status;

	Mutex send_mtx;

	ServiceGroup service_grp;
	CertInfo cert_info_cache;
	OdcInfo odc_info;
};

} // namespace sessionmgr
} // namespace gehua


#endif // !gethua_sessionmgr_terminal_session_h_
