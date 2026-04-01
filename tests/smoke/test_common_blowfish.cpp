#include <gtest/gtest.h>

#include <array>
#include <string>

#include "common/blowfish.h"

TEST(CommonBlowfishBootstrapTest, EncryptsAndDecryptsBlockAlignedData)
{
  KBEngine::KBEBlowfish blowfish("0123456789ABCDEF");
  ASSERT_TRUE(blowfish.isGood());

  const std::string plain = "12345678ABCDEFGH";
  std::array<unsigned char, 16> encrypted{};
  std::array<unsigned char, 16> decrypted{};

  ASSERT_EQ(blowfish.encrypt(reinterpret_cast<const unsigned char*>(plain.data()), encrypted.data(),
              static_cast<int>(plain.size())),
    static_cast<int>(plain.size()));
  ASSERT_NE(std::string(reinterpret_cast<const char*>(encrypted.data()), encrypted.size()), plain);

  ASSERT_EQ(blowfish.decrypt(encrypted.data(), decrypted.data(), static_cast<int>(encrypted.size())),
    static_cast<int>(plain.size()));
  EXPECT_EQ(std::string(reinterpret_cast<const char*>(decrypted.data()), decrypted.size()), plain);
}

TEST(CommonBlowfishBootstrapTest, RejectsInvalidInputLength)
{
  KBEngine::KBEBlowfish blowfish("0123456789ABCDEF");
  ASSERT_TRUE(blowfish.isGood());

  std::array<unsigned char, 15> output{};
  EXPECT_EQ(blowfish.encrypt(reinterpret_cast<const unsigned char*>("123456789012345"), output.data(), 15), -1);
  EXPECT_EQ(blowfish.decrypt(output.data(), output.data(), 15), -1);
}
