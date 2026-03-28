#include <gtest/gtest.h>

#include "helper/eventhistory_stats.h"

TEST(HelperEventStatsBootstrapTest, AggregatesEventsByFullName)
{
  KBEngine::EventHistoryStats stats("network");
  stats.trackEvent("send", "Login", 128);
  stats.trackEvent("send", "Login", 64);

  auto& values = stats.stats();
  ASSERT_EQ(values.size(), 1u);

  const auto iter = values.find("send.Login");
  ASSERT_NE(iter, values.end());
  EXPECT_EQ(iter->second.name, "send.Login");
  EXPECT_EQ(iter->second.size, 192u);
  EXPECT_EQ(iter->second.count, 2u);
}

TEST(HelperEventStatsBootstrapTest, TracksIndependentEventKeys)
{
  KBEngine::EventHistoryStats stats("console");
  stats.trackEvent("recv", "Query", 32, ":");
  stats.trackEvent("recv", "Answer", 48, ":");

  auto& values = stats.stats();
  ASSERT_EQ(values.size(), 2u);
  EXPECT_NE(values.find("recv:Query"), values.end());
  EXPECT_NE(values.find("recv:Answer"), values.end());
}

TEST(HelperEventStatsBootstrapTest, LargeMessagesStillUpdateCounters)
{
  KBEngine::EventHistoryStats stats("payload");
  stats.trackEvent("send", "Huge", 70000);

  const auto iter = stats.stats().find("send.Huge");
  ASSERT_NE(iter, stats.stats().end());
  EXPECT_EQ(iter->second.size, 70000u);
  EXPECT_EQ(iter->second.count, 1u);
}
