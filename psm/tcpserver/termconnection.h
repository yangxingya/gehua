/*
* @brief: terminal connection, tcp server connection.
*/

#if !defined gehua_tcpserver_terminal_connection_h_
#define gehua_tcpserver_terminal_connection_h_

#include <cpplib/logger.h>
#include <cpplib/netaio/aiomanager.h>
#include "../../common/portable.h"

struct PSMContext;
struct TermSession;
struct PtBase;
struct PtLoginRequest;
struct TermConnection : public AioConnection
{
    TermConnection(ILogger* logger, TCPConnection* tcp, AioTcpServer* server) 
        : AioConnection(tcp,server)
        , logger_(logger)
        , create_time_(get_up_time()),last_heartbeat_time_(get_up_time()) 
        , timeout_(3 * 60), login_(false)
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

    //TermSession *term_session_;
    weak_ptr<TermSession> term_session_;

    int timeout_;

    PSMContext *psm_ctx_;

private:
    // is login ??? flags.
    bool login_;
    TermConnection(TermConnection const&);
    TermConnection& operator=(TermConnection const&);

	bool reused_termsession(uint64_t ts_id);
	bool gen_termsession(PtLoginRequest *msg);
	bool valid_login(PtLoginRequest *msg);
};

#endif // !gehua_tcpserver_terminal_connection_h_
