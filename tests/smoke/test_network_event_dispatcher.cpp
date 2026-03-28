#include <gtest/gtest.h>

#include "network/event_dispatcher.h"

TEST(NetworkEventDispatcherBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(NetworkEventDispatcherBootstrapTest, CreateDispatcher)
{
  KBEngine::Network::EventDispatcher dispatcher;
  // breakProcessing is available in the real EventDispatcher
  dispatcher.breakProcessing(false);
}
