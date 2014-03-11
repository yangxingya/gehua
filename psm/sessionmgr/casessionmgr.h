/*
 * @brief: ca session manager.
 *
 */

#if !defined gehua_sessionmgr_ca_session_mgr_h_
#define gehua_sessionmgr_ca_session_mgr_h_

#include <map>
#include "../comm-def.h"
#include "../timeouttimer.h"

namespace gehua {
namespace sessionmgr {

using ::std::map;

struct CASession;
class CASessionMgr
{
public:
	CASession* FindCASessionById(caid_t caid);
	CASession* FindCASessionByTerminalSessionId(uint64_t ts_id);
	CASession* Create(caid_t caid);
	void Destory(CASession *ca_session);
	void Attach(CASession *ca_session);
	CASession* Detach(caid_t caid);
private:
	map<caid_t, CASession*> ca_session_map_;
	map<uint64_t CASession*> terminal_id_ca_session_map_;
	TimeOutTimer conn_timeout_timer_;
};

} // namespace sessionmgr
} // namespace gehua

#endif //!gehua_sessionmgr_ca_session_mgr_h_
