#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "helper/debug_option.h"
// serverconfig.inl requires full Network::Address definition
#include "network/address.h"
#include "network/common.h"
#include "resmgr/resmgr.h"
#include "server/serverconfig.h"
#include "test_resmgr_environment.h"

namespace {

std::filesystem::path make_temp_serverconfig_path()
{
  return std::filesystem::temp_directory_path() / "kbengine_serverconfig_test.xml";
}

}  // namespace

TEST(ServerServerConfigBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(ServerServerConfigBootstrapTest, SingletonAccess)
{
  new KBEngine::ServerConfig();
  KBEngine::ServerConfig& config = KBEngine::ServerConfig::getSingleton();
  (void)config;
  delete &config;
}

TEST(ServerServerConfigBootstrapTest, LoadsCoreXmlSettings)
{
  const auto xml_path = make_temp_serverconfig_path();
  std::ofstream out(xml_path);
  out << R"(<root>
  <packetAlwaysContainLength>1</packetAlwaysContainLength>
  <trace_packet>
    <debug_type>2</debug_type>
    <use_logfile>true</use_logfile>
    <disables>
      <item>Encrypted::packets</item>
      <item>Login</item>
    </disables>
  </trace_packet>
  <shutdown_time>3.25</shutdown_time>
  <shutdown_waittick>9.5</shutdown_waittick>
  <callback_timeout>3</callback_timeout>
  <thread_pool>
    <timeout>2.5</timeout>
    <init_create>4</init_create>
    <pre_create>6</pre_create>
    <max_create>10</max_create>
  </thread_pool>
  <channelCommon>
    <timeout>
      <internal>7.5</internal>
      <external>8.5</external>
    </timeout>
    <resend>
      <internal>
        <interval>11</interval>
        <retries>12</retries>
      </internal>
      <external>
        <interval>13</interval>
        <retries>14</retries>
      </external>
    </resend>
    <readBufferSize>
      <internal>1024</internal>
      <external>2048</external>
    </readBufferSize>
    <writeBufferSize>
      <internal>4096</internal>
      <external>8192</external>
    </writeBufferSize>
  </channelCommon>
  <gameUpdateHertz>30</gameUpdateHertz>
  <interfaces>
    <host>127.0.0.1</host>
    <port_min>20013</port_min>
    <port_max>20015</port_max>
    <SOMAXCONN>64</SOMAXCONN>
  </interfaces>
</root>)";
  out.close();

  delete KBEngine::ServerConfig::getSingletonPtr();
  if (KBEngine::Resmgr::getSingletonPtr() == nullptr)
  {
    new KBEngine::Resmgr();
  }
  KBEngine::Resmgr::getSingleton().initialize();

  KBEngine::Network::g_packetAlwaysContainLength = false;
  KBEngine::Network::g_trace_packet = 0;
  KBEngine::Network::g_trace_packet_use_logfile = false;
  KBEngine::Network::g_trace_encrypted_packet = true;
  KBEngine::Network::g_trace_packet_disables.clear();
  KBEngine::Network::g_channelInternalTimeout = 60.f;
  KBEngine::Network::g_channelExternalTimeout = 60.f;
  KBEngine::Network::g_intReSendInterval = 10;
  KBEngine::Network::g_extReSendInterval = 10;

  auto* config = new KBEngine::ServerConfig();
  ASSERT_TRUE(config->loadConfig(xml_path.string()));

  EXPECT_TRUE(KBEngine::Network::g_packetAlwaysContainLength);
  EXPECT_EQ(KBEngine::Network::g_trace_packet, 2);
  EXPECT_TRUE(KBEngine::Network::g_trace_packet_use_logfile);
  ASSERT_GE(KBEngine::Network::g_trace_packet_disables.size(), 2u);
  EXPECT_EQ(KBEngine::Network::g_trace_packet_disables[0], "Encrypted::packets");
  EXPECT_FALSE(KBEngine::Network::g_trace_encrypted_packet);

  EXPECT_EQ(config->gameUpdateHertz(), 30);
  EXPECT_FLOAT_EQ(config->shutdowntime(), 3.25f);
  EXPECT_FLOAT_EQ(config->shutdownWaitTickTime(), 9.5f);
  EXPECT_EQ(config->interfacesAddress(), "127.0.0.1");
  EXPECT_EQ(config->interfacesPortMin(), 20013);
  EXPECT_EQ(config->interfacesPortMax(), 20015);
  EXPECT_EQ(config->getInterfaces().tcp_SOMAXCONN, 64u);

  const KBEngine::ChannelCommon& channel_common = config->channelCommon();
  EXPECT_FLOAT_EQ(channel_common.channelInternalTimeout, 7.5f);
  EXPECT_FLOAT_EQ(channel_common.channelExternalTimeout, 8.5f);
  EXPECT_EQ(channel_common.intReadBufferSize, 1024u);
  EXPECT_EQ(channel_common.extReadBufferSize, 2048u);
  EXPECT_EQ(channel_common.intWriteBufferSize, 4096u);
  EXPECT_EQ(channel_common.extWriteBufferSize, 8192u);
  EXPECT_EQ(KBEngine::Network::g_intReSendInterval, 11u);
  EXPECT_EQ(KBEngine::Network::g_extReSendInterval, 13u);
  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelInternalTimeout, 7.5f);
  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelExternalTimeout, 8.5f);

  delete config;
  std::filesystem::remove(xml_path);
}

