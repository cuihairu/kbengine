#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include "../test_platform.h"

#include "common/rsa.h"

TEST(CommonRsaBootstrapTest, GeneratesKeysAndRoundTripsCiphertext) {
  const auto temp_dir = std::filesystem::temp_directory_path();
  const auto unique = std::to_string(KBE_GETPID()) + "_kbe_rsa_test";
  const auto public_key = temp_dir / (unique + "_pub.pem");
  const auto private_key = temp_dir / (unique + "_pri.pem");

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);

  KBEngine::KBE_RSA rsa;
  ASSERT_TRUE(rsa.generateKey(public_key.string(), private_key.string(), 1024, 65537));
  ASSERT_TRUE(rsa.isGood());

  const std::string plain = "kbengine-rsa";
  const auto encrypted_hex = rsa.encrypt(plain);
  ASSERT_FALSE(encrypted_hex.empty());

  const auto decrypted = rsa.decrypt(encrypted_hex);
  EXPECT_EQ(decrypted, plain);

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);
}

TEST(CommonRsaBootstrapTest, LoadsGeneratedKeyPairFromDisk) {
  const auto temp_dir = std::filesystem::temp_directory_path();
  const auto unique = std::to_string(KBE_GETPID()) + "_kbe_rsa_reload";
  const auto public_key = temp_dir / (unique + "_pub.pem");
  const auto private_key = temp_dir / (unique + "_pri.pem");

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);

  {
    KBEngine::KBE_RSA generator;
    ASSERT_TRUE(generator.generateKey(public_key.string(), private_key.string(), 1024, 65537));
  }

  KBEngine::KBE_RSA loaded(public_key.string(), private_key.string());
  ASSERT_TRUE(loaded.isGood());

  std::string encrypted;
  ASSERT_GT(loaded.encrypt("reload", encrypted), 0);

  std::string decrypted;
  ASSERT_GT(loaded.decrypt(encrypted, decrypted), 0);
  EXPECT_EQ(decrypted, "reload");

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);
}
