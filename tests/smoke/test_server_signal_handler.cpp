#include <gtest/gtest.h>

#include "server/signal_handler.h"

TEST(ServerSignalHandlerBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(ServerSignalHandlerBootstrapTest, SignalHandlersInstance)
{
  new KBEngine::SignalHandlers();
  KBEngine::SignalHandlers& sh = KBEngine::SignalHandlers::getSingleton();
  sh.clear();
  delete &sh;
}
