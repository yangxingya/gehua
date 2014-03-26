/*
* @brief: terminal server, tcp server, not http server
*/

#if !defined gehua_tcpserver_terminal_server_h_
#define gehua_tcpserver_terminal_server_h_

#include <cpplib/netaio/aiomanager.h>
#include "../../common/widget.h"
#include "termconnection.h"

struct PSMContext;
struct TermServer : public AioTcpServer
{
    TermServer(string listen_addr, unsigned int back_log)
        : AioTcpServer(listen_addr,back_log)
        , last_time_(get_up_time())
        , total_connections_(0),new_connections_(0),reject_connections_(0),disconnections_(0)
        , read_count_(0)
        , psm_ctx_(0)
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
        total_connections_  = 0;
        new_connections_    = 0;
        reject_connections_ = 0;
        disconnections_     = 0;
        read_count_         = 0;

        return AioTcpServer::Start();
    }

    virtual void Stop()
    {
        AioTcpServer::Stop();
    }

    virtual AioConnection* OnConnected(TCPConnection* tcp);
    virtual void OnDisconnected(AioConnection* conn);

    virtual void OnDataReceived(AioConnection* conn)
    {
        read_count_++;
    }

    double   last_time_;
    uint32_t total_connections_;
    uint32_t new_connections_;
    uint32_t reject_connections_;
    uint32_t disconnections_;
    uint32_t read_count_;

    PSMContext *psm_ctx_;

    string Addr() const { return listen_addr_; }
    string ip_string() const { return ::ip_string(listen_addr_); }
};

#endif // !gehua_tcpserver_terminal_server_h_
