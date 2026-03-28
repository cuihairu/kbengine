#include <gtest/gtest.h>

#include "network/interfaces.h"

namespace {

struct InputHandler final : KBEngine::Network::InputNotificationHandler {
  int last_fd = -1;

  int handleInputNotification(int fd) override
  {
    last_fd = fd;
    return fd + 1;
  }
};

struct OutputHandler final : KBEngine::Network::OutputNotificationHandler {
  int last_fd = -1;

  int handleOutputNotification(int fd) override
  {
    last_fd = fd;
    return fd + 2;
  }
};

struct TimeoutHandler final : KBEngine::Network::ChannelTimeOutHandler {
  KBEngine::Network::Channel* last_channel = nullptr;

  void onChannelTimeOut(KBEngine::Network::Channel* pChannel) override
  {
    last_channel = pChannel;
  }
};

struct DeregisterHandler final : KBEngine::Network::ChannelDeregisterHandler {
  KBEngine::Network::Channel* last_channel = nullptr;

  void onChannelDeregister(KBEngine::Network::Channel* pChannel) override
  {
    last_channel = pChannel;
  }
};

struct StatsHandler final : KBEngine::Network::NetworkStatsHandler {
  int sent_size = 0;
  int recv_size = 0;
  const KBEngine::Network::MessageHandler* sent_handler = nullptr;
  const KBEngine::Network::MessageHandler* recv_handler = nullptr;

  void onSendMessage(const KBEngine::Network::MessageHandler& msgHandler, int size) override
  {
    sent_handler = &msgHandler;
    sent_size = size;
  }

  void onRecvMessage(const KBEngine::Network::MessageHandler& msgHandler, int size) override
  {
    recv_handler = &msgHandler;
    recv_size = size;
  }
};

} // namespace

TEST(NetworkInterfacesBootstrapTest, NotificationHandlersReturnDerivedValues)
{
  InputHandler input;
  OutputHandler output;

  EXPECT_EQ(input.handleInputNotification(7), 8);
  EXPECT_EQ(input.last_fd, 7);
  EXPECT_EQ(output.handleOutputNotification(9), 11);
  EXPECT_EQ(output.last_fd, 9);
}

TEST(NetworkInterfacesBootstrapTest, ChannelHandlersCapturePointers)
{
  TimeoutHandler timeout;
  DeregisterHandler deregister;

  auto* channel = reinterpret_cast<KBEngine::Network::Channel*>(0x1234);
  timeout.onChannelTimeOut(channel);
  deregister.onChannelDeregister(channel);

  EXPECT_EQ(timeout.last_channel, channel);
  EXPECT_EQ(deregister.last_channel, channel);
}

TEST(NetworkInterfacesBootstrapTest, StatsHandlerCapturesSizesAndMessagePointers)
{
  StatsHandler handler;
  auto* message = reinterpret_cast<const KBEngine::Network::MessageHandler*>(0x5678);

  handler.onSendMessage(*message, 12);
  handler.onRecvMessage(*message, 34);

  EXPECT_EQ(handler.sent_handler, message);
  EXPECT_EQ(handler.recv_handler, message);
  EXPECT_EQ(handler.sent_size, 12);
  EXPECT_EQ(handler.recv_size, 34);
}
