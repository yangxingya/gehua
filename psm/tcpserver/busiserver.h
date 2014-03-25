
/*
* @brief: business server, it is tcp server, not http server.
*/

#if !defined gehua_tcpserver_business_server_h_
#define gehua_tcpserver_business_server_h_

#include "../timeouttimer.h"
#include "../../common/widget.h"
#include "busiconnection.h"

using ::std::list;
using ::std::map;

struct PSMContext;
struct BusiConnection;
struct BusiServer : public AioTcpServer
{
public:
    BusiServer(string listen_addr, unsigned int back_log)
        : AioTcpServer(listen_addr,back_log)
        , last_time_(get_up_time())
        , total_connections_(0),new_connections_(0),reject_connections_(0),disconnections_(0)
        , read_count_(0)
        , psm_ctx_(0)
    {
    }

    virtual string IdString() { return "BusiServer"; } 
    virtual AioConnection* OnConnected(TCPConnection* tcp);

    virtual void OnDisconnected( AioConnection* conn );
    virtual void OnDataReceived( AioConnection* conn );

    BusiConnection* GetConnection(uint64_t business_id);

    double   last_time_;
    uint32_t total_connections_;
    uint32_t new_connections_;
    uint32_t reject_connections_;
    uint32_t disconnections_;
    uint32_t read_count_;

    PSMContext *psm_ctx_;
    string Addr() const { return listen_addr_; }
    string ip_string() const { return ::ip_string(listen_addr_); }
private:
    list<BusiConnection*> conn_list_;

    /* one business id map one connection. for fast find
    * business connection so use map.
    */
    map<uint64_t, BusiConnection*> business_id_conn_map_;

    /* sm ip <-> connection is 1 <-> m relation. so use
    * ip map all connection on the same ip 
    */
    map<uint32_t, list<BusiConnection*> > ip_conn_list_map_;

    TimeOutTimer conn_timeout_timer_;
private:
    BusiServer(BusiServer const&);
    BusiServer& operator=(BusiServer const&);
};

#endif //!gehua_tcpserver_business_server_h_
