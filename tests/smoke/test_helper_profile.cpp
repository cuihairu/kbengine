#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "helper/profile.h"

TEST(HelperProfileBootstrapTest, RecordsElapsedTimeAndCount)
{
  KBEngine::ProfileGroup group("smoke");
  KBEngine::ProfileVal profile("work", &group);

  profile.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  profile.stop();

  EXPECT_EQ(profile.count(), 1u);
  EXPECT_GT(profile.lastTime(), 0u);
  EXPECT_GE(profile.sumTime(), profile.lastTime());
  EXPECT_FALSE(profile.running());
}

TEST(HelperProfileBootstrapTest, DefaultGroupRunningTimeAdvances)
{
  const auto before = KBEngine::runningTime();
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  const auto after = KBEngine::runningTime();

  EXPECT_GE(after, before);
}
