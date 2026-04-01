#include <gtest/gtest.h>

#include "network/event_dispatcher.h"
#include "server/components.h"

TEST(ComponentsBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(ComponentsBootstrapTest, StaticMemberAccess)
{
  EXPECT_EQ(KBEngine::Components::ANY_UID, -1);
}
