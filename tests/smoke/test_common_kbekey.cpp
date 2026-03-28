#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <unistd.h>

#include "common/common.h"
#include "common/kbekey.h"

namespace KBEngine
{
COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
GAME_TIME g_kbetime = 0;
}

namespace
{
std::filesystem::path make_temp_key_path(const char* suffix)
{
  const auto temp_dir = std::filesystem::temp_directory_path();
  const auto unique = std::to_string(::getpid()) + std::string("_kbe_kbekey_") + suffix;
  return temp_dir / unique;
}
}

TEST(CommonKBEKeyBootstrapTest, ServerModeGeneratesAndLoadsKeyPair)
{
  const auto public_key = make_temp_key_path("server_pub.pem");
  const auto private_key = make_temp_key_path("server_pri.pem");

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);

  KBEngine::g_componentType = KBEngine::BASEAPP_TYPE;
  KBEngine::KBEKey key(public_key.string(), private_key.string());
  ASSERT_TRUE(key.isGood());

  const std::string plain = "kbengine-kbekey-server";
  const auto encrypted = key.encrypt(plain);
  ASSERT_FALSE(encrypted.empty());
  EXPECT_EQ(key.decrypt(encrypted), plain);

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);
}

TEST(CommonKBEKeyBootstrapTest, ClientModeLoadsPublicKeyOnly)
{
  const auto public_key = make_temp_key_path("client_pub.pem");
  const auto private_key = make_temp_key_path("client_pri.pem");

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);

  {
    KBEngine::KBE_RSA generator;
    ASSERT_TRUE(generator.generateKey(public_key.string(), private_key.string(), 1024, 65537));
  }

  KBEngine::g_componentType = KBEngine::CLIENT_TYPE;
  KBEngine::KBEKey key(public_key.string(), "");
  EXPECT_TRUE(key.isGood());

  std::filesystem::remove(public_key);
  std::filesystem::remove(private_key);
}
