#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "xml/xml.h"

namespace {

std::filesystem::path make_temp_xml_path(const char* suffix)
{
  const auto temp_dir = std::filesystem::temp_directory_path();
  return temp_dir / (std::string("kbengine_xml_wrapper_") + suffix + ".xml");
}

}  // namespace

TEST(XmlWrapperTest, ReadsNestedNodesAndTypedValues)
{
  const auto xml_path = make_temp_xml_path("typed_values");
  std::ofstream out(xml_path);
  out << R"(<root>
  <message>
    <id>42</id>
    <descr> hello tinyxml2 </descr>
    <enabled>true</enabled>
    <ratio>3.5</ratio>
  </message>
</root>)";
  out.close();

  KBEngine::XML xml(xml_path.string().c_str());
  ASSERT_TRUE(xml.isGood());

  tinyxml2::XMLNode* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);
  EXPECT_EQ(xml.getKey(root), "message");

  tinyxml2::XMLNode* id = xml.enterNode(root->FirstChild(), "id");
  tinyxml2::XMLNode* descr = xml.enterNode(root->FirstChild(), "descr");
  tinyxml2::XMLNode* enabled = xml.enterNode(root->FirstChild(), "enabled");
  tinyxml2::XMLNode* ratio = xml.enterNode(root->FirstChild(), "ratio");

  ASSERT_NE(id, nullptr);
  ASSERT_NE(descr, nullptr);
  ASSERT_NE(enabled, nullptr);
  ASSERT_NE(ratio, nullptr);

  EXPECT_EQ(xml.getValInt(id), 42);
  EXPECT_EQ(xml.getValStr(descr), "hello tinyxml2");
  EXPECT_EQ(xml.getVal(descr), " hello tinyxml2 ");
  EXPECT_TRUE(xml.getBool(enabled));
  EXPECT_DOUBLE_EQ(xml.getValFloat(ratio), 3.5);
  EXPECT_TRUE(xml.hasNode(root->FirstChild(), "descr"));
  EXPECT_FALSE(xml.hasNode(root->FirstChild(), "missing"));

  std::filesystem::remove(xml_path);
}

TEST(XmlWrapperTest, SupportsKeyedRootLookupAndMissingFiles)
{
  const auto xml_path = make_temp_xml_path("key_lookup");
  std::ofstream out(xml_path);
  out << R"(<root>
  <settings>
    <port>6000</port>
  </settings>
</root>)";
  out.close();

  KBEngine::XML xml(xml_path.string().c_str());
  ASSERT_TRUE(xml.isGood());

  tinyxml2::XMLNode* root = xml.getRootNode("settings");
  ASSERT_NE(root, nullptr);
  EXPECT_EQ(xml.getKey(root->Parent()), "settings");
  EXPECT_EQ(xml.getKey(root), "port");
  ASSERT_NE(root->FirstChild(), nullptr);
  EXPECT_EQ(xml.getValInt(root->FirstChild()), 6000);

  KBEngine::XML missing((xml_path.string() + ".missing").c_str());
  EXPECT_FALSE(missing.isGood());
  EXPECT_EQ(missing.getRootNode(), nullptr);

  std::filesystem::remove(xml_path);
}
