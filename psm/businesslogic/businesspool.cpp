
#include "businesspool.h"
#include <protocol/protocol_v2_pt_message.h>
#include "../sessionmgr/termsession.h"
#include "../psmcontext.h"

BusinessPool::BusinessPool(Logger& logger, int thread_cnt)
    : logger_(logger), wk_pool_(logger, thread_cnt)
    , thread_cnt_(thread_cnt)
    , psm_ctx_(0)
    , started_(false)
    , global_cnt_(0)
{
    for (int i = 0; i < thread_cnt; ++i) {
        pool_relation_ca_session_mgr_.push_back(new CASessionMgr);
    }
}

void BusinessPool::AddWork(Work *wk, caid_t caid)
{
    if (!started_) {
        logger_.Warn("PSM Business Pool have stoped, but add work, caid is: %d", caid);
        return;
    }
    
    wk_pool_.Assign(wk, getIdByCAId(caid));
}

void BusinessPool::AddDelayedWork(DelayedWork *delay_wk, caid_t caid)
{
    if (!started_) {
        logger_.Warn("PSM Business Pool have stoped, but add delay work, caid is: %d", caid);
        return;
    }

    wk_pool_.Assign(delay_wk, getIdByCAId(caid));
}



uint64_t BusinessPool::genTermSessionId(
    double time, uint16_t g_cnt, uint32_t ip, caid_t caid)
{
    //todo:: generate terminal session id.
    // terminal session id is 64 bit 
    // [ip<1bytes>] + [time<3bytes>] + [global_cnt<2bytes>] + [caid<2bytes>];

    union tmp_t {
        struct {
            uint8_t  ip;
            uint8_t  t1;
            uint16_t t2;
            uint16_t cnt;
            uint16_t ca;
        } pt;
        uint64_t value;
    };

    uint32_t ti = (uint32_t)time;

    tmp_t tmp;
    //ip <i1.i2.i3.i4>, i4 is key, and i4 is high byte.
    tmp.pt.ip = (uint8_t)(ip & 0xFF000000);
    tmp.pt.t1 = (uint8_t)((ti & 0x00FF0000) >> 16);
    tmp.pt.t2 = (uint16_t)(ti & 0x0000FFFF);
    tmp.pt.cnt = g_cnt;
    tmp.pt.ca = (uint16_t)(caid & 0x00000000000000FF);

    return tmp.value;
}

uint16_t BusinessPool::getCABytesByTermSessionId(uint64_t ts_id)
{
    // is high 2 bytes or low 2bytes;

    //uint64_t and = 0x000000000000FFFF;
    //return (uint16_t)(ts_id & and);

    uint64_t and = 0xFFFF000000000000;
    return (uint16_t)((ts_id & and) >> 48);
}

uint16_t BusinessPool::getCAbytesByCAId(caid_t caid)
{
    // is high 2 bytes or low 2bytes;

    //uint64_t and = 0x000000000000FFFF;
    //return (uint16_t)(caid & and);

    uint64_t and = 0xFFFF000000000000;
    return (uint16_t)((caid & and) >> 48);
}

TermSession* BusinessPool::GenTermSession(PtLoginRequest *msg, TermConnection *conn)
{
    if (!started_) {
        logger_.Warn("PSM Business Pool have stoped, but generate terminal session"); 
        return 0;
    }

    TermSession *ts = new TermSession(logger_, msg, conn);
    if (!ts->valid()) {
        // valid failed.
        delete ts;
        logger_.Warn("PSM Business Pool generate terminal session user cert failure");
        return NULL;
    }

    // valid ok.

    // generate terminal session id.
    uint64_t ts_id = genTermSessionId(get_up_time(), global_cnt_++, psm_ctx_->ip_addr(), ts->CAId());
    logger_.Trace("PSM Business Pool generate terminal session id: "U64T, ts_id);
    ts->Id(ts_id);

    CASessionMgr *cs_mgr = pool_relation_ca_session_mgr_[getIdByTermSessionId(ts->Id())];
    CASession *cs = cs_mgr->FindCASessionById(ts->CAId());

    if (cs == NULL) {
        cs = cs_mgr->Create(ts->CAId());
        cs_mgr->Attach(cs);
    }

    ts->ca_session = cs;
    cs->Add(ts);

    //…Ë÷√Session√Ë ˆ∑˚
    {
        ByteStream agreekey;
        for ( int i=0; i<8; i++ ) {
            agreekey.Add((uint8_t*)"\x0F\x0E\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00",16);
        } 

        ts->session_info_desc.agreement_key_ = agreekey;
        ts->session_info_desc.session_id_    = ts->Id();
        ts->session_info_desc.timeout_       = 15;
        ts->session_info_desc.interval_time_ = 10;
        ts->session_info_desc.valid_         = true;
    }

    //…Ë÷√÷’∂À√Ë ˆ∑˚
    ts->terminal_info_desc.session_id_ = ts->Id();

    return ts;
}

int BusinessPool::getIdByTermSessionId(uint64_t ts_id)
{
    return (int)(getCABytesByTermSessionId(ts_id) % thread_cnt_);
}

int BusinessPool::getIdByCAId(caid_t caid)
{
    return (int)(getCAbytesByCAId(caid) % thread_cnt_);
}

uint32_t BusinessPool::DelTermSession(TermSession *ts)
{
    assert(ts != 0);

    CASession *cs = ts->ca_session;

    if (cs == NULL) {
        //log.
        return 0;
    }

    cs->Remove(ts->Id());

    uint32_t ret = cs->termCnt();

    //if no terminal session in ca session,
    // remove ca session.
    if (cs->termCnt() == 0) {
        CASessionMgr *cs_mgr = pool_relation_ca_session_mgr_[getIdByTermSessionId(ts->Id())];
        cs_mgr->Detach(cs->Id());
        cs_mgr->Destory(cs);
    }

    return ret;
}

bool BusinessPool::Start()
{
    if (!started_) {
        started_ = wk_pool_.Start();
    }
    return started_;
}

void BusinessPool::Stop()
{
    wk_pool_.Stop();
    started_ = false;
}

TermSession* BusinessPool::FindTermSessionById(uint64_t ts_id)
{
    CASessionMgr *csmgr = pool_relation_ca_session_mgr_[getIdByTermSessionId(ts_id)];

    CASession *cs = csmgr->FindCASessionByTermSessionId(ts_id);
    if (cs == NULL) return NULL;
    
    map<uint64_t, TermSession*>::iterator 
        it = cs->terminal_session_map_.find(ts_id);
    if (it == cs->terminal_session_map_.end())
        return NULL;
 
    return it->second;
}