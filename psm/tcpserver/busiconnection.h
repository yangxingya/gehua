/*
* @brief: business connection for business server.
*/

#if !defined gehua_tcpserver_business_connection_h_
#define gehua_tcpserver_business_connection_h_

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

struct BusiConnection : public AioConnection
{
    BusiConnection(TCPConnection* tcp, AioTcpServer* server) 
        : AioConnection(tcp,server)
    {
    }

    virtual ~BusiConnection()
    {
    }

    virtual void OnDisconnected()
    {
    }

    virtual void OnDataReceived()
    {

    }
    // get client ip use 4bytes "192.168.19.1" -> 4 bytes
    // every byte is 192, 168, 19, 1. so it is a 4bytes 
    // number.
    // todo :: need base class supported it.
    uint32_t ClientIp() const;

    uint32_t timeout;
    uint32_t last_recv_tk;

    PSMContext *psm_ctx_;
};

#endif // !gehua_tcpserver_business_connection_h_
