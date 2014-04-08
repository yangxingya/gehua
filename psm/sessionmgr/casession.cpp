

#include "casession.h"
#include "termsession.h"
#include "timeouttimer.h"

CASession::CASession(caid_t caid, TimeOutTimer& timeout_timer)
: caid_(caid), timeout_timer_(timeout_timer)
{

}

void CASession::Add(weak_ptr<TermSession> ts)
{
	shared_ptr<TermSession> sp_ts(ts.lock());
	if (!sp_ts) return;

	MutexLock lock(termsession_mtx_);
	terminal_session_map_[sp_ts->Id()] = sp_ts;
}

void CASession::Remove(uint64_t ts_id)
{
	MutexLock lock(termsession_mtx_);
	map<uint64_t, shared_ptr<TermSession> >::iterator
		it = terminal_session_map_.find(ts_id);

	if (it != terminal_session_map_.end()) {
		terminal_session_map_.erase(it);
	}
}

weak_ptr<TermSession> CASession::GetSTBTermSession()
{
    MutexLock lock(termsession_mtx_);

    map<uint64_t, shared_ptr<TermSession> >::iterator it = terminal_session_map_.begin();

    for ( ; it != terminal_session_map_.end(); it++ )
    {
        if ( it->second->terminal_info_desc_.terminal_class_ == TerminalSTB )
        {
            return it->second;
        }
    }
    
    return weak_ptr<TermSession>();
}
