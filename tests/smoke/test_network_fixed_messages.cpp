#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "network/fixed_messages.h"
#include "resmgr/resmgr.h"

namespace {

std::filesystem::path make_temp_fixed_messages_path()
{
  return std::filesystem::temp_directory_path() / "kbengine_fixed_messages_test.xml";
}

}  // namespace

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

TEST(NetworkFixedMessagesBootstrapTest, UsesXmlWrapperCompatibleStructure)
{
  const auto xml_path = make_temp_fixed_messages_path();
  std::ofstream out(xml_path);
  out << R"(<root>
  <Login>
    <id>101</id>
  </Login>
  <Ping>
    <id>202</id>
  </Ping>
</root>)";
  out.close();

  KBEngine::XML xml(xml_path.string().c_str());
  ASSERT_TRUE(xml.isGood());

  tinyxml2::XMLNode* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);
  EXPECT_EQ(xml.getKey(root), "Login");

  tinyxml2::XMLNode* login_id = xml.enterNode(root->FirstChild(), "id");
  ASSERT_NE(login_id, nullptr);
  EXPECT_EQ(xml.getValInt(login_id), 101);

  tinyxml2::XMLNode* ping = root->NextSibling();
  ASSERT_NE(ping, nullptr);
  EXPECT_EQ(xml.getKey(ping), "Ping");
  tinyxml2::XMLNode* ping_id = xml.enterNode(ping->FirstChild(), "id");
  ASSERT_NE(ping_id, nullptr);
  EXPECT_EQ(xml.getValInt(ping_id), 202);

  std::filesystem::remove(xml_path);
}
