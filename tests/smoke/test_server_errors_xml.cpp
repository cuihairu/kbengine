#include <gtest/gtest.h>

#include "common/platform.h"
#include "common/md5.h"
#include "resmgr/resmgr.h"
#include "test_resmgr_environment.h"
#include "xml/server_errors_xml.h"

TEST(ServerErrorsXmlTest, LoadsDefaultsWithoutOverrideFile)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_server_errors_defaults_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.system_res_dir() / "server" / "server_errors_defaults.xml",
      R"(<root>
  <ACCOUNT_CREATE_FAILED>
    <id>1</id>
    <descr>default create failed</descr>
  </ACCOUNT_CREATE_FAILED>
  <LOGIN_REJECTED>
    <id>2</id>
    <descr>default login rejected</descr>
  </LOGIN_REJECTED>
</root>)");

  KBEngine::xml::ServerErrorDescriptions errors;
  ASSERT_TRUE(KBEngine::xml::loadServerErrorDescriptions(errors, "ServerErrorsXmlTest"));
  ASSERT_EQ(errors.size(), 2u);

  auto create_failed = errors.find(1);
  ASSERT_NE(create_failed, errors.end());
  EXPECT_EQ(create_failed->second.first, "ACCOUNT_CREATE_FAILED");
  EXPECT_EQ(create_failed->second.second, "default create failed");

  auto login_rejected = errors.find(2);
  ASSERT_NE(login_rejected, errors.end());
  EXPECT_EQ(login_rejected->second.first, "LOGIN_REJECTED");
  EXPECT_EQ(login_rejected->second.second, "default login rejected");
}

TEST(ServerErrorsXmlTest, OverrideFileReplacesMatchingIdsAndKeepsDefaults)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_server_errors_override_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.system_res_dir() / "server" / "server_errors_defaults.xml",
      R"(<root>
  <ACCOUNT_CREATE_FAILED>
    <id>1</id>
    <descr>default create failed</descr>
  </ACCOUNT_CREATE_FAILED>
  <LOGIN_REJECTED>
    <id>2</id>
    <descr>default login rejected</descr>
  </LOGIN_REJECTED>
</root>)");

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "server_errors.xml",
      R"(<root>
  <ACCOUNT_CREATE_FAILED_OVERRIDE>
    <id>1</id>
    <descr>override create failed</descr>
  </ACCOUNT_CREATE_FAILED_OVERRIDE>
  <PASSWORD_INVALID>
    <id>3</id>
    <descr>password invalid</descr>
  </PASSWORD_INVALID>
</root>)");

  KBEngine::xml::ServerErrorDescriptions errors;
  ASSERT_TRUE(KBEngine::xml::loadServerErrorDescriptions(errors, "ServerErrorsXmlTest"));
  ASSERT_EQ(errors.size(), 3u);

  auto overridden = errors.find(1);
  ASSERT_NE(overridden, errors.end());
  EXPECT_EQ(overridden->second.first, "ACCOUNT_CREATE_FAILED_OVERRIDE");
  EXPECT_EQ(overridden->second.second, "override create failed");

  auto preserved = errors.find(2);
  ASSERT_NE(preserved, errors.end());
  EXPECT_EQ(preserved->second.first, "LOGIN_REJECTED");
  EXPECT_EQ(preserved->second.second, "default login rejected");

  auto added = errors.find(3);
  ASSERT_NE(added, errors.end());
  EXPECT_EQ(added->second.first, "PASSWORD_INVALID");
  EXPECT_EQ(added->second.second, "password invalid");
}

TEST(ServerErrorsXmlTest, InvalidOverrideFileFailsLoading)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_server_errors_invalid_override_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.system_res_dir() / "server" / "server_errors_defaults.xml",
      R"(<root>
  <ACCOUNT_CREATE_FAILED>
    <id>1</id>
    <descr>default create failed</descr>
  </ACCOUNT_CREATE_FAILED>
</root>)");

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "server_errors.xml",
      "<root><BROKEN>");

  KBEngine::xml::ServerErrorDescriptions errors;
  EXPECT_FALSE(KBEngine::xml::loadServerErrorDescriptions(errors, "ServerErrorsXmlTest"));
}

TEST(ServerErrorsXmlTest, DigestMatchesLegacyDefaultsThenOverrideSequence)
{
  KBEngineTest::ScopedResmgrEnvironment env("kbengine_server_errors_digest_test");
  ASSERT_TRUE(env.ready());

  KBEngineTest::write_file(
      env.system_res_dir() / "server" / "server_errors_defaults.xml",
      R"(<root>
  <ACCOUNT_CREATE_FAILED>
    <id>1</id>
    <descr>default create failed</descr>
  </ACCOUNT_CREATE_FAILED>
  <LOGIN_REJECTED>
    <id>2</id>
    <descr>default login rejected</descr>
  </LOGIN_REJECTED>
</root>)");

  KBEngineTest::write_file(
      env.user_res_dir() / "server" / "server_errors.xml",
      R"(<root>
  <ACCOUNT_CREATE_FAILED_OVERRIDE>
    <id>1</id>
    <descr>override create failed</descr>
  </ACCOUNT_CREATE_FAILED_OVERRIDE>
  <PASSWORD_INVALID>
    <id>3</id>
    <descr>password invalid</descr>
  </PASSWORD_INVALID>
</root>)");

  KBEngine::KBE_MD5 actual_md5;
  int32 actual_size = 0;
  ASSERT_TRUE(KBEngine::xml::appendServerErrorDescriptionsDigest(
      actual_md5, actual_size, "ServerErrorsXmlTest"));

  KBEngine::KBE_MD5 expected_md5;
  int32 isize = 0;
  int32 val = 1;
  expected_md5.append((void*)&val, sizeof(int32));
  std::string name = "ACCOUNT_CREATE_FAILED";
  expected_md5.append((void*)name.c_str(), name.size());
  std::string descr = "default create failed";
  expected_md5.append((void*)descr.c_str(), descr.size());
  isize++;

  val = 2;
  expected_md5.append((void*)&val, sizeof(int32));
  name = "LOGIN_REJECTED";
  expected_md5.append((void*)name.c_str(), name.size());
  descr = "default login rejected";
  expected_md5.append((void*)descr.c_str(), descr.size());
  isize++;
  expected_md5.append((void*)&isize, sizeof(int32));

  val = 1;
  expected_md5.append((void*)&val, sizeof(int32));
  name = "ACCOUNT_CREATE_FAILED_OVERRIDE";
  expected_md5.append((void*)name.c_str(), name.size());
  descr = "override create failed";
  expected_md5.append((void*)descr.c_str(), descr.size());
  isize++;

  val = 3;
  expected_md5.append((void*)&val, sizeof(int32));
  name = "PASSWORD_INVALID";
  expected_md5.append((void*)name.c_str(), name.size());
  descr = "password invalid";
  expected_md5.append((void*)descr.c_str(), descr.size());
  isize++;
  expected_md5.append((void*)&isize, sizeof(int32));

  EXPECT_EQ(actual_size, isize);
  EXPECT_EQ(actual_md5.getDigestStr(), expected_md5.getDigestStr());
}
