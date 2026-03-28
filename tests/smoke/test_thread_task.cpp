#include <gtest/gtest.h>

#include "thread/threadtask.h"

namespace
{
class CompletedTask : public KBEngine::thread::TPTask
{
public:
  bool process() override
  {
    ++process_calls_;
    return false;
  }

  int process_calls() const
  {
    return process_calls_;
  }

private:
  int process_calls_ = 0;
};

class ContinueOnMainThreadTask : public KBEngine::thread::TPTask
{
public:
  bool process() override
  {
    return true;
  }

  TPTaskState presentMainThread() override
  {
    return TPTASK_STATE_CONTINUE_MAINTHREAD;
  }
};
}

TEST(ThreadTaskBootstrapTest, DefaultPresentMainThreadReturnsCompleted)
{
  CompletedTask task;
  EXPECT_EQ(task.presentMainThread(), KBEngine::thread::TPTask::TPTASK_STATE_COMPLETED);
  EXPECT_FALSE(task.process());
  EXPECT_EQ(task.process_calls(), 1);
}

TEST(ThreadTaskBootstrapTest, DerivedTaskCanRequestMainThreadContinuation)
{
  ContinueOnMainThreadTask task;
  EXPECT_EQ(task.presentMainThread(), KBEngine::thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD);
  EXPECT_TRUE(task.process());
}
