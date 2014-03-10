/*
 * @brief: business pool, for all the business logic operate pool
 *         it is a mutil-thread pool
 */

#if !defined gehua_businesslogic_business_pool_h_
#define gehua_businesslogic_business_pool_h_

namespace gehua {
namespace businesslogic {

class Work;
class BusinessPool
{
public:
	void AddWork(Work *wk, caid_t caid);
	void AddDelayWork(DelayWork *delay_wk, caid_t caid);

	CASession* CreateCASession(caid_t caid);
	void DestroyCASession(CASession *ca_session);
	
	void Attach(CASession *ca_session);
	CASession* Detach(caid_t caid);

	CASession* FindCASessionById(caid_t caid);
	CASession* FindCASessionByTerminalSessionId(uint64_t terminal_session_id);
private:
	WorkPool wk_pool_;
	vector<CASessionMgr> pool_relation_ca_session_mgr_;

};

} // namespace businesslogic
} // namespace gehua

#endif // !gehua_businesslogic_business_pool_h_
