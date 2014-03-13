/*
* @brief: ca session.
*
*/

#if !defined gehua_sessionmgr_ca_session_h_
#define gehua_sessionmgr_ca_session_h_

#include <map>
#include "../comm-def.h"

using ::std::map;

struct TermConnection;
struct TimeOutTimer;
struct TermSession;
struct CASession
{
public:
    CASession(caid_t caid, TimeOutTimer& timeout_timer);
    void Add(TermSession* terminal_session);
    void Remove(uint64_t terminal_session_id);

    caid_t Id() const { return caid_; }
    size_t termCnt() const { return terminal_session_map_.size(); }
private:
    caid_t caid_;
    TimeOutTimer& timeout_timer_;
    map<uint64_t, TermSession*> terminal_session_map_;

    void OnAdd(TermConnection *tc);
    void OnRemove(TermConnection *tc);

    friend struct CASessionMgr;
};

#endif //!gehua_sessionmgr_ca_session_h_
