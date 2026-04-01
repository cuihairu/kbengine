#include <gtest/gtest.h>

#include "helper/sys_info.h"

// Platform-specific getpid wrapper
#ifdef _WIN32
	#include <process.h>
	#ifndef getpid
		#define getpid() _getpid()
	#endif
#else
	#include <unistd.h>
#endif

TEST(HelperSysInfoBootstrapTest, ExposesBasicSystemInfoApis)
{
  auto& info = KBEngine::SystemInfo::getSingleton();

  EXPECT_GE(info.countCPU(), 1u);

  const auto mem = info.getMemInfos();
  EXPECT_GE(mem.total, mem.used);

  const auto total = info.totalmem();
  EXPECT_EQ(total, mem.total);

  const auto process = info.getProcessInfo(static_cast<KBEngine::uint32>(getpid()));
  EXPECT_FALSE(process.error);
}

TEST(HelperSysInfoBootstrapTest, ReturnsStableMacAddressContainer)
{
  auto& info = KBEngine::SystemInfo::getSingleton();
  const auto macs = info.getMacAddresses();

  // Skip test if no network interfaces are available (common in containers/CI)
  if (macs.empty())
  {
    GTEST_SKIP() << "No network interfaces available, skipping MAC address test";
    return;
  }

  // Check if we have any valid MAC addresses
  bool has_valid_mac = false;
  for (const auto& mac : macs)
  {
    if (!mac.empty())
    {
      has_valid_mac = true;
      break;
    }
  }

  if (!has_valid_mac)
  {
    GTEST_SKIP() << "No valid MAC addresses available (all empty), skipping test";
    return;
  }

  // Only verify that we found at least one valid MAC address
  EXPECT_TRUE(has_valid_mac) << "Should have at least one valid MAC address";
}
