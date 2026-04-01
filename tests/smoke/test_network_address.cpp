#include <gtest/gtest.h>

#include <cstring>

#include "common/common.h"
#include "network/address.h"

// Use cross-platform uint32 type
#ifdef _WIN32
	typedef unsigned long u_int32_t;
#else
	#include <sys/types.h>
#endif

TEST(NetworkAddressBootstrapTest, FormatsAddressAndIpStrings)
{
  KBEngine::Network::Address address("127.0.0.1", 20013);

  EXPECT_STREQ(address.ipAsString(), "127.0.0.1");
  EXPECT_STREQ(address.c_str(), "127.0.0.1:20013");
}

TEST(NetworkAddressBootstrapTest, ConvertsBetweenStringAndNumericIp)
{
  u_int32_t raw = 0;
  ASSERT_EQ(KBEngine::Network::Address::string2ip("127.0.0.1", raw), 0);

  char buffer[32] = {0};
  ASSERT_GT(KBEngine::Network::Address::ip2string(raw, buffer), 0);
  EXPECT_STREQ(buffer, "127.0.0.1");
}

TEST(NetworkAddressBootstrapTest, SupportsLocalhostResolution)
{
  KBEngine::Network::Address address("localhost", 4000);
  EXPECT_STREQ(address.ipAsString(), "127.0.0.1");
}

TEST(NetworkAddressBootstrapTest, ReusesPoolObjectsAfterReclaim)
{
  KBEngine::Network::Address::destroyObjPool();

  KBEngine::Network::Address* first =
    KBEngine::Network::Address::createPoolObject("network-address-test");
  ASSERT_NE(first, nullptr);
  first->ip = 1;
  first->port = 2;

  // Verify the values were set correctly
  EXPECT_EQ(first->ip, 1u);
  EXPECT_EQ(first->port, 2u);

  KBEngine::Network::Address::reclaimPoolObject(first);

  // Note: After reclaim, the object should be reset but we shouldn't access 'first' anymore
  // Instead, create a new object and verify it comes from the pool and is properly initialized
  EXPECT_GE(KBEngine::Network::Address::ObjPool().size(), 1u);
  EXPECT_EQ(KBEngine::Network::Address::ObjPool().logPoints()["network-address-test"].count, 0);

  KBEngine::Network::Address* second =
    KBEngine::Network::Address::createPoolObject("network-address-test");
  ASSERT_NE(second, nullptr);

  // The new object should be properly initialized (either newly created or reused from pool)
  EXPECT_EQ(second->ip, 0u) << "Reused object should have been reset by onReclaimObject()";
  EXPECT_EQ(second->port, 0u) << "Reused object should have been reset by onReclaimObject()";

  KBEngine::Network::Address::reclaimPoolObject(second);
  KBEngine::Network::Address::destroyObjPool();
}
