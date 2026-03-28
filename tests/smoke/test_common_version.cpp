#include <gtest/gtest.h>

#include "common/kbeversion.h"

TEST(CommonVersionBootstrapTest, ExposesEngineVersionString)
{
  EXPECT_EQ(KBEngine::KBEVersion::versionString(), "2.5.10");
}

TEST(CommonVersionBootstrapTest, SupportsScriptVersionOverride)
{
  KBEngine::KBEVersion::setScriptVersion("3.12.13");
  EXPECT_EQ(KBEngine::KBEVersion::scriptVersionString(), "3.12.13");
}
