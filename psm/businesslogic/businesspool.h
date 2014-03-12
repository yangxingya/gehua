/*
 * @brief: business pool, for all the business logic operate pool
 *         it is a mutil-thread pool
 */

#if !defined gehua_businesslogic_business_pool_h_
#define gehua_businesslogic_business_pool_h_

#include <vector>
#include <cpplib/logger.h>
#include "../sessionmgr/casession.h"
#include "../sessionmgr/casessionmgr.h"
#include "workpool.h"

namespace gehua {
namespace businesslogic {

using ::std::vector;
using ::gehua::sessionmgr::CASession;
using ::gehua::sessionmgr::CASessionMgr;
using ::gehua::sessionmgr::TerminalSession;

struct PtLoginRequest;
struct TerminalConnection;

class Work;
class DelayWork;

class BusinessPool
{
public:
  BusinessPool(Logger& logger, int thread_cnt = 1);
	void AddWork(Work *wk, caid_t caid);
	void AddDelayWork(DelayWork *delay_wk, caid_t caid);

  TerminalSession* GenTermSession(PtLoginRequest *msg, TerminalConnection *conn);
  void DelTermSession(TerminalSession *term_session);
	CASession* FindCASessionById(caid_t caid);
	CASession* FindCASessionByTerminalSessionId(uint64_t terminal_session_id);
private:
  int getId(uint64_t term_session_id);
private:
  Logger &logger_;
	WorkPool wk_pool_;
	vector<CASessionMgr> pool_relation_ca_session_mgr_;
  int thread_cnt_;
};

} // namespace businesslogic
} // namespace gehua

#endif // !gehua_businesslogic_business_pool_h_