TEST(ServerServerConfigBootstrapTest, LoadsDbMgrInterfacesMachineAddressesAndEmailService)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_serverconfig_resmgr_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.system_res_dir() / "server" / "kbengine_defaults.xml",
      R"(<root>
  <interfaces>
    <host>127.0.0.1</host>
    <port_min>30099</port_min>
    <port_max>30099</port_max>
    <SOMAXCONN>64</SOMAXCONN>
  </interfaces>
  <dbmgr>
    <InterfacesServiceAddr>
      <enable>true</enable>
      <item>
        <host>10.0.0.10</host>
        <port>31001</port>
      </item>
      <item>
        <host>10.0.0.11</host>
        <port>0</port>
      </item>
    </InterfacesServiceAddr>
    <databaseInterfaces>
      <default>
        <pure>true</pure>
        <type>mysql</type>
        <host>127.0.0.1</host>
        <port>3306</port>
        <auth>
          <username>root</username>
          <password>rootpwd</password>
          <encrypt>false</encrypt>
        </auth>
        <databaseName>kbengine</databaseName>
        <numConnections>8</numConnections>
        <unicodeString>
          <characterSet>utf8mb4</characterSet>
          <collation>utf8mb4_bin</collation>
        </unicodeString>
      </default>
      <analytics>
        <pure>true</pure>
        <type>mysql</type>
        <host>192.168.1.20</host>
        <port>3307</port>
        <auth>
          <username>report</username>
          <password>reportpwd</password>
          <encrypt>true</encrypt>
        </auth>
        <databaseName>analytics</databaseName>
        <numConnections>3</numConnections>
        <unicodeString>
          <characterSet>latin1</characterSet>
          <collation>latin1_bin</collation>
        </unicodeString>
      </analytics>
    </databaseInterfaces>
  </dbmgr>
  <machine>
    <addresses>
      <internal>192.168.0.2</internal>
      <external>8.8.8.8</external>
    </addresses>
  </machine>
</root>)");

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "kbengine.xml",
      R"(<root>
  <email_service_config>server/email_service.xml</email_service_config>
</root>)");

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "email_service.xml",
      R"(<root>
  <smtp_server>smtp.example.com</smtp_server>
  <smtp_port>465</smtp_port>
  <username>robot@example.com</username>
  <password>plain-secret</password>
  <smtp_auth>1</smtp_auth>
  <email_activation>
    <subject>Activate</subject>
    <message> activation body </message>
    <deadline>3600</deadline>
    <backlink_success_message>ok</backlink_success_message>
    <backlink_fail_message>fail</backlink_fail_message>
    <backlink_hello_message>hello</backlink_hello_message>
  </email_activation>
  <email_resetpassword>
    <subject>Reset</subject>
    <message> reset body </message>
    <deadline>7200</deadline>
    <backlink_success_message>reset ok</backlink_success_message>
    <backlink_fail_message>reset fail</backlink_fail_message>
    <backlink_hello_message>reset hello</backlink_hello_message>
  </email_resetpassword>
  <email_bind>
    <subject>Bind</subject>
    <message> bind body </message>
    <deadline>1800</deadline>
    <backlink_success_message>bind ok</backlink_success_message>
    <backlink_fail_message>bind fail</backlink_fail_message>
    <backlink_hello_message>bind hello</backlink_hello_message>
  </email_bind>
