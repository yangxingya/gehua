

#include "busiserver.h"
#include "../psmcontext.h"


AioConnection* BusiServer::OnConnected(TCPConnection* tcp)
{
    BusiConnection* busi_conn = new BusiConnection(logger_, tcp, this);

    //set psm context.
    busi_conn->psm_ctx_ = psm_ctx_;

    LOCK_BEGIN(lock,mt_);
    if ( busi_conn ) {
        total_connections_ ++;
        new_connections_ ++;
    }
    else {
        reject_connections_++;
    }
    LOCK_END();

    LOCK_BEGIN(lockd,connection_mt_);

    //if not ip, then add ip map.
    map<uint32_t, list<BusiConnection*> >::iterator 
        it = ip_conn_list_map_.find(busi_conn->ClientIp());
    if (it == ip_conn_list_map_.end()) {
        ip_conn_list_map_[busi_conn->ClientIp()] = list<BusiConnection*>();
        logger_->Info("PSM Business Server first connection ip: %s", busi_conn->ip_string().c_str());
    }

    //add connection to ip map list.
    list<BusiConnection*> &ip_conns = ip_conn_list_map_[busi_conn->ClientIp()];
    ip_conns.push_back(busi_conn);


    all_conn_map_[busi_conn] = busi_conn;
    LOCK_END();

    return (AioConnection*)busi_conn;
}

void BusiServer::OnDisconnected(AioConnection* conn)
{
    BusiConnection *busi_conn = (BusiConnection*)conn;

    conn->SetDirty();

    LOCK_BEGIN(lock,mt_);
    disconnections_++;
    total_connections_--;
    LOCK_END();

    LOCK_BEGIN(lock2,connection_mt_);

    //find ip map conn list.
    map<uint32_t, list<BusiConnection*> >::iterator 
        it = ip_conn_list_map_.find(busi_conn->ClientIp());

    //if finded, erase conn.
    if (it != ip_conn_list_map_.end()) {
        list<BusiConnection*> &busi_conns = it->second;
        list<BusiConnection*>::iterator bit = busi_conns.begin();

        for (/* without initialize value */; bit != busi_conns.end(); ++bit) {
            if (*bit == busi_conn) {
                busi_conns.erase(bit);
                break;
            }
        }

        //if ip map conn list size is zero, erase <ip, list<conn*> > iterator.
        if (busi_conns.size() == 0) {
            ip_conn_list_map_.erase(it); 
            logger_->Info("PSM Business Server disconnected all connection ip: %s", busi_conn->ip_string().c_str());
        }
    }

    if ( busi_conn ) {
        disconnect_list_.push_back(busi_conn);
    }
    else {
        LOG_ERROR("disconnect connection isn't type of BusiServerConnection");
    }
    LOCK_END();
}

void BusiServer::OnDataReceived(AioConnection* conn)
{

}