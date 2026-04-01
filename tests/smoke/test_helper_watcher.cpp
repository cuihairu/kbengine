#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "helper/watcher.h"

namespace
{
class TestWatcherOwner
{
public:
  int value() const
  {
    return 42;
  }
};
}

TEST(HelperWatcherBootstrapTest, AddsAndFindsWatcherByPath)
{
  KBEngine::WatcherPaths::finalise();

  int watched_value = 7;
  auto* watcher = KBEngine::addWatcher("metrics/test_value", watched_value);
  ASSERT_NE(watcher, nullptr);

  auto found = KBEngine::WatcherPaths::root().getWatcher("root/metrics/test_value");
  ASSERT_TRUE(static_cast<bool>(found));
  EXPECT_STREQ(found->name(), "test_value");
  EXPECT_STREQ(found->path(), "root/metrics");

  KBEngine::WatcherPaths::finalise();
}

TEST(HelperWatcherBootstrapTest, SupportsMethodWatcherAndDirectoryListing)
{
  KBEngine::WatcherPaths::finalise();

  TestWatcherOwner owner;
  ASSERT_NE(KBEngine::addWatcher("services/value", &owner, &TestWatcherOwner::value), nullptr);

  std::vector<std::string> entries;
  KBEngine::WatcherPaths::root().dirPath("root/services", entries);
  ASSERT_EQ(entries.size(), 1u);
  EXPECT_EQ(entries.front(), "value");

  KBEngine::WatcherPaths::finalise();
}

TEST(HelperWatcherBootstrapTest, ReconstructsWatcherFromSerializedStream)
{
  KBEngine::WatcherPaths::finalise();

  KBEngine::MemoryStream stream;
  stream << std::string("rebuilt");

  auto* watcher = KBEngine::WatcherPaths::root().addWatcherFromStream(
    "root/runtime", "state", 99, WATCHER_VALUE_TYPE_STRING, &stream);

  ASSERT_NE(watcher, nullptr);
  EXPECT_STREQ(watcher->name(), "state");
  EXPECT_STREQ(watcher->path(), "root/runtime");
  EXPECT_EQ(std::string(watcher->getValue()), "rebuilt");

  KBEngine::WatcherPaths::finalise();
}
