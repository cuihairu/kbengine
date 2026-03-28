#include <gtest/gtest.h>

#include "network/network_stats.h"
#include "network/message_handler.h"

namespace {

struct RecordingStatsHandler final : KBEngine::Network::NetworkStatsHandler {
  int send_calls = 0;
  int recv_calls = 0;
  int last_send_size = 0;
  int last_recv_size = 0;
  const KBEngine::Network::MessageHandler* last_send_handler = nullptr;
  const KBEngine::Network::MessageHandler* last_recv_handler = nullptr;

  void onSendMessage(const KBEngine::Network::MessageHandler& msgHandler, int size) override
  {
    ++send_calls;
    last_send_size = size;
    last_send_handler = &msgHandler;
  }

  void onRecvMessage(const KBEngine::Network::MessageHandler& msgHandler, int size) override
  {
    ++recv_calls;
    last_recv_size = size;
    last_recv_handler = &msgHandler;
  }
};

} // namespace

TEST(NetworkStatsBootstrapTest, TracksSendAndReceiveCountersByMessageName)
{
  auto& stats = KBEngine::Network::g_networkStats;
  stats.stats().clear();

  KBEngine::Network::MessageHandler handler;
  handler.name = "Loginapp::onHello";

  stats.trackMessage(KBEngine::Network::NetworkStats::SEND, handler, 12);
  stats.trackMessage(KBEngine::Network::NetworkStats::RECV, handler, 34);
  stats.trackMessage(KBEngine::Network::NetworkStats::SEND, handler, 8);

  const auto iter = stats.stats().find(handler.name);
  ASSERT_NE(iter, stats.stats().end());
  EXPECT_EQ(iter->second.name, handler.name);
  EXPECT_EQ(iter->second.send_count, 2u);
  EXPECT_EQ(iter->second.send_size, 20u);
  EXPECT_EQ(iter->second.recv_count, 1u);
  EXPECT_EQ(iter->second.recv_size, 34u);

  EXPECT_EQ(handler.send_count, 2u);
  EXPECT_EQ(handler.send_size, 20u);
  EXPECT_EQ(handler.recv_count, 1u);
  EXPECT_EQ(handler.recv_size, 34u);
}

TEST(NetworkStatsBootstrapTest, NotifiesRegisteredHandlersAndStopsAfterRemoval)
{
  auto& stats = KBEngine::Network::g_networkStats;
  stats.stats().clear();

  KBEngine::Network::MessageHandler handler;
  handler.name = "Baseapp::onPing";

  RecordingStatsHandler recorder;
  stats.addHandler(&recorder);
  stats.trackMessage(KBEngine::Network::NetworkStats::SEND, handler, 55);
  stats.trackMessage(KBEngine::Network::NetworkStats::RECV, handler, 66);
  stats.removeHandler(&recorder);
  stats.trackMessage(KBEngine::Network::NetworkStats::SEND, handler, 77);

  EXPECT_EQ(recorder.send_calls, 1);
  EXPECT_EQ(recorder.recv_calls, 1);
  EXPECT_EQ(recorder.last_send_size, 55);
  EXPECT_EQ(recorder.last_recv_size, 66);
  EXPECT_EQ(recorder.last_send_handler, &handler);
  EXPECT_EQ(recorder.last_recv_handler, &handler);
}
