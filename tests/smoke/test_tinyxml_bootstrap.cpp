#include <gtest/gtest.h>

#include <string>

#include <tinyxml2.h>

TEST(TinyXmlBootstrapTest, ParsesBasicDocument) {
  const std::string xml = "<root><server><port>6000</port></server></root>";

  tinyxml2::XMLDocument doc;
  doc.Parse(xml.c_str());

  ASSERT_FALSE(doc.Error());
  auto* root = doc.RootElement();
  ASSERT_NE(root, nullptr);
  EXPECT_STREQ(root->Value(), "root");

  auto* server = root->FirstChildElement("server");
  ASSERT_NE(server, nullptr);
  auto* port = server->FirstChildElement("port");
  ASSERT_NE(port, nullptr);
  ASSERT_NE(port->GetText(), nullptr);
  EXPECT_STREQ(port->GetText(), "6000");
}
