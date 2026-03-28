#include <gtest/gtest.h>

#include <string>

#include "helper/debug_option.h"

TEST(HelperDebugOptionBootstrapTest, ExposesExpectedDefaults)
{
  EXPECT_FALSE(KBEngine::Network::g_packetAlwaysContainLength);
  EXPECT_EQ(KBEngine::Network::g_trace_packet, 0);
  EXPECT_TRUE(KBEngine::Network::g_trace_encrypted_packet);
  EXPECT_FALSE(KBEngine::Network::g_trace_packet_use_logfile);
  EXPECT_TRUE(KBEngine::Network::g_trace_packet_disables.empty());
  EXPECT_FALSE(KBEngine::g_debugEntity);
  EXPECT_EQ(KBEngine::g_appPublish, 1);
}

TEST(HelperDebugOptionBootstrapTest, AllowsRuntimeMutation)
{
  KBEngine::Network::g_packetAlwaysContainLength = true;
  KBEngine::Network::g_trace_packet = 2;
  KBEngine::Network::g_trace_encrypted_packet = false;
  KBEngine::Network::g_trace_packet_use_logfile = true;
  KBEngine::Network::g_trace_packet_disables = {"Login", "Ping"};
  KBEngine::g_debugEntity = true;
  KBEngine::g_appPublish = 0;

  EXPECT_TRUE(KBEngine::Network::g_packetAlwaysContainLength);
  EXPECT_EQ(KBEngine::Network::g_trace_packet, 2);
  EXPECT_FALSE(KBEngine::Network::g_trace_encrypted_packet);
  EXPECT_TRUE(KBEngine::Network::g_trace_packet_use_logfile);
  ASSERT_EQ(KBEngine::Network::g_trace_packet_disables.size(), 2u);
  EXPECT_EQ(KBEngine::Network::g_trace_packet_disables[0], "Login");
  EXPECT_TRUE(KBEngine::g_debugEntity);
  EXPECT_EQ(KBEngine::g_appPublish, 0);

  KBEngine::Network::g_packetAlwaysContainLength = false;
  KBEngine::Network::g_trace_packet = 0;
  KBEngine::Network::g_trace_encrypted_packet = true;
  KBEngine::Network::g_trace_packet_use_logfile = false;
  KBEngine::Network::g_trace_packet_disables.clear();
  KBEngine::g_debugEntity = false;
  KBEngine::g_appPublish = 1;
}
