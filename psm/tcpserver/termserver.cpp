
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

    if (new_conn) {
        total_connections_ ++;
        new_connections_ ++;
    } else {
        reject_connections_++;
    }
    return (AioConnection*)new_conn;
}

void TermServer::OnDisconnected(AioConnection* conn)
{
    conn->SetDirty();
    disconnections_++;
    // delete connection......
    TermConnection *term_conn = (TermConnection*)conn;

	uint64_t tid = 0;
	caid_t   cid = 0;
    shared_ptr<TermSession> sp_ts(term_conn->term_session_.lock());
    if (sp_ts) {
		tid = sp_ts->Id();
		cid = sp_ts->CAId();
    }

	logger_->Warn("[终端连接被终端断开][TSId:"SFMT64U"][CAId:"SFMT64U"]", tid, cid);

    term_conn->SetDirty();

    total_connections_--;
}
