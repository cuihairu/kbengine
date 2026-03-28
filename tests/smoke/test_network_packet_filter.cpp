#include <gtest/gtest.h>

#include "network/packet_filter_cmake.h"

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
};

}  // namespace

TEST(NetworkPacketFilterBootstrapTest, SendForwardsToPacketSender)
{
  KBEngine::Network::PacketFilter filter;
  KBEngine::Network::Channel channel;
  KBEngine::Network::Packet packet;
  RecordingSender sender;
  sender.result = KBEngine::Network::REASON_CHANNEL_LOST;

  const auto reason = filter.send(&channel, sender, &packet, 42);

  EXPECT_EQ(reason, KBEngine::Network::REASON_CHANNEL_LOST);
  EXPECT_EQ(sender.last_channel, &channel);
  EXPECT_EQ(sender.last_packet, &packet);
  EXPECT_EQ(sender.last_userarg, 42);
}

TEST(NetworkPacketFilterBootstrapTest, RecvForwardsToPacketReceiver)
{
  KBEngine::Network::PacketFilter filter;
  KBEngine::Network::Channel channel;
  KBEngine::Network::Packet packet;
  RecordingReceiver receiver;
  receiver.result = KBEngine::Network::REASON_CORRUPTED_PACKET;

  const auto reason = filter.recv(&channel, receiver, &packet);

  EXPECT_EQ(reason, KBEngine::Network::REASON_CORRUPTED_PACKET);
  EXPECT_EQ(receiver.last_channel, &channel);
  EXPECT_EQ(receiver.last_packet, &packet);
}
