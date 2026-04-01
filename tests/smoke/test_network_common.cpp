#include <gtest/gtest.h>

#include <thread>

#include "network/common.h"

#if KBE_PLATFORM != PLATFORM_WIN32
#include <unistd.h>
#endif

namespace KBEngine {
void invoke_close_channel_inactivity_detection()
{
  CLOSE_CHANNEL_INACTIVITIY_DETECTION();
}
} // namespace KBEngine

TEST(NetworkCommonBootstrapTest, ExposesExpectedDefaults)
{
  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelInternalTimeout, 60.f);
  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelExternalTimeout, 60.f);
  EXPECT_EQ(KBEngine::Network::g_SOMAXCONN, 5u);
  EXPECT_STREQ(KBEngine::Network::UDP_HELLO, "62a559f3fa7748bc22f8e0766019d498");
  EXPECT_STREQ(KBEngine::Network::reasonToString(KBEngine::Network::REASON_SUCCESS), "REASON_SUCCESS");
  EXPECT_STREQ(KBEngine::Network::reasonToString(KBEngine::Network::REASON_CHANNEL_CONDEMN), "REASON_CHANNEL_CONDEMN");
}

TEST(NetworkCommonBootstrapTest, CloseChannelInactivityDetectionMacroUpdatesTimeouts)
{
  KBEngine::Network::g_channelInternalTimeout = 12.f;
  KBEngine::Network::g_channelExternalTimeout = 18.f;

  KBEngine::invoke_close_channel_inactivity_detection();

  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelInternalTimeout, -1.f);
  EXPECT_FLOAT_EQ(KBEngine::Network::g_channelExternalTimeout, -1.f);

  KBEngine::Network::g_channelInternalTimeout = 60.f;
  KBEngine::Network::g_channelExternalTimeout = 60.f;
}

TEST(NetworkCommonBootstrapTest, InitializeAndFinalizeSucceedWithoutBootstrapPath)
{
  ASSERT_TRUE(KBEngine::Network::initialize());
  KBEngine::Network::finalise();
}

#if KBE_PLATFORM != PLATFORM_WIN32
TEST(NetworkCommonBootstrapTest, PollDetectsReadableFileDescriptor)
{
  int fds[2];
  ASSERT_EQ(::pipe(fds), 0);

  std::thread writer([write_fd = fds[1]]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const char value = 'x';
    ::write(write_fd, &value, 1);
    ::close(write_fd);
  });

  EXPECT_TRUE(KBEngine::Network::kbe_poll(fds[0]));

  char value = 0;
  ::read(fds[0], &value, 1);
  ::close(fds[0]);
  writer.join();
  EXPECT_EQ(value, 'x');
}
#endif
