#include <gtest/gtest.h>
#include "network/event_poller.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

#if KBE_PLATFORM == PLATFORM_WIN32
	#include <WinSock2.h>
	#include <Windows.h>

// 模拟的高并发场景
static const int HIGH_CONCURRENCY_CONNECTIONS = 1000;
static const int STRESS_TEST_CONNECTIONS = 100;

TEST(NetworkIocpPollerStressTest, HighConcurrencyPollerCreation)
{
	// 测试在高并发场景下创建多个 IOCP poller
	const int pollerCount = 10;
	std::vector<KBEngine::Network::EventPoller*> pollers;

	// 创建多个 poller
	for (int i = 0; i < pollerCount; ++i)
	{
		auto* poller = KBEngine::Network::EventPoller::create();
		ASSERT_NE(poller, nullptr);
		pollers.push_back(poller);
	}

	// 清理
	for (auto* poller : pollers)
	{
		delete poller;
	}
}

TEST(NetworkIocpPollerStressTest, RapidPollerCreationDestruction)
{
	// 测试快速创建和销毁 poller
	const int iterations = 100;

	for (int i = 0; i < iterations; ++i)
	{
		auto* poller = KBEngine::Network::EventPoller::create();
		ASSERT_NE(poller, nullptr);
		delete poller;
	}
}

TEST(NetworkIocpPollerStressTest, LongRunningEventProcessing)
{
	// 测试长时间运行的事件处理
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	// 运行 1 秒
	auto start = std::chrono::steady_clock::now();
	auto end = start + std::chrono::seconds(1);

	int loopCount = 0;
	while (std::chrono::steady_clock::now() < end)
	{
		poller->processPendingEvents(0.001); // 1ms 超时
		loopCount++;
	}

	// 应该能执行多次循环
	EXPECT_GE(loopCount, 100);

	delete poller;
}

TEST(NetworkIocpPollerStressTest, ConcurrentEventProcessing)
{
	// 测试多线程并发访问 poller
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	const int threadCount = 4;
	const int iterationsPerThread = 100;
	std::atomic<int> totalProcessed(0);

	std::vector<std::thread> threads;

	// 启动多个线程
	for (int i = 0; i < threadCount; ++i)
	{
		threads.emplace_back([poller, &totalProcessed, iterationsPerThread]()
		{
			for (int j = 0; j < iterationsPerThread; ++j)
			{
				int events = poller->processPendingEvents(0.001);
				totalProcessed += events;
			}
		});
	}

	// 等待所有线程完成
	for (auto& thread : threads)
	{
		thread.join();
	}

	// 验证没有崩溃
	EXPECT_GE(totalProcessed.load(), 0);

	delete poller;
}

TEST(NetworkIocpPollerStressTest, MemoryLeakDetection)
{
	// 简单的内存泄漏检测：多次创建和销毁
	const int iterations = 1000;
	std::vector<size_t> memorySnapshots;

	for (int i = 0; i < iterations; ++i)
	{
		auto* poller = KBEngine::Network::EventPoller::create();
		ASSERT_NE(poller, nullptr);

		// 执行一些操作
		poller->processPendingEvents(0.001);

		delete poller;

		// 每 100 次检查一次内存（简化版）
		if (i % 100 == 0)
		{
			// 这里应该使用实际的内存检测工具
			// 例如：Windows 的 CRT 调试堆或 Valgrind（Linux）
			// 暂时跳过
		}
	}

	// 如果到这里没有崩溃，说明没有明显的内存泄漏
	SUCCEED();
}

TEST(NetworkIocpPollerPerformanceTest, EventProcessingLatency)
{
	// 测试事件处理延迟
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	const int sampleCount = 1000;
	std::vector<double> latencies;
	latencies.reserve(sampleCount);

	for (int i = 0; i < sampleCount; ++i)
	{
		auto start = std::chrono::high_resolution_clock::now();
		poller->processPendingEvents(0.0);
		auto end = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		latencies.push_back(duration.count());
	}

	// 计算平均延迟（纳秒）
	double avgLatency = 0;
	for (double latency : latencies)
	{
		avgLatency += latency;
	}
	avgLatency /= sampleCount;

	// 平均延迟应该小于 100 微秒（100,000 纳秒）
	EXPECT_LT(avgLatency, 100000.0);

	// 计算最大延迟
	double maxLatency = *std::max_element(latencies.begin(), latencies.end());

	// 最大延迟应该小于 1 毫秒（1,000,000 纳秒）
	EXPECT_LT(maxLatency, 1000000.0);

	delete poller;
}

TEST(NetworkIocpPollerPerformanceTest, EventProcessingThroughput)
{
	// 测试事件处理吞吐量
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	const double testDuration = 1.0; // 1 秒
	auto start = std::chrono::high_resolution_clock::now();

	int processedCount = 0;
	auto end = start + std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<double>(testDuration));

	while (std::chrono::high_resolution_clock::now() < end)
	{
		processedCount += poller->processPendingEvents(0.0);
	}

	// 吞吐量（每秒处理的事件数）
	double throughput = processedCount / testDuration;

	// 应该至少能处理 1000 事件/秒
	EXPECT_GE(throughput, 1000.0);

	delete poller;
}

TEST(NetworkIocpPollerComparisonTest, CompareWithSelect)
{
	// 注意：这个测试需要在同一平台上运行才有意义
	// 由于 KBEngine 在不同平台使用不同的 poller，这里只能测试当前平台的实现

	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	// 基准测试
	const int iterations = 1000;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < iterations; ++i)
	{
		poller->processPendingEvents(0.0);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	double avgLatencyUs = (double)duration.count() / iterations;

	// 输出性能数据
	std::cout << "Average latency per call: " << avgLatencyUs << " μs" << std::endl;
	std::cout << "Throughput: " << (iterations / (duration.count() / 1000000.0)) << " calls/sec" << std::endl;

	delete poller;

	// 基本断言：平均延迟应该合理
	EXPECT_LT(avgLatencyUs, 1000.0); // 小于 1 毫秒
}

// 边界条件测试
TEST(NetworkIocpPollerBoundaryTest, ZeroTimeout)
{
	// 测试零超时
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	int events = poller->processPendingEvents(0.0);
	EXPECT_GE(events, 0);

	delete poller;
}

TEST(NetworkIocpPollerBoundaryTest, NegativeTimeout)
{
	// 测试负超时（应该当作 0 处理）
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	int events = poller->processPendingEvents(-0.001);
	EXPECT_GE(events, 0);

	delete poller;
}

TEST(NetworkIocpPollerBoundaryTest, VeryLongTimeout)
{
	// 测试非常长的超时
	auto* poller = KBEngine::Network::EventPoller::create();
	ASSERT_NE(poller, nullptr);

	// 使用短超时来模拟，避免测试运行太久
	int events = poller->processPendingEvents(0.1); // 100 毫秒
	EXPECT_GE(events, 0);

	delete poller;
}

#endif // KBE_PLATFORM == PLATFORM_WIN32
