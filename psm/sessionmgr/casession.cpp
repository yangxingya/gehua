

#include "casession.h"
#include "termsession.h"

CASession::CASession(caid_t caid, TimeOutTimer& timeout_timer)
  : caid_(caid), timeout_timer_(timeout_timer)
{

}

void CASession::Add(TermSession* ts)
{
  terminal_session_map_[ts->Id()] = ts;
  OnAdd(ts->terminal_conn);
}

void CASession::Remove(uint64_t ts_id)
{
  map<uint64_t, TermSession*>::iterator
    it = terminal_session_map_.find(ts_id);

  if (it != terminal_session_map_.end()) {
    OnRemove(it->second->terminal_conn);
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