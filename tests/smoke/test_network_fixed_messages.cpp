#include <gtest/gtest.h>

#include "network/fixed_messages.h"

TEST(NetworkFixedMessagesBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(NetworkFixedMessagesBootstrapTest, FixedMessagesInstance)
{
  KBEngine::Network::FixedMessages* created = nullptr;
  if (KBEngine::Network::FixedMessages::getSingletonPtr() == nullptr)
  {
    created = new KBEngine::Network::FixedMessages();
  }

  KBEngine::Network::FixedMessages& fm = KBEngine::Network::FixedMessages::getSingleton();
  const bool loaded = fm.loadConfig("nonexistent.xml", false);
  if (created != nullptr)
  {
    EXPECT_FALSE(loaded);
  }
  else
  {
    EXPECT_TRUE(loaded);
  }

  delete created;
}
