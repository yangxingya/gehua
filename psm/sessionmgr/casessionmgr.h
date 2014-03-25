/*
* @brief: ca session manager.
*
*/

#if !defined gehua_sessionmgr_ca_session_mgr_h_
#define gehua_sessionmgr_ca_session_mgr_h_

#include "../../common/widget.h"
#include "../timeouttimer.h"

using ::std::map;

struct CASession;
struct CASessionMgr
{
public:
    CASession* FindCASessionById(caid_t caid);
    CASession* FindCASessionByTermSessionId(uint64_t ts_id);
    CASession* Create(caid_t caid);
    void Destory(CASession *ca_session);
    void Attach(CASession *ca_session);
    CASession* Detach(caid_t caid);
private:
    map<caid_t, CASession*> ca_session_map_;
    map<uint64_t, CASession*> terminal_id_ca_session_map_;
    TimeOutTimer conn_timeout_timer_;
};

#endif //!gehua_sessionmgr_ca_session_mgr_h_
