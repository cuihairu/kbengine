#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "common/strutil.h"

TEST(CommonStrutilBootstrapTest, ConvertsCaseAndTrimsWhitespace) {
  EXPECT_EQ(KBEngine::strutil::toLower("AbC_123"), "abc_123");
  EXPECT_EQ(KBEngine::strutil::toUpper("AbC_123"), "ABC_123");
  EXPECT_EQ(KBEngine::strutil::kbe_trim(" \t KBEngine \n "), "KBEngine");
}

TEST(CommonStrutilBootstrapTest, ReplacesAndSplitsText) {
  std::string replaced = "a::b::";
  EXPECT_EQ(KBEngine::strutil::kbe_replace(replaced, "::", "/"), 2);
  EXPECT_EQ(replaced, "a/b/");

  std::vector<std::string> parts;
  EXPECT_EQ(KBEngine::strutil::kbe_splits("alpha,,beta,", ",", parts, true), 4);
  ASSERT_EQ(parts.size(), 4u);
  EXPECT_EQ(parts[0], "alpha");
  EXPECT_EQ(parts[1], "");
  EXPECT_EQ(parts[2], "beta");
  EXPECT_EQ(parts[3], "");
}

TEST(CommonStrutilBootstrapTest, EncodesAndDecodesHexBytes) {
  unsigned char source[] = {0xDE, 0xAD, 0xBE, 0xEF};
  unsigned char encoded[16] = {0};
  unsigned char decoded[4] = {0};

  EXPECT_EQ(KBEngine::strutil::bytes2string(source, 4, encoded, sizeof(encoded)), 8);
  EXPECT_STREQ(reinterpret_cast<const char*>(encoded), "DEADBEEF");

  unsigned char mutable_encoded[] = "deadbeef";
  EXPECT_EQ(KBEngine::strutil::string2bytes(mutable_encoded, decoded, sizeof(decoded)), 4);
  EXPECT_EQ(decoded[0], 0xDE);
  EXPECT_EQ(decoded[1], 0xAD);
  EXPECT_EQ(decoded[2], 0xBE);
  EXPECT_EQ(decoded[3], 0xEF);
}
