#include <gtest/gtest.h>

#include "network/channel.h"
#include "network/fixed_messages.h"
#include "network/message_handler.h"
#include "network/packet_reader.h"
#include "network/tcp_packet.h"
#include "helper/debug_helper.h"
#include "resmgr/resmgr.h"

namespace {

class NoopMessageHandler : public KBEngine::Network::MessageHandler
{
public:
  NoopMessageHandler()
  {
    msgLen = -1;
    name = "noop";
  }

  void handle(KBEngine::Network::Channel*, KBEngine::MemoryStream&) override {}
};

TEST(NetworkPacketReaderBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

class NetworkPacketReaderLimitTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!KBEngine::DebugHelper::isInit())
    {
      KBEngine::DebugHelper::initialize(KBEngine::UNKNOWN_COMPONENT_TYPE);
    }
    // The linked interface libraries register static MessageHandlers objects
    // whose constructors run before main() and create the FixedMessages
    // singleton, which in turn news a Resmgr.  Earlier tests (e.g.
    // ServerServerConfig via ScopedResmgrEnvironment, or NetworkFixedMessages)
    // may also create/destroy these singletons at runtime.  Reset both so the
    // test body below reconstructs a consistent FixedMessages+Resmgr pair:
    // leaving FixedMessages alive while deleting Resmgr makes its later
    // loadConfig() assert in Resmgr::getSingleton().
    if (KBEngine::Network::FixedMessages::getSingletonPtr() != nullptr)
    {
      delete KBEngine::Network::FixedMessages::getSingletonPtr();
    }
    if (KBEngine::Resmgr::getSingletonPtr() != nullptr)
    {
      delete KBEngine::Resmgr::getSingletonPtr();
    }
  }

  void TearDown() override
  {
    KBEngine::DebugHelper::finalise(true);
  }
};

TEST_F(NetworkPacketReaderLimitTest, RejectsExtendedLengthOverHardLimit)
{
  KBEngine::Network::FixedMessages* fm = KBEngine::Network::FixedMessages::getSingletonPtr();
  bool created = false;
  if (!fm)
  {
    fm = new KBEngine::Network::FixedMessages();
    created = true;
  }

  KBEngine::Network::MessageHandlers handlers("test");
  NoopMessageHandler* handler = new NoopMessageHandler();
  KBEngine::Network::MessageHandler* added = handlers.add("noop", nullptr, -1, handler);
  ASSERT_NE(added, nullptr);

  KBEngine::Network::Channel channel;
  KBEngine::Network::PacketReader reader(&channel);

  // Craft a packet: msgID (2 bytes) + 0xFFFF (2 bytes) + 0xFFFFFFFF (4 bytes).
  KBEngine::Network::TCPPacket packet;
  packet << (KBEngine::Network::MessageID)added->msgID;
  packet << (KBEngine::Network::MessageLength)65535;
  packet << (KBEngine::Network::MessageLength1)0xFFFFFFFFu;

  reader.processMessages(&handlers, &packet);

  EXPECT_GT(channel.condemn(), 0);

  if (created)
    delete fm;
}

}  // namespace
