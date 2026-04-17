#include <gtest/gtest.h>

#include "client_lib/config.h"
#include "common/kbeversion.h"
#include "entitydef/entitydef.h"
#include "helper/debug_option.h"
#include "network/common.h"
#include "test_resmgr_environment.h"

TEST(ClientLibConfigTest, LoadsDirectTinyXml2NodesAndWritesAccountName)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_client_config_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "client_config.xml",
      R"(<root>
  <packetAlwaysContainLength>1</packetAlwaysContainLength>
  <trace_packet>
    <debug_type>2</debug_type>
    <use_logfile>true</use_logfile>
    <disables>
      <item>Encrypted::packets</item>
      <item>Login</item>
    </disables>
  </trace_packet>
  <debugEntity>1</debugEntity>
  <publish>
    <state>0</state>
    <script_version>9.8.7</script_version>
  </publish>
  <channelCommon>
    <timeout>
      <internal>6.5</internal>
      <external>7.5</external>
    </timeout>
    <resend>
      <internal>
        <interval>15</interval>
        <retries>16</retries>
      </internal>
      <external>
        <interval>17</interval>
        <retries>18</retries>
      </external>
    </resend>
  </channelCommon>
  <telnet_service>
    <port>40100</port>
    <password>secret</password>
    <default_layer>python</default_layer>
  </telnet_service>
  <gameUpdateHertz>20</gameUpdateHertz>
  <ip>127.0.0.9</ip>
  <port>20001</port>
  <entryScriptFile>scripts/main.py</entryScriptFile>
  <accountName>old-account</accountName>
  <useLastAccountName>true</useLastAccountName>
  <encrypt_login>1</encrypt_login>
  <aliasEntityID>true</aliasEntityID>
  <entitydefAliasID>true</entitydefAliasID>
  <isOnInitCallPropertysSetMethods>false</isOnInitCallPropertysSetMethods>
</root>)");

  KBEngine::Network::g_packetAlwaysContainLength = false;
  KBEngine::Network::g_trace_packet = 0;
  KBEngine::Network::g_trace_packet_use_logfile = false;
  KBEngine::Network::g_trace_encrypted_packet = true;
  KBEngine::Network::g_trace_packet_disables.clear();
  KBEngine::Network::g_channelInternalTimeout = 60.f;
  KBEngine::Network::g_channelExternalTimeout = 60.f;
  KBEngine::Network::g_intReSendInterval = 10;
  KBEngine::Network::g_intReSendRetries = 0;
  KBEngine::Network::g_extReSendInterval = 10;
  KBEngine::Network::g_extReSendRetries = 0;
  KBEngine::g_debugEntity = false;
  KBEngine::g_appPublish = 1;
  KBEngine::KBEVersion::setScriptVersion("0.0.0");
  KBEngine::EntityDef::entityAliasID(false);
  KBEngine::EntityDef::entitydefAliasID(false);

  delete KBEngine::Config::getSingletonPtr();
  auto* config = new KBEngine::Config();
  ASSERT_TRUE(config->loadConfig("server/client_config.xml"));

  EXPECT_TRUE(KBEngine::Network::g_packetAlwaysContainLength);
  EXPECT_EQ(KBEngine::Network::g_trace_packet, 2);
  EXPECT_TRUE(KBEngine::Network::g_trace_packet_use_logfile);
  EXPECT_FALSE(KBEngine::Network::g_trace_encrypted_packet);
  ASSERT_EQ(KBEngine::Network::g_trace_packet_disables.size(), 2u);
  EXPECT_EQ(KBEngine::Network::g_trace_packet_disables[0], "Encrypted::packets");
  EXPECT_EQ(KBEngine::Network::g_trace_packet_disables[1], "Login");

  EXPECT_TRUE(KBEngine::g_debugEntity);
  EXPECT_EQ(KBEngine::g_appPublish, 0);
  EXPECT_EQ(KBEngine::KBEVersion::scriptVersionString(), "9.8.7");
  EXPECT_TRUE(KBEngine::EntityDef::entityAliasID());
  EXPECT_TRUE(KBEngine::EntityDef::entitydefAliasID());

  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelInternalTimeout, 6.5f);
  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelExternalTimeout, 7.5f);
  EXPECT_EQ(KBEngine::Network::g_intReSendInterval, 15u);
  EXPECT_EQ(KBEngine::Network::g_intReSendRetries, 16u);
  EXPECT_EQ(KBEngine::Network::g_extReSendInterval, 17u);
  EXPECT_EQ(KBEngine::Network::g_extReSendRetries, 18u);

  EXPECT_EQ(config->gameUpdateHertz(), 20);
  EXPECT_STREQ(config->ip(), "127.0.0.9");
  EXPECT_EQ(config->port(), 20001u);
  EXPECT_STREQ(config->entryScriptFile(), "scripts/main.py");
  EXPECT_STREQ(config->accountName(), "old-account");
  EXPECT_TRUE(config->useLastAccountName());
  EXPECT_EQ(config->encryptLogin(), 1);
  EXPECT_EQ(config->telnet_port, 40100u);
  EXPECT_EQ(config->telnet_passwd, "secret");
  EXPECT_EQ(config->telnet_deflayer, "python");
  EXPECT_FALSE(config->isOnInitCallPropertysSetMethods());

  config->writeAccountName("new-account");

  delete config;
  config = new KBEngine::Config();
  ASSERT_TRUE(config->loadConfig("server/client_config.xml"));
  EXPECT_STREQ(config->accountName(), "new-account");

  delete config;
}
