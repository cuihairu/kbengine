#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "thread/threadpool.h"

namespace
{
class CountingTPTask : public KBEngine::thread::TPTask
{
public:
  CountingTPTask(std::atomic<int>& worker_runs, std::atomic<int>& main_runs) :
    worker_runs_(worker_runs),
    main_runs_(main_runs)
  {
  }

  bool process() override
  {
    worker_runs_.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  TPTaskState presentMainThread() override
  {
    main_runs_.fetch_add(1, std::memory_order_relaxed);
    return TPTASK_STATE_COMPLETED;
  }

private:
  std::atomic<int>& worker_runs_;
  std::atomic<int>& main_runs_;
};
}

TEST(ThreadPoolBootstrapTest, ProcessesTaskOnWorkerAndCompletesOnMainThread)
{
  std::atomic<int> worker_runs{0};
  std::atomic<int> main_runs{0};

  KBEngine::thread::ThreadPool pool;
  ASSERT_TRUE(pool.createThreadPool(1, 1, 1));

  ASSERT_TRUE(pool.addTask(new CountingTPTask(worker_runs, main_runs)));

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (std::chrono::steady_clock::now() < deadline)
  {
    pool.onMainThreadTick();

    if (worker_runs.load(std::memory_order_relaxed) == 1 &&
        main_runs.load(std::memory_order_relaxed) == 1)
    {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_EQ(worker_runs.load(std::memory_order_relaxed), 1);
  EXPECT_EQ(main_runs.load(std::memory_order_relaxed), 1);

  pool.finalise();
}
