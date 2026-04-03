#include <gtest/gtest.h>

#include "server/sendmail_threadtasks.h"
#include "server/serverconfig.h"

namespace
{
class TestSendMailTask : public KBEngine::SendEMailTask
{
public:
  TestSendMailTask(const std::string& emailaddr,
                   const std::string& code,
                   const std::string& cbaddr,
                   KBEngine::uint32 cbport,
                   const std::string& opkey,
                   const std::string& subject,
                   const std::string& message)
      : KBEngine::SendEMailTask(emailaddr, code, cbaddr, cbport),
        opkey_(opkey),
        subject_(subject),
        message_(message)
  {
  }

  const char* getopkey() override
  {
    return opkey_.c_str();
  }

  const char* subject() override
  {
    return subject_.c_str();
  }

  const char* message() override
  {
    return message_.c_str();
  }

private:
  std::string opkey_;
  std::string subject_;
  std::string message_;
};
}

TEST(SendmailRequestTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(SendmailRequestTest, BuildsRequestForPlainSmtp)
{
  KBEngine::EmailServerInfo info;
  info.smtp_server = "smtp.example.com";
  info.smtp_port = 25;
  info.username = "sender@example.com";
  info.password = "secret";
  info.smtp_auth = 1;

  TestSendMailTask task(
      "player@example.com",
      "ABC123",
      "127.0.0.1",
      20086,
      "accountactivate",
      "Activate",
      "Hello ${username}, code=${code}, link=${backlink}");

  const auto request = task.prepareRequest(info);

  ASSERT_TRUE(request.isValid());
  EXPECT_EQ(request.smtpUrl, "smtp://smtp.example.com:25");
  EXPECT_EQ(request.fromAddress, "sender@example.com");
  EXPECT_EQ(request.toAddress, "player@example.com");
  EXPECT_EQ(request.subjectLine, "Activate");
  EXPECT_EQ(request.mailFrom, "<sender@example.com>");
  EXPECT_EQ(request.username, "sender@example.com");
  EXPECT_EQ(request.password, "secret");
  EXPECT_EQ(request.loginOptions, "AUTH=LOGIN");
  EXPECT_FALSE(request.useSsl);
  EXPECT_EQ(request.messageHtml,
            "Hello player@example.com, code=ABC123, "
            "link=http://127.0.0.1:20086/accountactivate_ABC123");
}

TEST(SendmailRequestTest, BuildsRequestForSmtpsAndPlainAuth)
{
  KBEngine::EmailServerInfo info;
  info.smtp_server = "mail.example.com";
  info.smtp_port = 465;
  info.username = "mailer@example.com";
  info.password = "pwd";
  info.smtp_auth = 2;

  TestSendMailTask task(
      "bind@example.com",
      "ZXCV",
      "login.example.com",
      8443,
      "bindmail",
      "Bind",
      "<p>${backlink}</p>");

  const auto request = task.prepareRequest(info);

  ASSERT_TRUE(request.isValid());
  EXPECT_EQ(request.smtpUrl, "smtps://mail.example.com:465");
  EXPECT_EQ(request.loginOptions, "AUTH=PLAIN");
  EXPECT_TRUE(request.useSsl);
  EXPECT_EQ(request.messageHtml, "<p>http://login.example.com:8443/bindmail_ZXCV</p>");
}

TEST(SendmailRequestTest, KeepsExplicitSchemeInServerAddress)
{
  KBEngine::EmailServerInfo info;
  info.smtp_server = "smtp://relay.example.com";
  info.smtp_port = 587;
  info.username = "relay@example.com";

  TestSendMailTask task(
      "user@example.com",
      "KBE",
      "cb.example.com",
      9000,
      "resetpassword",
      "Reset",
      "${code}");

  const auto request = task.prepareRequest(info);

  ASSERT_TRUE(request.isValid());
  EXPECT_EQ(request.smtpUrl, "smtp://relay.example.com:587");
}

TEST(SendmailRequestTest, InvalidWhenRequiredFieldsMissing)
{
  KBEngine::EmailServerInfo info;
  info.smtp_server = "";
  info.smtp_port = 25;
  info.username = " ";

  TestSendMailTask task(
      "   ",
      "KBE",
      "cb.example.com",
      9000,
      "resetpassword",
      "Reset",
      "${code}");

  const auto request = task.prepareRequest(info);

  EXPECT_FALSE(request.isValid());
  EXPECT_EQ(request.smtpUrl, "smtp://:25");
  EXPECT_TRUE(request.fromAddress.empty());
  EXPECT_TRUE(request.toAddress.empty());
}
