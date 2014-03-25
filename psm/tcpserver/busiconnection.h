/*
* @brief: business connection for business server.
*/

#if !defined gehua_tcpserver_business_connection_h_
#define gehua_tcpserver_business_connection_h_

#include <cpplib/logger.h>
#include <cpplib/netaio/aiomanager.h>

struct PbBase;
struct PSMContext;

struct BusiConnection : public AioConnection
{
    BusiConnection(ILogger* logger, TCPConnection* tcp, AioTcpServer* server) 
        : AioConnection(tcp,server)
        , logger_(logger)
    {
        client_ip_ = tcp->GetPeer().GetAddress().s_addr;
    }

    virtual ~BusiConnection()
    {
    }

    virtual void OnDisconnected()
    {
    }

    virtual void OnDataReceived();

    int parsePacket(PbBase** msg, uint32_t* len);
    // get client ip use 4bytes "192.168.19.1" -> 4 bytes
    // every byte is 192, 168, 19, 1. so it is a 4bytes 
    // number.
    // todo :: need base class supported it.
    uint32_t ClientIp() const { return client_ip_; }

    string ip_string() const 
    {   
        struct in_addr addr;
        addr.s_addr = client_ip_;
        return inet_ntoa(addr);
    }

    ILogger* logger_;

    uint32_t timeout;
    uint32_t last_recv_tk;

    PSMContext *psm_ctx_;

private:
    uint32_t client_ip_;
};

#endif // !gehua_tcpserver_business_connection_h_
