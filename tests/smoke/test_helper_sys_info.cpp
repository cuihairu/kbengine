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

  for (const auto& mac : macs)
  {
    EXPECT_FALSE(mac.empty());
  }
}
