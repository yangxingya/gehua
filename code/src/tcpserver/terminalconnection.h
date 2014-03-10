/*
 * @brief: terminal connection, tcp server connection.
 */

#if !defined gehua_tcpserver_terminal_connection_h_
#define gehua_tcpserver_terminal_connection_h_

namespace gehua {
namespace tcpserver {

struct TerminalConnection // : public AioConnection
{
	uint64_t terminal_session_id;
	TerminalSession *terminal_session;
	
	caid_t caid;
	CASession *ca_session;

	int timeout;
	int last_recv_tk;
};

} // namespace tcpserver
} // namespace gehua

#endif // !gehua_tcpserver_terminal_connection_h_
