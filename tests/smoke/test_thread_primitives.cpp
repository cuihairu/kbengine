#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "thread/threadguard.h"
#include "thread/threadmutex.h"

namespace
{
class Counter
{
public:
  void increment()
  {
    KBEngine::thread::ThreadGuard guard(&mutex_);
    ++value_;
  }

  int value() const
  {
    return value_;
  }

private:
  KBEngine::thread::ThreadMutex mutex_;
  int value_ = 0;
};
}

TEST(ThreadPrimitivesBootstrapTest, ThreadMutexLockUnlockWorks)
{
  KBEngine::thread::ThreadMutex mutex;
  mutex.lockMutex();
  mutex.unlockMutex();
  SUCCEED();
}

TEST(ThreadPrimitivesBootstrapTest, ThreadGuardProtectsConcurrentIncrements)
{
  Counter counter;
  std::vector<std::thread> threads;

  for (int i = 0; i < 4; ++i)
  {
    threads.emplace_back([&counter]() {
      for (int j = 0; j < 1000; ++j)
      {
        counter.increment();
      }
    });
  }

  for (auto& thread : threads)
  {
    thread.join();
  }

  EXPECT_EQ(counter.value(), 4000);
}
