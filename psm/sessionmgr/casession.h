/*
* @brief: ca session.
*
*/

#if !defined gehua_sessionmgr_ca_session_h_
#define gehua_sessionmgr_ca_session_h_

#include <cpplib/cpplibbase.h>
#include "../../common/widget.h"
#include "../../common/portable.h"

enum CATermMapStatus
{
	CATMS_Adding	= 1,
	CATMS_Removing	= 2,
	CATMS_Searching = 3,
	CATMS_Idle		= 0,
	CATMS_Invalided = 0xFF,
};

struct TermConnection;
struct TimeOutTimer;
struct TermSession;
struct CASession
{
public:
    CASession(caid_t caid, TimeOutTimer& timeout_timer);
    void Add(weak_ptr<TermSession> terminal_session);
    void Remove(uint64_t terminal_session_id);

    caid_t Id() const { return caid_; }
    size_t termCnt() const 
	{	
		MutexLock lock(termsession_mtx_);
		return terminal_session_map_.size(); 
	}

    weak_ptr<TermSession> GetSTBTermSession();


	mutable Mutex termsession_mtx_;
    map<uint64_t, shared_ptr<TermSession> > terminal_session_map_;
private:
    caid_t caid_;
    TimeOutTimer& timeout_timer_;

    friend struct CASessionMgr;
};

#endif //!gehua_sessionmgr_ca_session_h_
