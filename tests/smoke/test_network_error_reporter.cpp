#include <gtest/gtest.h>

#include "network/error_reporter.h"

TEST(NetworkErrorReporterBootstrapTest, RecordsFirstErrorByAddressAndMessage)
{
  KBEngine::Network::EventDispatcher dispatcher;
  KBEngine::Network::ErrorReporter reporter(dispatcher);
  KBEngine::Network::Address address("127.0.0.1", 4000);

  reporter.reportException(KBEngine::Network::REASON_CHANNEL_LOST, address, "network", "while testing");
  reporter.reportPendingExceptions(true);

  SUCCEED();
}

TEST(NetworkErrorReporterBootstrapTest, HandlesRepeatedReportsAndCleanup)
{
  KBEngine::Network::EventDispatcher dispatcher;
  KBEngine::Network::ErrorReporter reporter(dispatcher);
  KBEngine::Network::Address address("127.0.0.1", 4001);

  reporter.reportException(KBEngine::Network::REASON_RESOURCE_UNAVAILABLE, address);
  reporter.reportException(KBEngine::Network::REASON_RESOURCE_UNAVAILABLE, address);
  reporter.reportPendingExceptions(true);
  reporter.reportPendingExceptions(false);

  SUCCEED();
}
