#include <gtest/gtest.h>

// Undefine CODE_INLINE to avoid endpoint.inl which has platform-specific code
// that does not handle PLATFORM_APPLE correctly.
#undef CODE_INLINE

#include "network/endpoint.h"

TEST(NetworkEndpointBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}
