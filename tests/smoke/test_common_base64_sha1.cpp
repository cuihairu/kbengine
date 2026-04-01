#include <gtest/gtest.h>

#include <string>

#include "common/base64.h"
#include "common/sha1.h"

TEST(CommonCoreBootstrapTest, Base64RoundTrip) {
  const std::string plain = "KBEngine";
  const auto encoded = base64_encode(
    reinterpret_cast<const unsigned char*>(plain.data()),
    static_cast<unsigned int>(plain.size()));

  EXPECT_EQ(encoded, "S0JFbmdpbmU=");
  EXPECT_EQ(base64_decode(encoded), plain);
}

TEST(CommonCoreBootstrapTest, Sha1KnownVector) {
  KBEngine::KBE_SHA1 sha1;
  unsigned digest[5] = {0};

  const char* text = "abc";
  sha1.Input(text, 3);

  ASSERT_TRUE(sha1.Result(digest));
  EXPECT_EQ(digest[0], 0xA9993E36u);
  EXPECT_EQ(digest[1], 0x4706816Au);
  EXPECT_EQ(digest[2], 0xBA3E2571u);
  EXPECT_EQ(digest[3], 0x7850C26Cu);
  EXPECT_EQ(digest[4], 0x9CD0D89Du);
}
