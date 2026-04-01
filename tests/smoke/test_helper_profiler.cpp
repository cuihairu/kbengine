#include <gtest/gtest.h>

#include "helper/profiler.h"

TEST(HelperProfilerBootstrapTest, CanConstructAndDestroyProfiler)
{
  KBEngine::Profiler profiler;
  SUCCEED();
}
