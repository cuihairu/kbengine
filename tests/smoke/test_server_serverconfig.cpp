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
