#include <gtest/gtest.h>

// serverconfig.inl requires full Network::Address definition
#include "network/common.h"
#include "network/address.h"
#include "server/shutdowner.h"

TEST(ServerShutdownerBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}
