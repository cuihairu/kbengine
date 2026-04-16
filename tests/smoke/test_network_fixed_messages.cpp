#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "network/fixed_messages.h"
#include "resmgr/resmgr.h"
#include "test_resmgr_environment.h"
#include "xml/xml.h"

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
    delete KBEngine::Resmgr::getSingletonPtr();
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

TEST(NetworkFixedMessagesBootstrapTest, LoadsDirectTinyXml2ChildTraversal)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_fixed_messages_resmgr_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "fixed_messages.xml",
      R"(<root>
  <Login>
    <id>101</id>
    <note>ignored</note>
  </Login>
  <Ping>
    <id>202</id>
  </Ping>
  <Kick>
    <id>303</id>
  </Kick>
</root>)");

  auto* fixed_messages = KBEngine::Network::FixedMessages::getSingletonPtr();
  if (fixed_messages != nullptr)
  {
    delete fixed_messages;
  }

  delete KBEngine::Resmgr::getSingletonPtr();
  fixed_messages = new KBEngine::Network::FixedMessages();
  ASSERT_TRUE(fixed_messages->loadConfig("server/fixed_messages.xml", true));

  KBEngine::Network::FixedMessages::MSGInfo* login = fixed_messages->isFixed("Login");
  ASSERT_NE(login, nullptr);
  EXPECT_EQ(login->msgid, 101);
  EXPECT_EQ(login->msgname, "Login");

  KBEngine::Network::FixedMessages::MSGInfo* ping = fixed_messages->isFixed("Ping");
  ASSERT_NE(ping, nullptr);
  EXPECT_EQ(ping->msgid, 202);

  EXPECT_TRUE(fixed_messages->isFixed(static_cast<KBEngine::Network::MessageID>(303)));
  EXPECT_FALSE(fixed_messages->isFixed(static_cast<KBEngine::Network::MessageID>(404)));
  EXPECT_EQ(fixed_messages->isFixed("Missing"), nullptr);

  delete fixed_messages;
}

TEST(NetworkFixedMessagesBootstrapTest, LoadsDefaultsAndOverrideFiles)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_fixed_messages_override_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.system_res_dir() / "server" / "messages_fixed_defaults.xml",
      R"(<root>
  <Login>
    <id>11</id>
  </Login>
  <Ping>
    <id>22</id>
  </Ping>
</root>)");

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "messages_fixed.xml",
      R"(<root>
  <Kick>
    <id>33</id>
  </Kick>
</root>)");

  auto* fixed_messages = KBEngine::Network::FixedMessages::getSingletonPtr();
  if (fixed_messages != nullptr)
  {
    delete fixed_messages;
  }

  delete KBEngine::Resmgr::getSingletonPtr();
  fixed_messages = new KBEngine::Network::FixedMessages();

  ASSERT_TRUE(fixed_messages->loadConfig("server/messages_fixed_defaults.xml", true));
  ASSERT_TRUE(fixed_messages->loadConfig("server/messages_fixed.xml", false));

  auto* login = fixed_messages->isFixed("Login");
  ASSERT_NE(login, nullptr);
  EXPECT_EQ(login->msgid, 11);

  auto* ping = fixed_messages->isFixed("Ping");
  ASSERT_NE(ping, nullptr);
  EXPECT_EQ(ping->msgid, 22);

  auto* kick = fixed_messages->isFixed("Kick");
  ASSERT_NE(kick, nullptr);
  EXPECT_EQ(kick->msgid, 33);

  delete fixed_messages;
}
