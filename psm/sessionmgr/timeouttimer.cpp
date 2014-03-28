

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
				//��ʱ����...��ӵ������������棿
				logger_.Warn("[��ʱ��ʱ��]ɾ����ʱ���ն˻Ự���Ựid: "SFMT64U"CAId: "SFMT64U, sp_it->Id(), sp_it->CAId());
				psm_ctx_->busi_pool_->DelTermSession(sp_it);
				termsession_list_.erase(it++);
			} else {
				it++;
			}
		} else {
			//ɾ����Ч�Ự��
			termsession_list_.erase(it++);
		}
	}

	logger_.Trace("[��ʱ��ʱ��] ѭ����������ʱ��%.3f", get_up_time() - curr);
}