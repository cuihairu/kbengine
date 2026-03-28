#include <gtest/gtest.h>

#include "helper/debug_helper.h"

TEST(HelperDebugHelperBootstrapTest, InitializeAndFinalizeAreSafe)
{
  KBEngine::DebugHelper::initialize(KBEngine::UNKNOWN_COMPONENT_TYPE);
  ASSERT_TRUE(KBEngine::DebugHelper::isInit());

  KBEngine::DebugHelper::getSingleton().print_msg("debug-helper-print\n");
  KBEngine::DebugHelper::getSingleton().info_msg("debug-helper-info\n");
  KBEngine::DebugHelper::getSingleton().error_msg("debug-helper-error\n");

  KBEngine::DebugHelper::finalise(true);
  EXPECT_FALSE(KBEngine::DebugHelper::isInit());
}

TEST(HelperDebugHelperBootstrapTest, LogTypeMappingIsIdentityInBootstrapMode)
{
  EXPECT_EQ(KBEngine::KBELOG_TYPE_MAPPING(KBELOG_WARNING), KBELOG_WARNING);
}
