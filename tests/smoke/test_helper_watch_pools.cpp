#include <gtest/gtest.h>

#include "helper/watch_pools.h"

TEST(HelperWatchPoolsBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(HelperWatchPoolsBootstrapTest, InitAndFini)
{
  EXPECT_TRUE(KBEngine::WatchPool::initWatchPools());
  EXPECT_TRUE(KBEngine::WatchPool::finiWatchPools());
}
