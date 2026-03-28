#include <gtest/gtest.h>

#include "network/bundle.h"

TEST(NetworkBundleBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(NetworkBundleBootstrapTest, CreateBundle)
{
  KBEngine::Network::Bundle bundle;
  (void)bundle;
}
