/*
 * @brief: ca session.
 *
 */

#if !defined gehua_sessionmgr_ca_session_h_
#define gehua_sessionmgr_ca_session_h_

#include <map>
#include "../comm-def.h"

namespace gehua {
namespace sessionmgr {

using ::std::map;

class TerminalConnection;
class TimeOutTimer;
struct CASession
{
public:
	CASession(caid_t caid, TimeOutTimer& timeout_timer);
	void Add(TerminalSession* terminal_session);
  void Remove(uint64_t terminal_session_id);

  caid_t Id() const { return caid_; }
private:
	map<uint64_t, TerminalSession*> terminal_session_map_;
	TimeOutTimer& timeout_timer_;
	caid_t caid_;

	void OnAdd(TerminalConnection *tc);
  void OnRemove(TerminalConnection *tc);
};

} // namespace sessionmgr
} // namespace gehua

#endif //!gehua_sessionmgr_ca_session_h_
