/*
* @brief: ca session.
*
*/

#if !defined gehua_sessionmgr_ca_session_h_
#define gehua_sessionmgr_ca_session_h_

#include <memory>
#include <cpplib/cpplibbase.h>
#include "../../common/widget.h"

using ::std::tr1::weak_ptr;
using ::std::tr1::shared_ptr;

struct TermConnection;
struct TimeOutTimer;
struct TermSession;
struct CASession
{
public:
    CASession(caid_t caid, TimeOutTimer& timeout_timer);
    void Add(weak_ptr<TermSession> terminal_session);
    void Remove(uint64_t terminal_session_id);

    caid_t Id() const { return caid_; }
    size_t termCnt() const { return terminal_session_map_.size(); }

    weak_ptr<TermSession> GetSTBTermSession();

    map<uint64_t, shared_ptr<TermSession> > terminal_session_map_;
private:
    caid_t caid_;
    TimeOutTimer& timeout_timer_;

    void OnAdd(weak_ptr<TermSession> ts);
    void OnRemove(uint64_t ts_id);

    friend struct CASessionMgr;
};

#endif //!gehua_sessionmgr_ca_session_h_
