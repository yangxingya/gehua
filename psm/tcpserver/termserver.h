/*
* @brief: terminal server, tcp server, not http server
*/

#if !defined gehua_tcpserver_terminal_server_h_
#define gehua_tcpserver_terminal_server_h_

#include <cpplib/timer.h>
#include <cpplib/netaio/aiomanager.h>
#include "../../common/widget.h"
#include "termconnection.h"

struct PSMContext;
struct TermServer : public AioTcpServer, Timer
{
    TermServer(string listen_addr, unsigned int back_log)
        : AioTcpServer(listen_addr,back_log)
        , Timer(1000)
        , psm_ctx_(0)
        , last_time_(get_up_time())
        , total_connections_(0),new_connections_(0),reject_connections_(0),disconnections_(0)
        , read_count_(0)
    {
    }

    virtual ~TermServer()
    {
    }

    virtual string IdString()
    {
        return "TermServer";
    }

    virtual bool Start()
    {
        LOCK_BEGIN(lock,mt_);
        total_connections_  = 0;
        new_connections_    = 0;
        reject_connections_ = 0;
        disconnections_     = 0;
        LOCK_END();

        Activate(true);

        return AioTcpServer::Start();
    }

    virtual void Stop()
    {
        AioTcpServer::Stop();
        Activate(false);
    }

    string Addr() const { return listen_addr_; }
    string ip_string() const { return ::ip_string(listen_addr_); }

    PSMContext *psm_ctx_;

protected:
    virtual AioConnection* OnConnected(TCPConnection* tcp);
    virtual void OnDisconnected(AioConnection* conn);

    virtual void OnDataReceived(AioConnection* conn)
    {
        read_count_++;
    }

    virtual void OnPacketParsed(AioConnection* conn, void* object)
    {
    
    }

    virtual void OnTimer()
    {
        double   now = get_up_time();
        double   lt  = 0;
        uint32_t tc  = 0;
        uint32_t nc  = 0;
        uint32_t dc  = 0;
        uint32_t rc  = 0;

        LOCK_BEGIN(lock,mt_);
        lt  = last_time_;
        tc  = total_connections_;
        nc  = new_connections_;
        dc  = disconnections_;
        rc  = reject_connections_;
        last_time_          = now;
        new_connections_    = 0;
        reject_connections_ = 0;
        disconnections_     = 0;
        LOCK_END();

		/*
        LOG_INFO( "[%s:TCP] dur=%8.3lf(sec) total=%d new=%d dis=%d rej=%d pkt=%d",
                  IdString().c_str(),
                  now-lt, tc, nc, dc, rc, pkt );
		*/

        LOCK_BEGIN(lockd,connection_mt_);
        for ( list<TermConnection*>::iterator it = disconnect_list_.begin(); it != disconnect_list_.end(); ) {
            TermConnection* conn = *it;
            list<TermConnection*>::iterator tmp = it++;

            if ( conn->IsDirty() && conn->IsReferenced() == false ) {
                all_conn_map_.erase(conn);
                disconnect_list_.erase(tmp);
                delete conn;
            }
        }
        LOCK_END(); 
    }

    Mutex    mt_;
    double   last_time_;
    uint32_t total_connections_;
    uint32_t new_connections_;
    uint32_t reject_connections_;
    uint32_t disconnections_;
    uint32_t read_count_;

    Mutex connection_mt_;
    list<TermConnection*> disconnect_list_;
    map<TermConnection*,TermConnection*> all_conn_map_;
};

#endif // !gehua_tcpserver_terminal_server_h_
