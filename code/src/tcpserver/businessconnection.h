/*
 * @brief: business connection for business server.
 */

#if !defined gehua_tcpserver_business_connection_h_
#define gehua_tcpserver_business_connection_h_

namespace gehua {
namespace tcpserver {

struct BusinessConnection // : public AioConnection
{
	// get client ip use 4bytes "192.168.19.1" -> 4 bytes
	// every byte is 192, 168, 19, 1. so it is a 4bytes 
	// number.
	// todo :: need base class supported it.
	uint32_t ClientIp() const;

	uint32_t timeout;
	uint32_t last_recv_tk;
};

} // namespace tcpserver
} // namespace gehua

#endif // !gehua_tcpserver_business_connection_h_
