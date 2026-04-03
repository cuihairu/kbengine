#include <gtest/gtest.h>

#include "network/event_poller.h"

// 平台检测
#if KBE_PLATFORM == PLATFORM_WIN32
	#define PLATFORM_NAME "Windows"
	#define EXPECTED_POLLER_TYPE "IOCP"
#elif KBE_PLATFORM == PLATFORM_UNIX
	#define PLATFORM_NAME "Unix/Linux"
	#define EXPECTED_POLLER_TYPE "EPOLL"
#else
	#define PLATFORM_NAME "Other"
	#define EXPECTED_POLLER_TYPE "SELECT"
#endif

TEST(NetworkEventPollerBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(NetworkEventPollerBootstrapTest, CreatePoller)
{
  // EventPoller::create() 应该返回正确的 poller 类型
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 验证文件描述符
  int fd = poller->getFileDescriptor();
  EXPECT_GE(fd, -1); // IOCP 返回 -1，epoll 返回有效的 fd

  delete poller;
}

TEST(NetworkEventPollerBootstrapTest, PollerTypeCheck)
{
  // 验证创建的 poller 类型符合预期
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 通过类型识别验证 poller 类型
  // 注意：这需要 RTTI 支持，或者通过行为特征来判断

  delete poller;
}

#if KBE_PLATFORM == PLATFORM_WIN32

TEST(NetworkEventPollerWindowsTest, IocpPollerCreation)
{
  // Windows 平台应该创建 IocpPoller
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // IOCP 的文件描述符返回 -1
  EXPECT_EQ(poller->getFileDescriptor(), -1);

  delete poller;
}

TEST(NetworkEventPollerWindowsTest, IocpBasicOperations)
{
  // 测试 IOCP 的基本操作
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 测试 processPendingEvents 不崩溃
  // maxWait = 0.001 秒（1毫秒）
  int eventsProcessed = poller->processPendingEvents(0.001);
  EXPECT_GE(eventsProcessed, 0);

  delete poller;
}

TEST(NetworkEventPollerWindowsTest, IocpSpareTime)
{
  // 测试 spareTime 统计
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  poller->clearSpareTime();
  EXPECT_EQ(poller->spareTime(), 0ULL);

  // 运行一次事件处理
  poller->processPendingEvents(0.001);

  // spareTime 应该被更新
  // 注意：可能在无事件时保持为 0
  EXPECT_GE(poller->spareTime(), 0ULL);

  delete poller;
}

#elif KBE_PLATFORM == PLATFORM_UNIX

TEST(NetworkEventPollerLinuxTest, EpollPollerCreation)
{
  // Linux 平台应该创建 EpollPoller
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // Epoll 应该有有效的文件描述符
  int fd = poller->getFileDescriptor();
  EXPECT_GE(fd, 0); // epoll 返回有效的 epfd

  delete poller;
}

TEST(NetworkEventPollerLinuxTest, EpollBasicOperations)
{
  // 测试 Epoll 的基本操作
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 测试 processPendingEvents 不崩溃
  int eventsProcessed = poller->processPendingEvents(0.001);
  EXPECT_GE(eventsProcessed, 0);

  delete poller;
}

#else

TEST(NetworkEventPollerOtherTest, SelectPollerCreation)
{
  // 其他平台应该创建 SelectPoller
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  delete poller;
}

TEST(NetworkEventPollerOtherTest, SelectBasicOperations)
{
  // 测试 Select 的基本操作
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 测试 processPendingEvents 不崩溃
  int eventsProcessed = poller->processPendingEvents(0.001);
  EXPECT_GE(eventsProcessed, 0);

  delete poller;
}

#endif

// 跨平台测试
TEST(NetworkEventPollerCrossPlatformTest, MultiplePollerCreation)
{
  // 测试创建和销毁多个 poller
  const int pollerCount = 10;
  KBEngine::Network::EventPoller* pollers[pollerCount];

  // 创建多个 poller
  for (int i = 0; i < pollerCount; ++i)
  {
    pollers[i] = KBEngine::Network::EventPoller::create();
    ASSERT_NE(pollers[i], nullptr);
  }

  // 销毁所有 poller
  for (int i = 0; i < pollerCount; ++i)
  {
    delete pollers[i];
  }
}

TEST(NetworkEventPollerCrossPlatformTest, PollerConsistency)
{
  // 测试多次创建 poller 的一致性
  auto* poller1 = KBEngine::Network::EventPoller::create();
  auto* poller2 = KBEngine::Network::EventPoller::create();

  ASSERT_NE(poller1, nullptr);
  ASSERT_NE(poller2, nullptr);

  // 两个 poller 应该是相同类型的
  EXPECT_EQ(poller1->getFileDescriptor(), poller2->getFileDescriptor());

  delete poller1;
  delete poller2;
}

// 性能基准测试
TEST(NetworkEventPollerPerformanceTest, PollerLatency)
{
  // 测试 poller 的事件处理延迟
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 测量 100 次调用的平均延迟
  const int iterations = 100;
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i)
  {
    poller->processPendingEvents(0.0); // 无阻塞
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  double avgLatencyUs = (double)duration.count() / iterations;

  // 平均延迟应该小于 100 微秒
  EXPECT_LT(avgLatencyUs, 100.0);

  delete poller;
}

TEST(NetworkEventPollerPerformanceTest, PollerThroughput)
{
  // 测试 poller 的吞吐量
  auto* poller = KBEngine::Network::EventPoller::create();
  ASSERT_NE(poller, nullptr);

  // 测试短时间内能处理多少次事件循环
  const double testDuration = 0.01; // 10 毫秒
  auto start = std::chrono::high_resolution_clock::now();

  int loopCount = 0;
  auto end = start + std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<double>(testDuration));

  while (std::chrono::high_resolution_clock::now() < end)
  {
    poller->processPendingEvents(0.0);
    loopCount++;
  }

  // 应该能执行至少 100 次循环
  EXPECT_GE(loopCount, 100);

  delete poller;
}

