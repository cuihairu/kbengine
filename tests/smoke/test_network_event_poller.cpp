#include <gtest/gtest.h>

#include "network/event_poller.h"

TEST(NetworkEventPollerBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(NetworkEventPollerBootstrapTest, CreatePoller)
{
  // EventPoller::create() returns a SelectPoller on macOS (no epoll)
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);
  delete poller;
}
