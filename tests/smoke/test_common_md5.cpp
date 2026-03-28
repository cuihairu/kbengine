#include <gtest/gtest.h>

#include <string>

#include "common/md5.h"

TEST(CommonMd5BootstrapTest, ComputesKnownDigest) {
  const std::string text = "abc";

  EXPECT_EQ(KBEngine::KBE_MD5::getDigest(text.data(), static_cast<int>(text.size())),
            "900150983CD24FB0D6963F7D28E17F72");
}

TEST(CommonMd5BootstrapTest, SupportsIncrementalAppend) {
  KBEngine::KBE_MD5 md5;
  md5.append("a", 1);
  md5.append("b", 1);
  md5.append("c", 1);

  EXPECT_TRUE(md5.isFinal() == false);
  EXPECT_EQ(md5.getDigestStr(), "900150983CD24FB0D6963F7D28E17F72");
  EXPECT_TRUE(md5.isFinal());
}
