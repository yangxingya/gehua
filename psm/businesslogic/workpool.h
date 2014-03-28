
/*
* @breif: work pool 
*/

#if !defined gehua_businesslogic_work_pool_h_
#define gehua_businesslogic_work_pool_h_

#include <cpplib/logger.h>
#include <cpplib/workqueue.h>

struct WorkPool 
{
    WorkPool(Logger& logger, int sz = 1);
	~WorkPool();

    bool Start();
    void Stop();

    void Assign(Work *wk, int id);
    void Assign(DelayedWork *dwk, int id);

    void CancelAllWork();
private:
    Logger &logger_;
    bool started_;
    vector<WorkQueue*> wq_array_;
    vector<WorkQueue*> delay_wq_array_;
};

inline 
WorkPool::WorkPool(Logger& logger, int sz)
: logger_(logger), started_(false)
{
    for (int i = 0; i < sz; ++i) {
        wq_array_.push_back(new WorkQueue);
        delay_wq_array_.push_back(new WorkQueue);
    }
}

inline 
WorkPool::~WorkPool()
{
	for (size_t i = 0; i < wq_array_.size(); ++i) {
		delete wq_array_[i];
		delete delay_wq_array_[i];
	}
}

inline 
bool WorkPool::Start()
{
    if (!started_) {
        for (size_t i = 0; i < wq_array_.size(); ++i) {
            wq_array_[i]->Start();
            delay_wq_array_[i]->Start();
        }

        started_ = true;
    }

    return started_;
}

inline 
void WorkPool::Stop()
{
    if (started_) {
        for (size_t i = 0; i < wq_array_.size(); ++i) {
            wq_array_[i]->Stop();
            delay_wq_array_[i]->Stop();
        }
        started_ = false;  
    }
}

inline
void WorkPool::Assign(Work *wk, int id)
{
    assert(id >= 0);
    assert(id < (int)wq_array_.size());

    wq_array_[id]->QueueWork(wk);
}

inline
void WorkPool::Assign(DelayedWork *dwk, int id)
{
    assert(id >= 0);
    assert(id < (int)delay_wq_array_.size());

    delay_wq_array_[id]->QueueWork(dwk);
}

inline 
void WorkPool::CancelAllWork()
{
    for (size_t i = 0; i < wq_array_.size(); ++i) {
		wq_array_[i]->Flush();
		delay_wq_array_[i]->Flush();
    }
}


#endif // !gehua_businesslogic_work_pool_h_