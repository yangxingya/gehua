

#include "busiserver.h"
#include "../psmcontext.h"


AioConnection* BusiServer::OnConnected(TCPConnection* tcp)
{
    BusiConnection* new_conn = new BusiConnection(tcp, this);

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

void BusiServer::OnDisconnected(AioConnection* conn)
{

}

void BusiServer::OnDataReceived(AioConnection* conn)
{

}