</root>)");

  delete KBEngine::ServerConfig::getSingletonPtr();
  auto* config = new KBEngine::ServerConfig();

  ASSERT_TRUE(config->loadConfig("server/kbengine_defaults.xml"));
  ASSERT_TRUE(config->loadConfig("server/kbengine.xml"));

  const std::vector<KBEngine::Network::Address> addrs = config->interfacesAddrs();
  ASSERT_EQ(addrs.size(), 2u);
  EXPECT_STREQ(addrs[0].ipAsString(), "10.0.0.10");
  EXPECT_EQ(addrs[0].port, 31001);
  EXPECT_STREQ(addrs[1].ipAsString(), "10.0.0.11");
  EXPECT_EQ(addrs[1].port, KBE_INTERFACES_TCP_PORT);

  const KBEngine::DBInterfaceInfo* default_db = config->dbInterface("default");
  ASSERT_NE(default_db, nullptr);
  EXPECT_FALSE(default_db->isPure);
  EXPECT_STREQ(default_db->db_type, "mysql");
  EXPECT_STREQ(default_db->db_ip, "127.0.0.1");
  EXPECT_EQ(default_db->db_port, 3306u);
  EXPECT_STREQ(default_db->db_username, "root");
  EXPECT_STREQ(default_db->db_password, "rootpwd");
  EXPECT_FALSE(default_db->db_passwordEncrypt);
  EXPECT_STREQ(default_db->db_name, "kbengine");
  EXPECT_EQ(default_db->db_numConnections, 8u);
  EXPECT_EQ(default_db->db_unicodeString_characterSet, "utf8mb4");
  EXPECT_EQ(default_db->db_unicodeString_collation, "utf8mb4_bin");

  const KBEngine::DBInterfaceInfo* analytics_db = config->dbInterface("analytics");
  ASSERT_NE(analytics_db, nullptr);
  EXPECT_TRUE(analytics_db->isPure);
  EXPECT_STREQ(analytics_db->db_ip, "192.168.1.20");
  EXPECT_EQ(analytics_db->db_port, 3307u);
  EXPECT_TRUE(analytics_db->db_passwordEncrypt);
  EXPECT_STREQ(analytics_db->db_name, "analytics");
  EXPECT_EQ(analytics_db->db_numConnections, 3u);

  const KBEngine::ENGINE_COMPONENT_INFO& machine = config->getKBMachine();
  ASSERT_EQ(machine.machine_addresses.size(), 2u);
  EXPECT_EQ(machine.machine_addresses[0], "192.168.0.2");
  EXPECT_EQ(machine.machine_addresses[1], "8.8.8.8");

  EXPECT_EQ(config->emailServerInfo_.smtp_server, "smtp.example.com");
  EXPECT_EQ(config->emailServerInfo_.smtp_port, 465u);
  EXPECT_EQ(config->emailServerInfo_.username, "robot@example.com");
  EXPECT_EQ(config->emailServerInfo_.password, "plain-secret");
  EXPECT_EQ(config->emailServerInfo_.smtp_auth, 1u);

  EXPECT_EQ(config->emailAtivationInfo_.subject, "Activate");
  EXPECT_EQ(config->emailAtivationInfo_.message, " activation body ");
  EXPECT_EQ(config->emailAtivationInfo_.deadline, 3600u);
  EXPECT_EQ(config->emailAtivationInfo_.backlink_success_message, "ok");
  EXPECT_EQ(config->emailAtivationInfo_.backlink_fail_message, "fail");
  EXPECT_EQ(config->emailAtivationInfo_.backlink_hello_message, "hello");

  EXPECT_EQ(config->emailResetPasswordInfo_.subject, "Reset");
  EXPECT_EQ(config->emailResetPasswordInfo_.message, " reset body ");
  EXPECT_EQ(config->emailResetPasswordInfo_.deadline, 7200u);
  EXPECT_EQ(config->emailResetPasswordInfo_.backlink_success_message, "reset ok");
  EXPECT_EQ(config->emailResetPasswordInfo_.backlink_fail_message, "reset fail");
  EXPECT_EQ(config->emailResetPasswordInfo_.backlink_hello_message, "reset hello");

  EXPECT_EQ(config->emailBindInfo_.subject, "Bind");
  EXPECT_EQ(config->emailBindInfo_.message, " bind body ");
  EXPECT_EQ(config->emailBindInfo_.deadline, 1800u);
  EXPECT_EQ(config->emailBindInfo_.backlink_success_message, "bind ok");
  EXPECT_EQ(config->emailBindInfo_.backlink_fail_message, "bind fail");
  EXPECT_EQ(config->emailBindInfo_.backlink_hello_message, "bind hello");

  delete config;
}
