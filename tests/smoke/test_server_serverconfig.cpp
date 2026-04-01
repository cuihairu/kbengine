#include <gtest/gtest.h>

// serverconfig.inl requires full Network::Address definition
#include "network/address.h"
#include "network/common.h"
#include "server/serverconfig.h"

TEST(ServerServerConfigBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(ServerServerConfigBootstrapTest, SingletonAccess)
{
  new KBEngine::ServerConfig();
  KBEngine::ServerConfig& config = KBEngine::ServerConfig::getSingleton();
  (void)config;
  delete &config;
}
