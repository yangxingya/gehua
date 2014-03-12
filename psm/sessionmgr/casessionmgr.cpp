
#include "casessionmgr.h"
#include "casession.h"

CASession* CASessionMgr::FindCASessionById(caid_t caid)
{
  map<caid_t, CASession*>::iterator 
    it = ca_session_map_.find(caid);
  if (it != ca_session_map_.end())
    return it->second;

  return NULL;
}

CASession* CASessionMgr::FindCASessionByTermSessionId(uint64_t ts_id)
{
  map<uint64_t, CASession*>::iterator 
    it = terminal_id_ca_session_map_.find(ts_id);
  if (it != terminal_id_ca_session_map_.end())
    return it->second;

  return NULL;
}

CASession* CASessionMgr::Create(caid_t caid)
{
  return new CASession(caid, conn_timeout_timer_);
}

void CASessionMgr::Destory(CASession *ca_session)
{
  delete ca_session;
}

void CASessionMgr::Attach(CASession *cs)
{
  ca_session_map_[cs->Id()] = cs;

  map<uint64_t, TermSession*>::iterator 
    it = cs->terminal_session_map_.begin();

  for (; it != cs->terminal_session_map_.end(); ++it) {
    terminal_id_ca_session_map_[it->first] = cs;
  }
}

CASession* CASessionMgr::Detach(caid_t caid)
{
  CASession *cs = NULL;
  map<caid_t, CASession*>::iterator 
    it = ca_session_map_.find(caid);
  if (it != ca_session_map_.end()) {

    cs = it->second;

    map<uint64_t, TermSession*>::iterator 
      tit = it->second->terminal_session_map_.begin();

    map<uint64_t, CASession*>::iterator cit;
    for (; tit != cs->terminal_session_map_.end(); ++tit) {
      cit = terminal_id_ca_session_map_.find(tit->first);
      if (cit != terminal_id_ca_session_map_.end()) {
        terminal_id_ca_session_map_.erase(it);
      }
    }
    ca_session_map_.erase(it);
  }
  return cs;
}