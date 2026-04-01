#include <gtest/gtest.h>

#include "network/ikcp.h"

TEST(NetworkIkcpBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(NetworkIkcpBootstrapTest, CreateKcpSession)
{
  ikcpcb* kcp = ikcp_create(0x1234, nullptr);
  ASSERT_NE(kcp, nullptr);
  EXPECT_EQ(kcp->conv, 0x1234);
  ikcp_release(kcp);
}
