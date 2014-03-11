/*
 * @brief: terminal server, tcp server, not http server
 */

#if !defined gehua_tcpserver_terminal_server_h_
#define gehua_tcpserver_terminal_server_h_

#include <cpplib/logger.h>
#include <cpplib/netaio/aiobase.h>
#include <cpplib/netaio/aioconnection.h>
#include <cpplib/netaio/aioprotocolbase.h>
#include <cpplib/netaio/aiotcpclient.h>
#include <cpplib/netaio/aiotcpserver.h>
#include <cpplib/netaio/aioreceiver.h>
#include <cpplib/netaio/aiosender.h>
#include <cpplib/netaio/aiomanager.h>
#include "terminalconnection.h"

namespace gehua {
namespace tcpserver {

struct PSMContext;
struct TerminalServer : public AioTcpServer
{
    TerminalServer(string listen_addr, unsigned int back_log, unsigned int wq_count=1 )
        : AioTcpServer(listen_addr,back_log,wq_count)
        , last_time_(get_up_time())
        , total_connections_(0),new_connections_(0),reject_connections_(0),disconnections_(0)
        , read_count_(0)
        , psm_ctx_(0)
    {
        SetLogger(&g_logger);
    }

    virtual ~TerminalServer()
    {
    }

    virtual string IdString()
    {
        return "TerminalServer";
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

    virtual AioConnection* OnConnected( TCPConnection* tcp )
    {
        TerminalConnection* new_conn = new TerminalConnection(tcp, this);

        //set psm context.
        new_conn->SetPSMContext(psm_ctx_);

        if (new_conn) {
            total_connections_ ++;
            new_connections_ ++;
        } else {
            reject_connections_++;
        }
        return (AioConnection*)new_conn;
    }

    virtual void OnDisconnected(AioConnection* conn)
    {
        conn->SetDirty();
        disconnections_++;
        total_connections_--;
    }

    virtual void OnDataReceived(AioConnection* conn)
    {
        read_count_++;
    }

    void SetPSMContext(PSMContext *psm_ctx)
    {
        psm_ctx_ = psm_ctx;
    }

    double   last_time_;
    uint32_t total_connections_;
    uint32_t new_connections_;
    uint32_t reject_connections_;
    uint32_t disconnections_;
    uint32_t read_count_;

    PSMContext *psm_ctx_;
};

} // namespace tcpserver
} // namespace gehua

#endif // !gehua_tcpserver_terminal_server_h_
