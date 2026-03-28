#include <gtest/gtest.h>

#include "common/deadline.h"

TEST(CommonDeadlineBootstrapTest, BreaksSecondsIntoTimeUnits)
{
  KBEngine::Deadline deadline(90061);
  EXPECT_EQ(deadline.days, 1u);
  EXPECT_EQ(deadline.hours, 1u);
  EXPECT_EQ(deadline.minutes, 1u);
  EXPECT_EQ(deadline.seconds, 1u);
  EXPECT_EQ(deadline.print(), "1days/1:1:1");
}

TEST(CommonDeadlineBootstrapTest, HandlesZeroDuration)
{
  KBEngine::Deadline deadline(0);
  EXPECT_EQ(deadline.days, 0u);
  EXPECT_EQ(deadline.hours, 0u);
  EXPECT_EQ(deadline.minutes, 0u);
  EXPECT_EQ(deadline.seconds, 0u);
}
