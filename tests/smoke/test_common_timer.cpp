#include <gtest/gtest.h>

#include "common/timer.h"

namespace {

class CountingTimerHandler : public KBEngine::TimerHandler {
public:
  void handleTimeout(KBEngine::TimerHandle handle, void* pUser) override {
    ++timeout_count;
    last_user = pUser;
    last_handle = handle;
  }

  void onRelease(KBEngine::TimerHandle handle, void* pUser) override {
    ++release_count;
    released_user = pUser;
    released_handle = handle;
  }

  int timeout_count = 0;
  int release_count = 0;
  void* last_user = nullptr;
  void* released_user = nullptr;
  KBEngine::TimerHandle last_handle;
  KBEngine::TimerHandle released_handle;
};

} // namespace

TEST(CommonTimerBootstrapTest, FiresOneShotAndReleasesHandler) {
  KBEngine::Timers64 timers;
  CountingTimerHandler handler;
  int user_value = 7;

  const auto handle = timers.add(10, 0, &handler, &user_value);

  EXPECT_TRUE(timers.legal(handle));
  EXPECT_EQ(timers.process(9), 0);
  EXPECT_EQ(handler.timeout_count, 0);

  EXPECT_EQ(timers.process(10), 1);
  EXPECT_EQ(handler.timeout_count, 1);
  EXPECT_EQ(handler.release_count, 1);
  EXPECT_EQ(handler.last_user, &user_value);
  EXPECT_EQ(handler.released_user, &user_value);
  EXPECT_FALSE(timers.legal(handle));
}

TEST(CommonTimerBootstrapTest, RepeatingTimerCanBeCancelled) {
  KBEngine::Timers64 timers;
  CountingTimerHandler handler;

  auto handle = timers.add(5, 5, &handler, nullptr);

  EXPECT_EQ(timers.process(5), 1);
  EXPECT_EQ(handler.timeout_count, 1);
  EXPECT_EQ(handler.release_count, 0);
  EXPECT_TRUE(timers.legal(handle));
  EXPECT_EQ(timers.nextExp(5), 5u);

  handle.cancel();
  EXPECT_EQ(handler.release_count, 1);
  EXPECT_FALSE(timers.legal(handle));
  EXPECT_EQ(timers.process(10), 0);
}

TEST(CommonTimerBootstrapTest, ExposesTimerInfoBeforeCancellation) {
  KBEngine::Timers64 timers;
  CountingTimerHandler handler;
  int user_value = 99;

  auto handle = timers.add(12, 3, &handler, &user_value);
  KBEngine::Timers64::TimeStamp time = 0;
  KBEngine::Timers64::TimeStamp interval = 0;
  void* user = nullptr;

  ASSERT_TRUE(timers.getTimerInfo(handle, time, interval, user));
  EXPECT_EQ(time, 12u);
  EXPECT_EQ(interval, 3u);
  EXPECT_EQ(user, &user_value);

  handle.cancel();
  EXPECT_FALSE(timers.getTimerInfo(handle, time, interval, user));
}
