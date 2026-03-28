#include <gtest/gtest.h>

#include "network/packet_filter.h"
#include "network/packet_receiver.h"
#include "network/packet_sender.h"

namespace {

class RecordingSender final : public KBEngine::Network::PacketSender
{
public:
  KBEngine::Network::Reason result = KBEngine::Network::REASON_SUCCESS;
  KBEngine::Network::Channel* last_channel = nullptr;
  KBEngine::Network::Packet* last_packet = nullptr;
  int last_userarg = 0;

  KBEngine::Network::Reason processFilterPacket(
    KBEngine::Network::Channel* pChannel,
    KBEngine::Network::Packet* pPacket,
    int userarg) override
  {
    last_channel = pChannel;
    last_packet = pPacket;
    last_userarg = userarg;
    return result;
  }

  bool processSend(KBEngine::Network::Channel*, int) override
  {
    return true;
  }
};

class RecordingReceiver final : public KBEngine::Network::PacketReceiver
{
public:
  KBEngine::Network::Reason result = KBEngine::Network::REASON_SUCCESS;
  KBEngine::Network::Channel* last_channel = nullptr;
  KBEngine::Network::Packet* last_packet = nullptr;

  KBEngine::Network::Reason processFilteredPacket(
    KBEngine::Network::Channel* pChannel,
    KBEngine::Network::Packet* pPacket) override
  {
    last_channel = pChannel;
    last_packet = pPacket;
    return result;
  }

  bool processRecv(bool) override
  {
    return true;
  }

  RecvState checkSocketErrors(int, bool) override
  {
    return RECV_STATE_BREAK;
  }
};

}  // namespace

TEST(NetworkPacketFilterBootstrapTest, SendForwardsToPacketSender)
{
  KBEngine::Network::PacketFilter filter;
  RecordingSender sender;
  sender.result = KBEngine::Network::REASON_CHANNEL_LOST;
  auto* channel = static_cast<KBEngine::Network::Channel*>(nullptr);
  auto* packet = static_cast<KBEngine::Network::Packet*>(nullptr);

  const auto reason = filter.send(channel, sender, packet, 42);

  EXPECT_EQ(reason, KBEngine::Network::REASON_CHANNEL_LOST);
  EXPECT_EQ(sender.last_channel, channel);
  EXPECT_EQ(sender.last_packet, packet);
  EXPECT_EQ(sender.last_userarg, 42);
}

TEST(NetworkPacketFilterBootstrapTest, RecvForwardsToPacketReceiver)
{
  KBEngine::Network::PacketFilter filter;
  RecordingReceiver receiver;
  receiver.result = KBEngine::Network::REASON_CORRUPTED_PACKET;
  auto* channel = static_cast<KBEngine::Network::Channel*>(nullptr);
  auto* packet = static_cast<KBEngine::Network::Packet*>(nullptr);

  const auto reason = filter.recv(channel, receiver, packet);

  EXPECT_EQ(reason, KBEngine::Network::REASON_CORRUPTED_PACKET);
  EXPECT_EQ(receiver.last_channel, channel);
  EXPECT_EQ(receiver.last_packet, packet);
}
