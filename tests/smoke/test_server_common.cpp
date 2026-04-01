#include <gtest/gtest.h>

#include "server/common.h"

TEST(ServerCommonBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(ServerCommonBootstrapTest, SecondsToTicks)
{
  auto* fn = &KBEngine::secondsToTicks;
  ASSERT_NE(fn, nullptr);
}

TEST(ServerCommonBootstrapTest, Datatype2Id)
{
  EXPECT_EQ(KBEngine::datatype2id(std::string("STRING")), 1);
  EXPECT_EQ(KBEngine::datatype2id(std::string("UINT8")), 2);
  EXPECT_EQ(KBEngine::datatype2id(std::string("FLOAT")), 13);
  EXPECT_EQ(KBEngine::datatype2id(std::string("UNKNOWN_TYPE")), 0);
}

TEST(ServerCommonBootstrapTest, Datatype2NativeType)
{
  EXPECT_EQ(KBEngine::datatype2nativetype(std::string("STRING")), "STRING");
  EXPECT_EQ(KBEngine::datatype2nativetype((KBEngine::uint16)13), "FLOAT");
  EXPECT_EQ(KBEngine::datatype2nativetype((KBEngine::uint16)99), "");
}
