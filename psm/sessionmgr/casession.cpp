

#include "casession.h"
#include "termsession.h"

CASession::CASession(caid_t caid, TimeOutTimer& timeout_timer)
: caid_(caid), timeout_timer_(timeout_timer)
{

}

void CASession::Add(weak_ptr<TermSession> ts)
{
    shared_ptr<TermSession> sp_ts(ts.lock());
    if (!sp_ts) return;

    terminal_session_map_[sp_ts->Id()] = ts;
    OnAdd(sp_ts->term_conn_);
}

void CASession::Remove(uint64_t ts_id)
{
    map<uint64_t, weak_ptr<TermSession> >::iterator
        it = terminal_session_map_.find(ts_id);

    if (it != terminal_session_map_.end()) {
        shared_ptr<TermSession> sp_ts(it->second.lock());
        if (!sp_ts) return;
        OnRemove(sp_ts->term_conn_);
        terminal_session_map_.erase(it);
    }
}

void CASession::OnAdd(TermConnection *tc)
{
    //timeout_timer_.Add(
}

void CASession::OnRemove(TermConnection *tc)
{
    //...
}

weak_ptr<TermSession> CASession::GetSTBTermSession()
{
    map<uint64_t, weak_ptr<TermSession> >::iterator it = terminal_session_map_.begin();

    for ( ; it != terminal_session_map_.end(); it++ )
    {
        shared_ptr<TermSession> sp_ts(it->second.lock());
        if (!sp_ts) continue;
        if ( sp_ts->terminal_info_desc_.terminal_class_ == TerminalSTB )
        {
            return it->second;
        }
    }
    
    return NULL;
}
