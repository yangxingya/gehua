/*
* @brief: connection timeout timer.
*/

#if !defined gehua_timeouttimer_h_
#define gehua_timeouttimer_h_

#include <memory>
#include <cpplib/timer.h>
#include <cpplib/mutex.h>
#include <cpplib/mutexlock.h>
#include "termsession.h"

using ::std::tr1::weak_ptr;
using ::std::tr1::shared_ptr;

const int kDefaultTimeout = 3 * 60;

struct TermSession;
struct PSMContext;

struct TimeOutTimer : public Timer
{
    TimeOutTimer(Logger &logger, PSMContext *psm_ctx) : Timer(), logger_(logger), psm_ctx_(psm_ctx) {}
    virtual ~TimeOutTimer() {}
    void Add(weak_ptr<TermSession> ts);
    void Remove(uint64_t termsession_id);
protected:
    virtual void OnTimer();
private:
    Logger      &logger_;
	PSMContext  *psm_ctx_;
    Mutex       mutex_;
    map<uint64_t, weak_ptr<TermSession> > termsession_list_;
};

#endif // !gehua_timeouttimer_h_
