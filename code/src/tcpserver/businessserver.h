
/*
 * @brief: business server, it is tcp server, not http server.
 */

#if !defined gehua_tcpserver_business_server_h_
#define gehua_tcpserver_business_server_h_

#include <list>
#include <map>
#include "../timeouttimer.h"
#include "businessconnection.h"

namespace gehua {
namespace tcpserver {

using ::std::list;
using ::std::map;

struct BusinessConnection;
class BusinessServer // : public AioTcpServer
public:
	BusinessConnection* GetConnection(uint64_t business_id);
private:
	list<BusinessConnection*> conn_list_;
	
	/* one business id map one connection. for fast find
	 * business connection so use map.
	 */
	map<uint64_t, BusinessConnection*> business_id_conn_map_;
	
	/* sm ip <-> connection is 1 <-> m relation. so use
	 * ip map all connection on the same ip 
	 */
	map<uint32_t, list<BusinessConnection*> > ip_conn_list_map_;

	TimeOutTimer conn_timeout_timer_;
} // namespace tcpserver
} // namespace gehua

#endif //!gehua_tcpserver_business_server_h_
