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
struct PSMContext;

using ::std::vector;

struct BusinessPool
{
public:
    BusinessPool(Logger& logger, int thread_cnt = 1);
    void AddWork(Work *wk, caid_t caid);
    void AddDelayedWork(DelayedWork *delay_wk, caid_t caid);

    TermSession* GenTermSession(PtLoginRequest *msg, TermConnection *conn);
    //return the number of terminal session in the same ca sesssion.
    uint32_t DelTermSession(TermSession *term_session);
    CASession* FindCASessionById(caid_t caid);
    TermSession* FindTermSessionById(uint64_t term_session_id);

    bool Start();
    void Stop();

    PSMContext *psm_ctx_;
private:
    int getIdByTermSessionId(uint64_t term_session_id);
    int getIdByCAId(caid_t caid);
    static uint64_t genTermSessionId(
        double time, uint16_t g_cnt, uint32_t ip, caid_t caid);
    static uint16_t getCABytesByTermSessionId(uint64_t ts_id);
    static uint16_t getCAbytesByCAId(caid_t caid);
private:
    Logger &logger_;
    WorkPool wk_pool_;
    vector<CASessionMgr*> pool_relation_ca_session_mgr_;
    int thread_cnt_;
    volatile bool started_;

    uint16_t global_cnt_;

    BusinessPool(BusinessPool const&);
    BusinessPool& operator=(BusinessPool const&);
};

#endif // !gehua_businesslogic_business_pool_h_
