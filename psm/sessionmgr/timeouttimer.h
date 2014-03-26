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
#include "../tcpserver/termconnection.h"

using ::std::tr1::weak_ptr;
using ::std::tr1::shared_ptr;

struct TimeOutTimer : public Timer
{
    TimeOutTimer(Logger &logger) : Timer(), logger_(logger) {}
    virtual ~TimeOutTimer() {}

    void Add(weak_ptr<TermSession> ts)
    {
        shared_ptr<TermSession> sp_ts(ts.lock());
        if (sp_ts) {
            MutexLock lock(mutex_);
            termsession_list_[sp_ts->Id()] = sp_ts;
        }
    }

    void Remove(uint64_t termsession_id)
    {
        map<uint64_t, weak_ptr<TermSession> >::iterator it;

        MutexLock lock(mutex_);
        if ((it = termsession_list_.find(termsession_id)) 
            != termsession_list_.end()) {
                termsession_list_.erase(it);
        }

    }

protected:
    virtual void OnTimer()
    {
        int    timeout;
        double last;
        double curr = get_up_time();

        MutexLock lock(mutex_);
        
        map<uint64_t, weak_ptr<TermSession> >::iterator
            it = termsession_list_.begin();
        for (/* without initialize */; it != termsession_list_.end(); /* without condtion */) {
            shared_ptr<TermSession> sp_it(it->second.lock());
            if (sp_it) {
                last    = sp_it->term_conn_->last_heartbeat_time_;
                timeout = sp_it->term_conn_->timeout_;
                if ((int)(curr - last) > timeout) {
                    //超时处理...添加到工作队列里面？

                } else {
                    
                }
                it++;
            } else {
                //删除无效会话。
                termsession_list_.erase(it++);
            }
        }

        logger_.Trace("[超时计时器] 循环遍历处理时间%.3f", get_up_time() - curr);
    }

private:
    Logger& logger_;
    Mutex   mutex_;
    map<uint64_t, weak_ptr<TermSession> > termsession_list_;
};

#endif // !gehua_timeouttimer_h_
