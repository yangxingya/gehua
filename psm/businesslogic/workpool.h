
/*
* @breif: work pool 
*/

#if !defined gehua_businesslogic_work_pool_h_
#define gehua_businesslogic_work_pool_h_

#include <assert.h>
#include <vector>
#include <cpplib/logger.h>
#include <cpplib/workqueue.h>

using ::std::vector;

struct WorkPool 
{
    WorkPool(Logger& logger, int sz = 1);

    void Assign(Work *wk, int id);
    void Assign(DelayedWork *dwk, int id);

    void CancelAllWork();
private:
    Logger &logger_;
    vector<WorkQueue*> wq_array_;
    vector<WorkQueue*> delay_wq_array_;
};

inline 
WorkPool::WorkPool(Logger& logger, int sz)
: logger_(logger)
{
    for (int i = 0; i < sz; ++i) {
        wq_array_.push_back(new WorkQueue);
        delay_wq_array_.push_back(new WorkQueue);
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

}


#endif // !gehua_businesslogic_work_pool_h_