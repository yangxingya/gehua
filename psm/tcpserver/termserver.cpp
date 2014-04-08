
#include "termserver.h"
#include "../psmcontext.h"
#include "../sessionmgr/termsession.h"
#include "../businesslogic/businesspool.h"

AioConnection* TermServer::OnConnected( TCPConnection* tcp )
{
    TermConnection* new_conn = new TermConnection(logger_, tcp, this);

    logger_->Trace("PSM new Terminal connection in");

    //set psm context.
    new_conn->psm_ctx_ = psm_ctx_;

    LOCK_BEGIN(lock,mt_);
    if ( new_conn ) {
        total_connections_ ++;
        new_connections_ ++;
    }
    else {
        reject_connections_++;
    }
    LOCK_END();

    LOCK_BEGIN(lockd,connection_mt_);
    all_conn_map_[new_conn] = new_conn;
    LOCK_END();

    return (AioConnection*)new_conn;
}

void TermServer::OnDisconnected(AioConnection* conn)
{
    uint64_t tid = 0;
    caid_t   cid = 0;

    TermConnection *term_conn = (TermConnection*)conn;
    shared_ptr<TermSession> sp_ts(term_conn->term_session_.lock());
    if (sp_ts) {
        tid = sp_ts->Id();
        cid = sp_ts->CAId();
        MutexLock lock(sp_ts->termconn_mtx_);
        sp_ts->term_conn_ = NULL;
    }

    logger_->Warn("[Terminal Disconnected][TSId:0x"SFMT64X"][CAId:"SFMT64U"]", tid, cid);

    conn->SetDirty();

    LOCK_BEGIN(lock,mt_);
    disconnections_++;
    total_connections_--;
    LOCK_END();



    LOCK_BEGIN(lock2,connection_mt_);

    if ( term_conn ) {
        disconnect_list_.push_back(term_conn);
    }
    else {
        LOG_ERROR("disconnect connection isn't type of TermServerConnection");
    }
    LOCK_END();
}
