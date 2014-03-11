
/*
 * @breif: work pool 
 */

#if !defined gehua_businesslogic_work_pool_h_
#define gehua_businesslogic_work_pool_h_

#include <assert.h>
#include <vector>
#include <cpplib/logger.h>
#include <cpplib/workqueue.h>

namespace gehua {
namespace businesslogic {

using ::std::vector;

struct WorkPool 
{
  WorkPool(Logger& logger, int sz = 1);

  void Assign(Work *wk, int id);
  void Assign(DelayWork *dwk, int id);
private:
  Logger &logger_;
  vector<WorkQueue> wq_array_;
  vector<WorkQueue> delay_wq_array_;
};

inline 
void WorkPool::WorkPool(Logger& logger, int sz)
  : logger_(logger)
{
  wq_array_.resize(sz);
  delay_wq_array_.resize(sz);
}

inline
void WorkPool::Assign(Work *wk, int id)
{
  assert(id >= 0);
  assert(id < wq_array_.size());

  wq_array_[id].QueueWork(wk);
}

inline
void WorkPool::Assign(DelayWork *dwk, int id)
{
  assert(id >= 0);
  assert(id < delay_wq_array_.size());

  delay_wq_array_[id].QueueWork(dwk);
}

} // namespace businesslogic
} // namespace gehua


#endif // !gehua_businesslogic_work_pool_h_