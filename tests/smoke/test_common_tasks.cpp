#include <gtest/gtest.h>

#include "common/tasks.h"

namespace
{
class CountingTask : public KBEngine::Task
{
public:
  explicit CountingTask(int keep_running_for) :
    keep_running_for_(keep_running_for),
    calls_(0)
  {
  }

  bool process() override
  {
    ++calls_;
    return calls_ < keep_running_for_;
  }

  int calls() const
  {
    return calls_;
  }

private:
  int keep_running_for_;
  int calls_;
};
}

TEST(CommonTasksBootstrapTest, RemovesTaskWhenProcessReturnsFalse)
{
  KBEngine::Tasks tasks;
  CountingTask single_run(1);

  tasks.add(&single_run);
  tasks.process();
  EXPECT_EQ(single_run.calls(), 1);

  tasks.process();
  EXPECT_EQ(single_run.calls(), 1);
}

TEST(CommonTasksBootstrapTest, CancelRemovesPendingTask)
{
  KBEngine::Tasks tasks;
  CountingTask recurring(3);

  tasks.add(&recurring);
  EXPECT_TRUE(tasks.cancel(&recurring));
  EXPECT_FALSE(tasks.cancel(&recurring));

  tasks.process();
  EXPECT_EQ(recurring.calls(), 0);
}

TEST(CommonTasksBootstrapTest, KeepsRecurringTaskUntilItCompletes)
{
  KBEngine::Tasks tasks;
  CountingTask recurring(3);

  tasks.add(&recurring);
  tasks.process();
  tasks.process();
  tasks.process();
  tasks.process();

  EXPECT_EQ(recurring.calls(), 3);
}
