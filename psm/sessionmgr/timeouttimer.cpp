

#include "timeouttimer.h"
#include "../tcpserver/termconnection.h"
#include "../psmcontext.h"

void TimeOutTimer::Add(weak_ptr<TermSession> ts)
{
	shared_ptr<TermSession> sp_ts(ts.lock());
	if (sp_ts) {
		MutexLock lock(mutex_);
		termsession_list_[sp_ts->Id()] = sp_ts;
	}
}

void TimeOutTimer::Remove(uint64_t termsession_id)
{
	map<uint64_t, weak_ptr<TermSession> >::iterator it;

	MutexLock lock(mutex_);
	if ((it = termsession_list_.find(termsession_id)) 
		!= termsession_list_.end()) {
			termsession_list_.erase(it);
	}

}

void TimeOutTimer::OnTimer()
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
			{
				last = get_up_time();
				timeout = kDefaultTimeout;
				MutexLock lock(sp_it->termconn_mtx_);
				if (sp_it->term_conn_) {
					last    = sp_it->term_conn_->last_heartbeat_time_;
					timeout = sp_it->term_conn_->timeout_;
				}
			}
			if ((int)(curr - last) > timeout) {
				//超时处理...添加到工作队列里面？
				logger_.Warn("[超时计时器]删除超时的终端会话，会话id: 0x"SFMT64X"CAId: "SFMT64U, sp_it->Id(), sp_it->CAId());
				CASession *ca_session = sp_it->ca_session_;
				if (psm_ctx_->busi_pool_->DelTermSession(sp_it) > 0) {
					psm_ctx_->term_basic_func_svr_->NotifyAllTerminalStatusPChanged(ca_session, sp_it->Id());
				}
                MutexLock lock(sp_it->termconn_mtx_);
				if (sp_it->term_conn_) {
                    sp_it->term_conn_->SetDirty();
                }
				termsession_list_.erase(it++);
			} else {
				it++;
			}
		} else {
			//删除无效会话。
			termsession_list_.erase(it++);
		}
	}

	logger_.Trace("[超时计时器] 循环遍历处理时间%.3f", get_up_time() - curr);
}