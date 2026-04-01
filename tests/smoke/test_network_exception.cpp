#include <gtest/gtest.h>

#include "network/network_exception.h"

TEST(NetworkExceptionBootstrapTest, ExposesReasonWithoutAddress)
{
  KBEngine::Network::NetworkException exception(KBEngine::Network::REASON_CHANNEL_LOST);

  KBEngine::Network::Address address(1, 2);
  EXPECT_EQ(exception.reason(), KBEngine::Network::REASON_CHANNEL_LOST);
  EXPECT_FALSE(exception.getAddress(address));
  EXPECT_TRUE(address.isNone());
}

TEST(NetworkExceptionBootstrapTest, CopiesOffendingAddressWhenPresent)
{
  KBEngine::Network::Address original("127.0.0.1", 32000);
  KBEngine::Network::NetworkException exception(
    KBEngine::Network::REASON_RESOURCE_UNAVAILABLE,
    original);

  KBEngine::Network::Address copied;
  ASSERT_TRUE(exception.getAddress(copied));
  EXPECT_EQ(copied, original);
  EXPECT_EQ(exception.reason(), KBEngine::Network::REASON_RESOURCE_UNAVAILABLE);
}
