#include <gtest/gtest.h>

#include "helper/memory_helper.h"

TEST(HelperMemoryBootstrapTest, LeakDetectionHookIsCallable)
{
  KBEngine::startLeakDetection(KBEngine::BASEAPP_TYPE, 123);
  SUCCEED();
}
