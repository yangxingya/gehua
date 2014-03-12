
#include "businesspool.h"
#include <protocol/protocol_v2_pt_message.h>
#include "../sessionmgr/terminalsession.h"

using ::gehua::businesslogic::BusinessPool;

BusinessPool::BusinessPool(Logger& logger, int thread_cnt)
  : logger_(logger), wk_pool_(logger, thread_cnt)
  , thread_cnt_(thread_cnt)
{
  pool_relation_ca_session_mgr_.resize(thread_cnt);
}

void BusinessPool::AddWork(Work *wk, caid_t caid)
{

}

void BusinessPool::AddDelayWork(DelayWork *delay_wk, caid_t caid)
{

}

TerminalSession* BusinessPool::GenTermSession(PtLoginRequest *msg, TerminalConnection *conn)
{
    TerminalSession *ts = new TerminalSession(msg, conn);
    if (!ts->valid()) {
       delete ts;
       //log.
       return NULL;
    }

    //login valid ok.
    ts->valid_status = 0;
    
    CASessionMgr &cs_mgr = pool_relation_ca_session_mgr_[getId(ts->Id())];
    CASession *cs = cs_mgr.FindCASessionById(ts->CAId());

    if (cs == NULL) {
        cs = cs_mgr.Create(ts->CAId());
    }

    ts->ca_session = cs;
    cs->Add(ts);
  
    return ts;
}

int BusinessPool::getId(uint64_t term_session_id)
{
    return (int)(term_session_id % thread_cnt_);
}

void BusinessPool::DelTermSession(TerminalSession *term_session)
{
    assert(term_session != 0);

    CASession *cs = term_session->ca_session;

    if (cs == NULL) {
        //log.
        return;
    }

    cs->Remove(term_session->Id());
    
    //remove ca session.
    if (cs->termCnt() == 0) {
        CASessionMgr &cs_mgr = pool_relation_ca_session_mgr_[getId(ts->Id())];
        cs_mgr.Detach(cs->Id());
        cs_mgr.Destory(cs);
    }
}