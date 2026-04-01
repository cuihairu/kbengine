#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "common/timestamp.h"

TEST(CommonTimestampBootstrapTest, TimestampIsMonotonicEnoughForShortSleep) {
  const auto before = KBEngine::timestamp();
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  const auto after = KBEngine::timestamp();

  EXPECT_GT(after, before);
}

TEST(CommonTimestampBootstrapTest, StampConversionIsConsistent) {
  const auto per_second = KBEngine::stampsPerSecond();
  const auto per_second_double = KBEngine::stampsPerSecondD();

  EXPECT_GT(per_second, 0u);
  EXPECT_GT(per_second_double, 0.0);

  const auto half_second = KBEngine::TimeStamp::fromSeconds(0.5);
  EXPECT_NEAR(KBEngine::TimeStamp::toSeconds(half_second), 0.5, 0.05);
}

TEST(CommonTimestampBootstrapTest, TimingMethodNameIsAvailable) {
  const char* name = KBEngine::getTimingMethodName();

  ASSERT_NE(name, nullptr);
  EXPECT_STRNE(name, "");
}
