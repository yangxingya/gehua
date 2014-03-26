/*
* @brief: ca session manager.
*
*/

#if !defined gehua_sessionmgr_ca_session_mgr_h_
#define gehua_sessionmgr_ca_session_mgr_h_

#include <cpplib/logger.h>
#include "../../common/widget.h"
#include "timeouttimer.h"

using ::std::map;

struct CASession;
struct CASessionMgr
{
public:
    CASessionMgr(Logger &logger, uint32_t timer_span = 5 * 60 * 1000);
    CASession* FindCASessionById(caid_t caid);
    CASession* FindCASessionByTermSessionId(uint64_t ts_id);
    CASession* Create(caid_t caid);
    void Destory(CASession *ca_session);
    void Attach(CASession *ca_session);
    void AttachTermSessionInfo(uint64_t ts_id, CASession *ca_session);

    CASession* Detach(caid_t caid);
    void DetachTermSessionInfo(uint64_t ts_id);

private:
    Logger&      logger_;
    TimeOutTimer conn_timeout_timer_;

    map<caid_t, CASession*>   ca_session_map_;
    map<uint64_t, CASession*> terminal_id_ca_session_map_;
    
};

#endif //!gehua_sessionmgr_ca_session_mgr_h_
