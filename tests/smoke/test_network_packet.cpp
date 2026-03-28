#include <gtest/gtest.h>

#include "network/packet.h"

namespace {

class DummyPacket final : public KBEngine::Network::Packet
{
public:
  DummyPacket()
    : KBEngine::Network::Packet(7, false, 64)
  {
  }

  int recvFromEndPoint(KBEngine::Network::EndPoint&, KBEngine::Network::Address*) override
  {
    return 0;
  }
};

}  // namespace

TEST(NetworkPacketBootstrapTest, StartsWithExpectedDefaults)
{
  DummyPacket packet;

  EXPECT_EQ(packet.messageID(), 7);
  EXPECT_FALSE(packet.isTCPPacket());
  EXPECT_FALSE(packet.encrypted());
  EXPECT_TRUE(packet.empty());
  EXPECT_EQ(packet.sentSize, 0u);
  EXPECT_EQ(packet.pBundle(), nullptr);
}

TEST(NetworkPacketBootstrapTest, ResetPacketClearsMutableState)
{
  DummyPacket packet;
  packet << static_cast<uint8_t>(11);
  packet.encrypted(true);
  packet.sentSize = 32;
  packet.messageID(99);

  ASSERT_FALSE(packet.empty());

  packet.resetPacket();

  EXPECT_EQ(packet.messageID(), 0);
  EXPECT_FALSE(packet.encrypted());
  EXPECT_TRUE(packet.empty());
  EXPECT_EQ(packet.sentSize, 0u);
  EXPECT_EQ(packet.rpos(), 0u);
  EXPECT_EQ(packet.wpos(), 0u);
}

TEST(NetworkPacketBootstrapTest, ReclaimObjectInvokesResetPacket)
{
  DummyPacket packet;
  packet << static_cast<uint16_t>(512);
  packet.encrypted(true);
  packet.sentSize = 64;

  packet.onReclaimObject();

  EXPECT_TRUE(packet.empty());
  EXPECT_FALSE(packet.encrypted());
  EXPECT_EQ(packet.sentSize, 0u);
  EXPECT_EQ(packet.messageID(), 0);
}
