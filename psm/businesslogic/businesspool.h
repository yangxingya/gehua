/*
 * @brief: business pool, for all the business logic operate pool
 *         it is a mutil-thread pool
 */

#if !defined gehua_businesslogic_business_pool_h_
#define gehua_businesslogic_business_pool_h_

#include <vector>
#include <cpplib/logger.h>
#include "../comm-def.h"
#include "../sessionmgr/casession.h"
#include "../sessionmgr/casessionmgr.h"
#include "workpool.h"

struct CASession;
struct CASessionMgr;
struct TermSession;
struct TermConnection;
struct PtLoginRequest;

using ::std::vector;

class BusinessPool
{
public:
  BusinessPool(Logger& logger, int thread_cnt = 1);
	void AddWork(Work *wk, caid_t caid);
	void AddDelayedWork(DelayedWork *delay_wk, caid_t caid);

  TermSession* GenTermSession(PtLoginRequest *msg, TermConnection *conn);
  void DelTermSession(TermSession *term_session);
	CASession* FindCASessionById(caid_t caid);
	CASession* FindCASessionByTermSessionId(uint64_t terminal_session_id);
private:
  int getId(uint64_t term_session_id);
private:
  Logger &logger_;
	WorkPool wk_pool_;
	vector<CASessionMgr*> pool_relation_ca_session_mgr_;
  int thread_cnt_;

  BusinessPool(BusinessPool const&);
  BusinessPool& operator=(BusinessPool const&);
};

#endif // !gehua_businesslogic_business_pool_h_
