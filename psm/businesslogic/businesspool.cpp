
#include "businesspool.h"
#include <protocol/protocol_v2_pt_message.h>
#include "../sessionmgr/termsession.h"

BusinessPool::BusinessPool(Logger& logger, int thread_cnt)
  : logger_(logger), wk_pool_(logger, thread_cnt)
  , thread_cnt_(thread_cnt)
{
  for (int i = 0; i < thread_cnt; ++i) {
    pool_relation_ca_session_mgr_.push_back(new CASessionMgr);
  }
}

void BusinessPool::AddWork(Work *wk, caid_t caid)
{
  
}

void BusinessPool::AddDelayedWork(DelayedWork *delay_wk, caid_t caid)
{

}

TermSession* BusinessPool::GenTermSession(PtLoginRequest *msg, TermConnection *conn)
{
    TermSession *ts = new TermSession(msg, conn);
    if (!ts->valid()) {
       delete ts;
       //log.
       return NULL;
    }

    //login valid ok.
    ts->valid_status = 0;
    
    CASessionMgr *cs_mgr = pool_relation_ca_session_mgr_[getId(ts->Id())];
    CASession *cs = cs_mgr->FindCASessionById(ts->CAId());

    if (cs == NULL) {
        cs = cs_mgr->Create(ts->CAId());
    }

    ts->ca_session = cs;
    cs->Add(ts);
  
    return ts;
}

int BusinessPool::getId(uint64_t term_session_id)
{
    return (int)(term_session_id % thread_cnt_);
}

void BusinessPool::DelTermSession(TermSession *ts)
{
    assert(ts != 0);

    CASession *cs = ts->ca_session;

    if (cs == NULL) {
        //log.
        return;
    }

    cs->Remove(ts->Id());
    
    //remove ca session.
    if (cs->termCnt() == 0) {
        CASessionMgr *cs_mgr = pool_relation_ca_session_mgr_[getId(ts->Id())];
        cs_mgr->Detach(cs->Id());
        cs_mgr->Destory(cs);
    }
}