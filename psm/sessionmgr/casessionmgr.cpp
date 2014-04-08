
#include "casessionmgr.h"
#include "casession.h"

CASessionMgr::CASessionMgr(Logger &logger, PSMContext *psm_ctx, uint32_t timer_span)
    : logger_(logger), conn_timeout_timer_(logger, psm_ctx)
{
    conn_timeout_timer_.Activate(timer_span);
}

CASession* CASessionMgr::FindCASessionById(caid_t caid)
{
    MutexLock lock(ca_session_mtx_);

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
    {
        MutexLock lock(ca_session_mtx_);
        ca_session_map_[cs->Id()] = cs;
    }

	MutexLock lock(cs->termsession_mtx_);
    map<uint64_t, shared_ptr<TermSession> >::iterator 
        it = cs->terminal_session_map_.begin();

    for (; it != cs->terminal_session_map_.end(); ++it) {
        terminal_id_ca_session_map_[it->first] = cs;
    }
}

CASession* CASessionMgr::Detach(caid_t caid)
{
    CASession *cs = NULL;

    {
        MutexLock lock(ca_session_mtx_);
        map<caid_t, CASession*>::iterator 
            it = ca_session_map_.find(caid);
        if (it != ca_session_map_.end()) {
            cs = it->second;
            ca_session_map_.erase(it);
        }
    }

    if (cs != NULL) {
        map<uint64_t, CASession*>::iterator cit;

        MutexLock lock(cs->termsession_mtx_);

        map<uint64_t, shared_ptr<TermSession> >::iterator 
            tit = cs->terminal_session_map_.begin();
        //删除终端会话id映射CA会话表中的 CA会话的终端会话id列表。
        for (; tit != cs->terminal_session_map_.end(); ++tit) {
            cit = terminal_id_ca_session_map_.find(tit->first);
            if (cit != terminal_id_ca_session_map_.end()) {
                terminal_id_ca_session_map_.erase(cit);
            }
        }
    }
    return cs;
}

void CASessionMgr::AttachTermSessionInfo( uint64_t ts_id, CASession *ca_session )
{
    terminal_id_ca_session_map_[ts_id] = ca_session;
}

void CASessionMgr::DetachTermSessionInfo( uint64_t ts_id )
{
    map<uint64_t, CASession*>::iterator 
        it = terminal_id_ca_session_map_.find(ts_id);
    if (it != terminal_id_ca_session_map_.end())
    {
        terminal_id_ca_session_map_.erase(it);
    }
}

void CASessionMgr::AddToTimer(weak_ptr<TermSession> ts)
{
	conn_timeout_timer_.Add(ts);
}

void CASessionMgr::RemoveFromTimer(uint64_t ts_id)
{
	conn_timeout_timer_.Remove(ts_id);
}
