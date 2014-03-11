/*
 * @brief: terminal server, tcp server, not http server
 */

#if !defined gehua_tcpserver_terminal_server_h_
#define gehua_tcpserver_terminal_server_h_

#include <cpplib/netaio/aiobase.h>
#include <cpplib/netaio/aioconnection.h>
#include <cpplib/netaio/aioprotocolbase.h>
#include <cpplib/netaio/aiotcpclient.h>
#include <cpplib/netaio/aiotcpserver.h>
#include <cpplib/netaio/aioreceiver.h>
#include <cpplib/netaio/aiosender.h>
#include <cpplib/netaio/aiomanager.h>

namespace gehua {
namespace tcpserver {

class TerminalServer : public AioTcpServer
{
public:
  TerminalServer(string listen_addr, unsigned int back_log, unsigned int wq_count)
    : AioTcpServer(listen_addr, back_log, wq_count)
  {}

  virtual ~TerminalServer() {}


};

} // namespace tcpserver
} // namespace gehua

#endif // !gehua_tcpserver_terminal_server_h_
