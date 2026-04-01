#include <gtest/gtest.h>

#include "thread/concurrency.h"

namespace
{
int g_start_calls = 0;
int g_end_calls = 0;

void on_start()
{
  ++g_start_calls;
}

void on_end()
{
  ++g_end_calls;
}
}

TEST(ThreadConcurrencyBootstrapTest, DefaultCallbacksAreSafeNoOps)
{
  KBEngine::KBEConcurrency::setMainThreadIdleCallbacks(nullptr, nullptr);

  KBEngine::KBEConcurrency::onStartMainThreadIdling();
  KBEngine::KBEConcurrency::onEndMainThreadIdling();

  SUCCEED();
}

TEST(ThreadConcurrencyBootstrapTest, InvokesRegisteredCallbacks)
{
  g_start_calls = 0;
  g_end_calls = 0;

  KBEngine::KBEConcurrency::setMainThreadIdleCallbacks(&on_start, &on_end);

  KBEngine::KBEConcurrency::onStartMainThreadIdling();
  KBEngine::KBEConcurrency::onEndMainThreadIdling();
  KBEngine::KBEConcurrency::onStartMainThreadIdling();

  EXPECT_EQ(g_start_calls, 2);
  EXPECT_EQ(g_end_calls, 1);

  KBEngine::KBEConcurrency::setMainThreadIdleCallbacks(nullptr, nullptr);
}
