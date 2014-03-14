
#include "termserver.h"
#include "../psmcontext.h"

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