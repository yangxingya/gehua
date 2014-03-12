/*
 * @brief: terminal connection, tcp server connection.
 */

#if !defined gehua_tcpserver_terminal_connection_h_
#define gehua_tcpserver_terminal_connection_h_

#include <cpplib/logger.h>
#include <cpplib/netaio/aiobase.h>
#include <cpplib/netaio/aioconnection.h>
#include <cpplib/netaio/aioprotocolbase.h>
#include <cpplib/netaio/aiotcpclient.h>
#include <cpplib/netaio/aiotcpserver.h>
#include <cpplib/netaio/aioreceiver.h>
#include <cpplib/netaio/aiosender.h>
#include <cpplib/netaio/aiomanager.h>

struct PSMContext;
struct TermSession;
struct PtBase;
struct TermConnection : public AioConnection
{
    TermConnection(TCPConnection* tcp, AioTcpServer* server) 
        : AioConnection(tcp,server)
        , logger_(0)
        , create_time_(get_up_time()),last_heartbeat_time_(0) 
        , term_session_(0)
        , timeout_(0), login_(false)
    {
    }

    virtual ~TermConnection()
    {
    }

    virtual void OnDisconnected()
    {
    }

    virtual void OnDataReceived();

    int parsePacket(PtBase** msg, uint32_t* len);
    void SendHeartbeatResponse(string request_str);

    ILogger* logger_;
    double   create_time_;
    double   last_heartbeat_time_;

    /*sessionmgr::*/TermSession *term_session_;

	  int timeout_;

    /*psm::*/PSMContext *psm_ctx_;

private:
    // is login ??? flags.
    bool login_;
    TermConnection(TermConnection const&);
    TermConnection& operator=(TermConnection const&);
};

#endif // !gehua_tcpserver_terminal_connection_h_
