#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <Python.h>

#include "entitydef/common.h"
#include "entitydef/datatypes.h"
#include "entitydef/entity_component.h"
#define private public
#include "entitydef/entitydef.h"
#undef private
#include "entitydef/py_entitydef.h"
#include "entitydef/property.h"
#include "entitydef/method.h"
#include "entitydef/scriptdef_module.h"
#include "helper/debug_option.h"
#include "helper/debug_helper.h"
#include "resmgr/resmgr.h"
#include "xml/xml.h"

namespace {

namespace fs = std::filesystem;

void write_file(const fs::path& path, const std::string& contents)
{
  fs::create_directories(path.parent_path());
  std::ofstream out(path);
  ASSERT_TRUE(out.is_open()) << path;
  out << contents;
}

void ensure_python_runtime()
{
  if (!Py_IsInitialized())
  {
    Py_Initialize();
  }

  PyObject* kbe_module = PyImport_AddModule("KBEngine");
  ASSERT_NE(kbe_module, nullptr);

  if (!PyObject_HasAttrString(kbe_module, "Entity"))
  {
    PyObject* entity_module = PyModule_New("Entity");
    ASSERT_NE(entity_module, nullptr);
    ASSERT_EQ(PyModule_AddObject(kbe_module, "Entity", entity_module), 0);
  }
}

void prepend_python_path(const std::string& path)
{
  PyObject* sys_path = PySys_GetObject(const_cast<char*>("path"));
  ASSERT_NE(sys_path, nullptr);

  PyObject* py_path = PyUnicode_FromString(path.c_str());
  ASSERT_NE(py_path, nullptr);
  ASSERT_EQ(PyList_Insert(sys_path, 0, py_path), 0);
  Py_DECREF(py_path);
}

void initialize_entity_flags()
{
  KBEngine::g_entityFlagMapping.clear();
  KBEngine::g_entityFlagMapping["CELL"] = KBEngine::ED_FLAG_CELL_PUBLIC;
  KBEngine::g_entityFlagMapping["CELL_AND_CLIENT"] = KBEngine::ED_FLAG_CELL_PUBLIC_AND_OWN;
  KBEngine::g_entityFlagMapping["CELL_AND_CLIENTS"] = KBEngine::ED_FLAG_ALL_CLIENTS;
  KBEngine::g_entityFlagMapping["CELL_AND_OTHER_CLIENTS"] = KBEngine::ED_FLAG_OTHER_CLIENTS;
  KBEngine::g_entityFlagMapping["BASE_AND_CLIENT"] = KBEngine::ED_FLAG_BASE_AND_CLIENT;
  KBEngine::g_entityFlagMapping["BASE"] = KBEngine::ED_FLAG_BASE;
  KBEngine::g_entityFlagMapping["CELL_PUBLIC"] = KBEngine::ED_FLAG_CELL_PUBLIC;
  KBEngine::g_entityFlagMapping["CELL_PRIVATE"] = KBEngine::ED_FLAG_CELL_PRIVATE;
  KBEngine::g_entityFlagMapping["ALL_CLIENTS"] = KBEngine::ED_FLAG_ALL_CLIENTS;
  KBEngine::g_entityFlagMapping["CELL_PUBLIC_AND_OWN"] = KBEngine::ED_FLAG_CELL_PUBLIC_AND_OWN;
  KBEngine::g_entityFlagMapping["OTHER_CLIENTS"] = KBEngine::ED_FLAG_OTHER_CLIENTS;
  KBEngine::g_entityFlagMapping["OWN_CLIENT"] = KBEngine::ED_FLAG_OWN_CLIENT;
}

struct EntityDefTestEnv
{
  fs::path root;
  fs::path scripts_dir;
  fs::path defs_dir;
  bool ready;

  EntityDefTestEnv()
      : root(fs::temp_directory_path() / fs::path("kbengine_entitydef_xml_tests")),
        scripts_dir(root / "assets" / "scripts"),
        defs_dir(scripts_dir / "entity_defs"),
        ready(false)
  {
    fs::remove_all(root);
    fs::create_directories(root / "kbe" / "res" / "server");
    fs::create_directories(root / "assets" / "res" / "server");
    fs::create_directories(defs_dir / "interfaces");
    fs::create_directories(defs_dir / "components");
    fs::create_directories(scripts_dir / "base" / "components");
    fs::create_directories(scripts_dir / "cell" / "components");

    write_file(root / "kbe" / "res" / "server" / "kbengine_defaults.xml", "<root/>\n");
    write_file(root / "assets" / "res" / "server" / "kbengine.xml", "<root/>\n");
    write_file(scripts_dir / "entities.xml", "<root><Avatar hasCell=\"true\" hasBase=\"true\" hasClient=\"true\"/></root>\n");
    write_file(scripts_dir / "base" / "components" / "Inventory.py", "# base component\n");
    write_file(scripts_dir / "cell" / "components" / "Inventory.py", "# cell component\n");

    const std::string res_path =
        (root / "kbe" / "res").string() + ";" +
        (root / "assets").string() + ";" +
        (root / "assets" / "scripts").string() + ";" +
        (root / "assets" / "res").string();

    ::setenv("KBE_ROOT", root.string().c_str(), 1);
    ::setenv("KBE_RES_PATH", res_path.c_str(), 1);
    ::setenv("KBE_BIN_PATH", (root / "kbe" / "bin" / "server").string().c_str(), 1);

    if (KBEngine::Resmgr::getSingletonPtr() == nullptr)
    {
      new KBEngine::Resmgr();
    }

    ready = KBEngine::Resmgr::getSingleton().initialize();
  }

  ~EntityDefTestEnv()
  {
    fs::remove_all(root);
  }
};

class EntityDefXmlLoadingTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!KBEngine::DebugHelper::isInit())
    {
      KBEngine::DebugHelper::initialize(KBEngine::UNKNOWN_COMPONENT_TYPE);
    }

    ensure_python_runtime();
    KBEngine::EntityDef::finalise();
    KBEngine::DataTypes::finalise();
    initialize_entity_flags();
  }

  void TearDown() override
  {
    KBEngine::EntityDef::finalise();
    KBEngine::DataTypes::finalise();
    initialize_entity_flags();
    KBEngine::DebugHelper::finalise(true);
  }
};

}  // namespace

namespace KBEngine {

PyTypeObject EntityComponent::_scriptType = {PyVarObject_HEAD_INIT(nullptr, 0)};

}  // namespace KBEngine

namespace {

TEST_F(EntityDefXmlLoadingTest, LoadsDefInfoAcrossInterfacesComponentsAndParents)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.defs_dir / "types.xml",
      R"(<root>
  <StatsMap>FIXED_DICT<Properties>
      <level><Type>UINT32</Type><Persistent>true</Persistent></level>
      <tags><Type>ARRAY<of>UINT8</of></Type></tags>
    </Properties></StatsMap>
</root>)");

  write_file(
      env.defs_dir / "ParentEntity.def",
      R"(<root>
  <Properties>
    <title>
      <Flags>BASE</Flags>
      <Type>STRING</Type>
    </title>
  </Properties>
</root>)");

  write_file(
      env.defs_dir / "interfaces" / "SharedIface.def",
      R"(<root>
  <ClientMethods>
    <SyncClient>
      <Utype>320</Utype>
    </SyncClient>
  </ClientMethods>
</root>)");

  write_file(
      env.defs_dir / "interfaces" / "MoveIface.def",
      R"(<root>
  <Properties>
    <speed>
      <Flags>BASE</Flags>
      <Type>UINT32</Type>
    </speed>
  </Properties>
  <BaseMethods>
    <Move>
      <Arg>UINT16</Arg>
      <Utype>210</Utype>
    </Move>
  </BaseMethods>
  <Interfaces>
    <type>
      <SharedIface/>
    </type>
  </Interfaces>
</root>)");

  write_file(
      env.defs_dir / "components" / "Inventory.def",
      R"(<root>
  <Properties>
    <slotCount>
      <Flags>BASE_AND_CLIENT</Flags>
      <Type>UINT16</Type>
    </slotCount>
    <state>
      <Flags>BASE</Flags>
      <Type>StatsMap</Type>
    </state>
  </Properties>
  <CellMethods>
    <Notify>
      <Exposed/>
      <Utype>410</Utype>
    </Notify>
  </CellMethods>
</root>)");

  write_file(
      env.defs_dir / "Avatar.def",
      R"(<root>
  <Properties>
    <health>
      <Flags>BASE_AND_CLIENT</Flags>
      <Persistent>true</Persistent>
      <Type>UINT32</Type>
      <Identifier>true</Identifier>
      <Index>UNIQUE</Index>
      <DatabaseLength>4</DatabaseLength>
      <Default>33</Default>
      <DetailLevel>NEAR</DetailLevel>
      <Utype>101</Utype>
    </health>
    <positionHistory>
      <Flags>BASE</Flags>
      <Type>ARRAY<of>UINT32</of></Type>
    </positionHistory>
  </Properties>
  <CellMethods>
    <Pulse>
      <Arg>ARRAY<of>UINT8</of></Arg>
      <Utype>120</Utype>
    </Pulse>
  </CellMethods>
  <BaseMethods>
    <Spawn>
      <Arg>UINT8</Arg>
      <Utype>121</Utype>
    </Spawn>
  </BaseMethods>
  <ClientMethods>
    <NotifyClient>
      <Utype>122</Utype>
    </NotifyClient>
  </ClientMethods>
  <Interfaces>
    <Interface>
      <MoveIface/>
    </Interface>
  </Interfaces>
  <Components>
    <inventory>
      <Type>
        <Inventory/>
      </Type>
      <Persistent>false</Persistent>
      <Utype>150</Utype>
    </inventory>
    <inventoryBackup>
      <Type>
        <Inventory/>
      </Type>
      <Persistent>true</Persistent>
      <Utype>151</Utype>
    </inventoryBackup>
  </Components>
  <Parent>
    <ParentEntity/>
  </Parent>
  <DetailLevels>
    <NEAR>
      <radius>10</radius>
      <hyst>2</hyst>
    </NEAR>
    <MEDIUM>
      <radius>5</radius>
      <hyst>3</hyst>
    </MEDIUM>
    <FAR>
      <radius>7</radius>
      <hyst>4</hyst>
    </FAR>
  </DetailLevels>
  <Volatile>
    <position>1.5</position>
    <yaw/>
    <pitch>2.5</pitch>
    <roll/>
    <optimized>false</optimized>
  </Volatile>
</root>)");

  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::XML xml((env.defs_dir / "Avatar.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("Avatar", 1);
  ASSERT_TRUE(KBEngine::EntityDef::loadDefInfo(
      env.defs_dir.string() + "/",
      "Avatar",
      &xml,
      root,
      &module));

  EXPECT_NE(module.findBasePropertyDescription("health"), nullptr);
  EXPECT_NE(module.findClientPropertyDescription("health"), nullptr);
  EXPECT_NE(module.findBasePropertyDescription("positionHistory"), nullptr);
  EXPECT_NE(module.findBasePropertyDescription("speed"), nullptr);
  EXPECT_NE(module.findBasePropertyDescription("title"), nullptr);
  EXPECT_NE(module.findBasePropertyDescription("inventory"), nullptr);
  EXPECT_NE(module.findBasePropertyDescription("inventoryBackup"), nullptr);
  EXPECT_NE(module.findClientPropertyDescription("inventory"), nullptr);
  EXPECT_NE(module.findComponentDescription("inventory"), nullptr);
  EXPECT_NE(module.findComponentDescription("inventoryBackup"), nullptr);

  EXPECT_NE(module.findCellMethodDescription("Pulse"), nullptr);
  EXPECT_NE(module.findBaseMethodDescription("Spawn"), nullptr);
  EXPECT_NE(module.findBaseMethodDescription("Move"), nullptr);
  EXPECT_NE(module.findClientMethodDescription("NotifyClient"), nullptr);
  EXPECT_NE(module.findClientMethodDescription("SyncClient"), nullptr);

  const auto& detail = module.getDetailLevel();
  EXPECT_FLOAT_EQ(detail.level[DETAIL_LEVEL_NEAR].radius, 10.0f);
  EXPECT_FLOAT_EQ(detail.level[DETAIL_LEVEL_MEDIUM].radius, 17.0f);
  EXPECT_FLOAT_EQ(detail.level[DETAIL_LEVEL_FAR].radius, 27.0f);

  auto* volatile_info = module.getPVolatileInfo();
  ASSERT_NE(volatile_info, nullptr);
  EXPECT_FLOAT_EQ(volatile_info->position(), 1.5f);
  EXPECT_FLOAT_EQ(volatile_info->yaw(), KBEngine::VolatileInfo::ALWAYS);
  EXPECT_FLOAT_EQ(volatile_info->pitch(), 2.5f);
  EXPECT_FLOAT_EQ(volatile_info->roll(), KBEngine::VolatileInfo::ALWAYS);
  EXPECT_FALSE(volatile_info->optimized());
}

TEST_F(EntityDefXmlLoadingTest, RejectsDuplicatePropertyAndMethodUTypes)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "Conflict.def",
      R"(<root>
  <Properties>
    <hp>
      <Flags>BASE</Flags>
      <Type>UINT32</Type>
      <Utype>42</Utype>
    </hp>
    <mp>
      <Flags>BASE</Flags>
      <Type>UINT32</Type>
      <Utype>42</Utype>
    </mp>
  </Properties>
  <BaseMethods>
    <Foo>
      <Utype>99</Utype>
    </Foo>
    <Bar>
      <Utype>99</Utype>
    </Bar>
  </BaseMethods>
</root>)");

  KBEngine::XML xml((env.defs_dir / "Conflict.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("Conflict", 2);
  EXPECT_FALSE(KBEngine::EntityDef::loadAllDefDescriptions("Conflict", &xml, root, &module));
}

TEST_F(EntityDefXmlLoadingTest, LoadsClientMethodArgsFromDefXml)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  write_file(
      env.defs_dir / "Ghost.def",
      R"(<root>
  <ClientMethods>
    <Echo>
      <Arg>UINT8</Arg>
      <Utype>88</Utype>
    </Echo>
  </ClientMethods>
</root>)");

  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));
  KBEngine::XML xml((env.defs_dir / "Ghost.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("Ghost", 3);
  ASSERT_TRUE(KBEngine::EntityDef::loadDefClientMethods(
      "Ghost",
      &xml,
      xml.enterNode(root, "ClientMethods"),
      &module));

  auto* method = module.findClientMethodDescription("Echo");
  ASSERT_NE(method, nullptr);
  EXPECT_EQ(method->getArgSize(), 1);
}

TEST_F(EntityDefXmlLoadingTest, ScriptModuleLoadingAcceptsEmptyEntitiesRoot)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.scripts_dir / "entities.xml", "<root/>\n");

  std::vector<PyTypeObject*> script_base_types;
  EXPECT_TRUE(KBEngine::EntityDef::loadAllComponentScriptModules(
      env.scripts_dir.string() + "/",
      script_base_types));
  EXPECT_TRUE(KBEngine::EntityDef::loadAllEntityScriptModules(
      env.scripts_dir.string() + "/",
      script_base_types));
}

TEST_F(EntityDefXmlLoadingTest, BaseDataTypeInitializeReturnsTrue)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "InitProbe.def", "<root><Value><Type>UINT8</Type></Value></root>\n");

  KBEngine::XML xml((env.defs_dir / "InitProbe.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::IntType<uint8> data_type;
  EXPECT_TRUE(data_type.initialize(&xml, root));
}

TEST_F(EntityDefXmlLoadingTest, ScriptDefModuleBuildsAliasMapsForPropertiesMethodsAndComponents)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::EntityDef::entitydefAliasID(true);

  KBEngine::ScriptDefModule component("Inventory", 11);
  KBEngine::ScriptDefModule module("Avatar", 10);

  auto* client_prop = KBEngine::PropertyDescription::createDescription(
      1,
      "UINT32",
      "hp",
      KBEngine::ED_FLAG_BASE_AND_CLIENT,
      true,
      KBEngine::DataTypes::getDataType("UINT32"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_NEAR);
  ASSERT_NE(client_prop, nullptr);
  ASSERT_TRUE(module.addPropertyDescription("hp", client_prop, KBEngine::CLIENT_TYPE));

  auto* cell_method = new KBEngine::MethodDescription(
      2,
      KBEngine::CELLAPP_TYPE,
      "Pulse",
      KBEngine::MethodDescription::EXPOSED);
  ASSERT_TRUE(module.addCellMethodDescription("Pulse", cell_method));

  auto* client_method = new KBEngine::MethodDescription(
      3,
      KBEngine::CLIENT_TYPE,
      "Sync",
      KBEngine::MethodDescription::NO_EXPOSED);
  ASSERT_TRUE(module.addClientMethodDescription("Sync", client_method));

  ASSERT_TRUE(module.addComponentDescription("inventory", &component));

  module.onLoaded();

  EXPECT_TRUE(module.usePropertyDescrAlias());
  EXPECT_TRUE(module.useMethodDescrAlias());
  ASSERT_GE(client_prop->aliasID(), 0);
  EXPECT_EQ(module.findAliasPropertyDescription(client_prop->aliasID()), client_prop);
  EXPECT_EQ(module.findAliasMethodDescription(1), client_method);
  EXPECT_EQ(module.findComponentDescription(static_cast<KBEngine::ENTITY_COMPONENT_ALIASID>(0)), &component);
  EXPECT_EQ(module.findComponentDescription("inventory"), &component);
  EXPECT_EQ(module.findComponentDescription(component.getUType()), &component);

  KBEngine::EntityDef::entitydefAliasID(false);
}

TEST_F(EntityDefXmlLoadingTest, ScriptDefModuleRejectsPropertyConflictsAndTracksPersistentLookups)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::ScriptDefModule component("Inventory", 21);
  KBEngine::ScriptDefModule module("Avatar", 20);
  ASSERT_TRUE(module.addComponentDescription("inventory", &component));

  auto* method = new KBEngine::MethodDescription(
      7,
      KBEngine::BASEAPP_TYPE,
      "cast",
      KBEngine::MethodDescription::NO_EXPOSED);
  ASSERT_TRUE(module.addBaseMethodDescription("cast", method));

  auto* cast_prop = KBEngine::PropertyDescription::createDescription(
      8,
      "UINT16",
      "cast",
      KBEngine::ED_FLAG_BASE,
      true,
      KBEngine::DataTypes::getDataType("UINT16"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_FAR);
  ASSERT_NE(cast_prop, nullptr);
  EXPECT_FALSE(module.addPropertyDescription("cast", cast_prop, KBEngine::BASEAPP_TYPE));

  auto* inventory_prop = KBEngine::PropertyDescription::createDescription(
      9,
      "UINT16",
      "inventory",
      KBEngine::ED_FLAG_BASE,
      true,
      KBEngine::DataTypes::getDataType("UINT16"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_FAR);
  ASSERT_NE(inventory_prop, nullptr);
  EXPECT_FALSE(module.addPropertyDescription("inventory", inventory_prop, KBEngine::BASEAPP_TYPE));

  auto* hp_prop = KBEngine::PropertyDescription::createDescription(
      10,
      "UINT32",
      "hp",
      KBEngine::ED_FLAG_BASE,
      true,
      KBEngine::DataTypes::getDataType("UINT32"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_FAR);
  ASSERT_NE(hp_prop, nullptr);
  ASSERT_TRUE(module.addPropertyDescription("hp", hp_prop, KBEngine::BASEAPP_TYPE));
  EXPECT_EQ(module.findBasePropertyDescription("hp"), hp_prop);
  EXPECT_EQ(module.findBasePropertyDescription(static_cast<KBEngine::ENTITY_PROPERTY_UID>(10)), hp_prop);
  EXPECT_EQ(module.findPersistentPropertyDescription("hp"), hp_prop);
  EXPECT_EQ(module.findPersistentPropertyDescription(static_cast<KBEngine::ENTITY_PROPERTY_UID>(10)), hp_prop);
  EXPECT_EQ(module.findPropertyDescription("hp", KBEngine::BASEAPP_TYPE), hp_prop);
  EXPECT_EQ(module.findPropertyDescription(static_cast<KBEngine::ENTITY_PROPERTY_UID>(10), KBEngine::BASEAPP_TYPE), hp_prop);
}

TEST_F(EntityDefXmlLoadingTest, ScriptDefModuleAutoMatchesComponentAndEntityOwnership)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.scripts_dir / "entities.xml",
      "<root><Hero hasClient=\"false\" hasBase=\"true\" hasCell=\"false\"/></root>\n");
  write_file(env.scripts_dir / "client" / "Hero.py", "# client entity\n");
  write_file(env.scripts_dir / "base" / "Hero.py", "# base entity\n");
  write_file(env.scripts_dir / "cell" / "Hero.py", "# cell entity\n");

  KBEngine::ScriptDefModule component("Inventory", 30);
  component.isComponentModule(true);
  auto* exposed_method = new KBEngine::MethodDescription(
      31,
      KBEngine::BASEAPP_TYPE,
      "Notify",
      KBEngine::MethodDescription::EXPOSED);
  ASSERT_TRUE(component.addBaseMethodDescription("Notify", exposed_method));
  component.autoMatchCompOwn();
  EXPECT_TRUE(component.hasBase());
  EXPECT_TRUE(component.hasCell());
  EXPECT_TRUE(component.hasClient());

  KBEngine::ScriptDefModule entity("Hero", 32);
  entity.autoMatchCompOwn();
  EXPECT_FALSE(entity.hasClient());
  EXPECT_TRUE(entity.hasBase());
  EXPECT_FALSE(entity.hasCell());
}

TEST_F(EntityDefXmlLoadingTest, ScriptDefModuleDisablesAliasesWhenCountsOverflow)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::EntityDef::entitydefAliasID(true);
  KBEngine::ScriptDefModule module("Overflow", 40);

  for (int i = 0; i < 252; ++i)
  {
    const std::string name = "prop" + std::to_string(i);
    auto* prop = KBEngine::PropertyDescription::createDescription(
        static_cast<KBEngine::ENTITY_PROPERTY_UID>(1000 + i),
        "UINT8",
        name,
        KBEngine::ED_FLAG_OWN_CLIENT,
        true,
        KBEngine::DataTypes::getDataType("UINT8"),
        false,
        "",
        0,
        "",
        DETAIL_LEVEL_FAR);
    ASSERT_NE(prop, nullptr);
    ASSERT_TRUE(module.addPropertyDescription(name.c_str(), prop, KBEngine::CLIENT_TYPE));
  }

  for (int i = 0; i < 255; ++i)
  {
    const std::string name = "method" + std::to_string(i);
    auto* method = new KBEngine::MethodDescription(
        static_cast<KBEngine::ENTITY_METHOD_UID>(2000 + i),
        KBEngine::CLIENT_TYPE,
        name,
        KBEngine::MethodDescription::NO_EXPOSED);
    ASSERT_TRUE(module.addClientMethodDescription(name.c_str(), method));
  }

  module.onLoaded();

  EXPECT_FALSE(module.usePropertyDescrAlias());
  EXPECT_FALSE(module.useMethodDescrAlias());
  EXPECT_EQ(module.findAliasPropertyDescription(4), nullptr);
  EXPECT_EQ(module.findAliasMethodDescription(1), nullptr);

  KBEngine::EntityDef::entitydefAliasID(false);
}

TEST_F(EntityDefXmlLoadingTest, ScriptDefModuleDebugDumpAndSmartUTypePaths)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::ScriptDefModule module("DebugAvatar", 50);

  auto* cell_prop = KBEngine::PropertyDescription::createDescription(
      51,
      "UINT8",
      "cp",
      KBEngine::ED_FLAG_ALL_CLIENTS,
      true,
      KBEngine::DataTypes::getDataType("UINT8"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_NEAR);
  auto* base_prop = KBEngine::PropertyDescription::createDescription(
      52,
      "UINT16",
      "bp",
      KBEngine::ED_FLAG_BASE,
      false,
      KBEngine::DataTypes::getDataType("UINT16"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_FAR);
  auto* client_prop = KBEngine::PropertyDescription::createDescription(
      53,
      "UINT32",
      "xp",
      KBEngine::ED_FLAG_OWN_CLIENT,
      false,
      KBEngine::DataTypes::getDataType("UINT32"),
      false,
      "",
      0,
      "",
      DETAIL_LEVEL_FAR);
  ASSERT_TRUE(module.addPropertyDescription("cp", cell_prop, KBEngine::CELLAPP_TYPE));
  ASSERT_TRUE(module.addPropertyDescription("bp", base_prop, KBEngine::BASEAPP_TYPE));
  ASSERT_TRUE(module.addPropertyDescription("xp", client_prop, KBEngine::CLIENT_TYPE));

  auto* cell_method = new KBEngine::MethodDescription(
      54, KBEngine::CELLAPP_TYPE, "cm", KBEngine::MethodDescription::EXPOSED);
  auto* base_method = new KBEngine::MethodDescription(
      55, KBEngine::BASEAPP_TYPE, "bm", KBEngine::MethodDescription::EXPOSED);
  auto* client_method = new KBEngine::MethodDescription(
      56, KBEngine::CLIENT_TYPE, "xm", KBEngine::MethodDescription::NO_EXPOSED);
  ASSERT_TRUE(module.addCellMethodDescription("cm", cell_method));
  ASSERT_TRUE(module.addBaseMethodDescription("bm", base_method));
  ASSERT_TRUE(module.addClientMethodDescription("xm", client_method));

  KBEngine::g_debugEntity = true;
  module.c_str();
  module.setUType(57);

  KBEngine::MemoryStream stream;
  KBEngine::EntityDef::entitydefAliasID(false);
  module.addSmartUTypeToStream(&stream);
  EXPECT_GT(stream.length(), 0u);

  KBEngine::g_componentType = KBEngine::CELLAPP_TYPE;
  EXPECT_EQ(module.getPropertyDescrs().size(), 1u);
  KBEngine::g_componentType = KBEngine::BASEAPP_TYPE;
  EXPECT_EQ(module.getPropertyDescrs().size(), 1u);
  KBEngine::g_componentType = KBEngine::CLIENT_TYPE;
  EXPECT_EQ(module.getPropertyDescrs().size(), 1u);
  KBEngine::g_debugEntity = false;
}

TEST_F(EntityDefXmlLoadingTest, DataTypesManageAliasesAndDeletionPaths)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  EXPECT_TRUE(KBEngine::DataTypes::validTypeName("VectorAlias"));
  EXPECT_FALSE(KBEngine::DataTypes::validTypeName("_Hidden"));

  const auto base_types = KBEngine::DataTypes::getBaseTypeNames();
  EXPECT_FALSE(base_types.empty());
  EXPECT_NE(std::find(base_types.begin(), base_types.end(), "UINT8"), base_types.end());

  auto* custom = new KBEngine::IntType<uint8>();
  const auto custom_uid = custom->id();
  ASSERT_TRUE(KBEngine::DataTypes::addDataType("CustomUint8", custom));
  EXPECT_EQ(KBEngine::DataTypes::getDataType("CustomUint8"), custom);
  EXPECT_EQ(KBEngine::DataTypes::getDataType("CustomUint8", false), custom);
  EXPECT_EQ(KBEngine::DataTypes::getDataType(custom_uid), custom);

  EXPECT_FALSE(KBEngine::DataTypes::addDataType("customuint8", new KBEngine::IntType<uint8>()));
  EXPECT_FALSE(KBEngine::DataTypes::addDataType(custom_uid, custom));

  KBEngine::DataTypes::delDataType("CustomUint8");
  EXPECT_EQ(KBEngine::DataTypes::getDataType("CustomUint8", false), nullptr);
  KBEngine::DataTypes::delDataType("MissingType");
}

TEST_F(EntityDefXmlLoadingTest, DataTypesRejectInvalidTypeDefinitions)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::SmartPointer<KBEngine::XML> null_xml;
  EXPECT_FALSE(KBEngine::DataTypes::loadTypes(null_xml));

  write_file(
      env.defs_dir / "invalid_alias.xml",
      R"(<root><_BadAlias>UINT8</_BadAlias></root>)");
  KBEngine::SmartPointer<KBEngine::XML> invalid_alias_xml(
      new KBEngine::XML((env.defs_dir / "invalid_alias.xml").string().c_str()));
  ASSERT_TRUE(invalid_alias_xml->isGood());
  EXPECT_FALSE(KBEngine::DataTypes::loadTypes(invalid_alias_xml));

  write_file(
      env.defs_dir / "unknown_alias.xml",
      R"(<root><KnownMissing>NOT_A_REAL_TYPE</KnownMissing></root>)");
  KBEngine::SmartPointer<KBEngine::XML> unknown_alias_xml(
      new KBEngine::XML((env.defs_dir / "unknown_alias.xml").string().c_str()));
  ASSERT_TRUE(unknown_alias_xml->isGood());
  EXPECT_FALSE(KBEngine::DataTypes::loadTypes(unknown_alias_xml));

  write_file(
      env.defs_dir / "bad_array.xml",
      R"(<root><BrokenArray>ARRAY</BrokenArray></root>)");
  KBEngine::SmartPointer<KBEngine::XML> bad_array_xml(
      new KBEngine::XML((env.defs_dir / "bad_array.xml").string().c_str()));
  ASSERT_TRUE(bad_array_xml->isGood());
  EXPECT_FALSE(KBEngine::DataTypes::loadTypes(bad_array_xml));

  write_file(
      env.defs_dir / "bad_dict.xml",
      R"(<root><BrokenDict>FIXED_DICT</BrokenDict></root>)");
  KBEngine::SmartPointer<KBEngine::XML> bad_dict_xml(
      new KBEngine::XML((env.defs_dir / "bad_dict.xml").string().c_str()));
  ASSERT_TRUE(bad_dict_xml->isGood());
  EXPECT_FALSE(KBEngine::DataTypes::loadTypes(bad_dict_xml));
}

TEST_F(EntityDefXmlLoadingTest, DetailLevelInfoRejectsIncompleteLevels)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.defs_dir / "BrokenDetail.def",
      R"(<root>
  <DetailLevels>
    <NEAR>
      <radius>10</radius>
      <hyst>1</hyst>
    </NEAR>
    <MEDIUM>
      <radius>5</radius>
    </MEDIUM>
    <FAR>
      <radius>7</radius>
      <hyst>2</hyst>
    </FAR>
  </DetailLevels>
</root>)");

  KBEngine::XML xml((env.defs_dir / "BrokenDetail.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("BrokenDetail", 60);
  EXPECT_FALSE(KBEngine::EntityDef::loadDetailLevelInfo(
      env.defs_dir.string() + "/",
      "BrokenDetail",
      &xml,
      root,
      &module));
}

TEST_F(EntityDefXmlLoadingTest, VolatileInfoAppliesAlwaysAndMissingDefaults)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.defs_dir / "VolatileDefaults.def",
      R"(<root>
  <Volatile>
    <position/>
    <yaw>3.5</yaw>
    <optimized/>
  </Volatile>
</root>)");

  KBEngine::XML xml((env.defs_dir / "VolatileDefaults.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("VolatileDefaults", 61);
  ASSERT_TRUE(KBEngine::EntityDef::loadVolatileInfo(
      env.defs_dir.string() + "/",
      "VolatileDefaults",
      &xml,
      root,
      &module));

  auto* volatile_info = module.getPVolatileInfo();
  ASSERT_NE(volatile_info, nullptr);
  EXPECT_FLOAT_EQ(volatile_info->position(), KBEngine::VolatileInfo::ALWAYS);
  EXPECT_FLOAT_EQ(volatile_info->yaw(), 3.5f);
  EXPECT_FLOAT_EQ(volatile_info->pitch(), -1.0f);
  EXPECT_FLOAT_EQ(volatile_info->roll(), -1.0f);
  EXPECT_TRUE(volatile_info->optimized());
}

TEST_F(EntityDefXmlLoadingTest, InterfacesLoadLowercaseAndUppercaseTagVariants)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "interfaces" / "IfaceLower.def",
      R"(<root>
  <Properties>
    <lowerProp>
      <Flags>BASE</Flags>
      <Type>UINT8</Type>
    </lowerProp>
  </Properties>
</root>)");

  write_file(
      env.defs_dir / "interfaces" / "IfaceUpper.def",
      R"(<root>
  <BaseMethods>
    <UpperCall>
      <Utype>401</Utype>
    </UpperCall>
  </BaseMethods>
</root>)");

  write_file(
      env.defs_dir / "VariantIface.def",
      R"(<root>
  <Interfaces>
    <interface>
      <IfaceLower/>
    </interface>
    <Type>
      <IfaceUpper/>
    </Type>
    <Ignored>
      <IfaceMissing/>
    </Ignored>
  </Interfaces>
</root>)");

  KBEngine::XML xml((env.defs_dir / "VariantIface.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("VariantIface", 62);
  ASSERT_TRUE(KBEngine::EntityDef::loadInterfaces(
      env.defs_dir.string() + "/",
      "VariantIface",
      &xml,
      root,
      &module));

  EXPECT_NE(module.findBasePropertyDescription("lowerProp"), nullptr);
  EXPECT_NE(module.findBaseMethodDescription("UpperCall"), nullptr);
}

TEST_F(EntityDefXmlLoadingTest, ComponentsRejectLimitedNamesAndMissingTypes)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "BadComponentName.def",
      R"(<root>
  <Components>
    <position>
      <Type>
        <Inventory/>
      </Type>
    </position>
  </Components>
</root>)");

  KBEngine::XML invalid_name_xml((env.defs_dir / "BadComponentName.def").string().c_str());
  ASSERT_TRUE(invalid_name_xml.isGood());
  auto* invalid_name_root = invalid_name_xml.getRootNode();
  ASSERT_NE(invalid_name_root, nullptr);

  KBEngine::ScriptDefModule invalid_name_module("BadComponentName", 63);
  EXPECT_FALSE(KBEngine::EntityDef::loadComponents(
      env.defs_dir.string() + "/",
      "BadComponentName",
      &invalid_name_xml,
      invalid_name_root,
      &invalid_name_module));

  write_file(
      env.defs_dir / "MissingComponentType.def",
      R"(<root>
  <Components>
    <inventory>
      <Persistent>true</Persistent>
    </inventory>
  </Components>
</root>)");

  KBEngine::XML missing_type_xml((env.defs_dir / "MissingComponentType.def").string().c_str());
  ASSERT_TRUE(missing_type_xml.isGood());
  auto* missing_type_root = missing_type_xml.getRootNode();
  ASSERT_NE(missing_type_root, nullptr);

  KBEngine::ScriptDefModule missing_type_module("MissingComponentType", 64);
  EXPECT_FALSE(KBEngine::EntityDef::loadComponents(
      env.defs_dir.string() + "/",
      "MissingComponentType",
      &missing_type_xml,
      missing_type_root,
      &missing_type_module));
}

TEST_F(EntityDefXmlLoadingTest, ComponentsLoadParentAndInterfaceDefinitions)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(env.scripts_dir / "base" / "components" / "Equipment.py", "# equipment base\n");
  write_file(env.scripts_dir / "cell" / "components" / "Equipment.py", "# equipment cell\n");

  write_file(
      env.defs_dir / "interfaces" / "EquipmentIface.def",
      R"(<root>
  <CellMethods>
    <SyncEquipment>
      <Utype>430</Utype>
    </SyncEquipment>
  </CellMethods>
</root>)");

  write_file(
      env.defs_dir / "components" / "EquipmentBase.def",
      R"(<root>
  <Properties>
    <baseWeight>
      <Flags>BASE</Flags>
      <Type>UINT16</Type>
    </baseWeight>
  </Properties>
</root>)");

  write_file(
      env.defs_dir / "components" / "Equipment.def",
      R"(<root>
  <Properties>
    <durability>
      <Flags>BASE_AND_CLIENT</Flags>
      <Type>UINT32</Type>
    </durability>
  </Properties>
  <Interfaces>
    <Type>
      <EquipmentIface/>
    </Type>
  </Interfaces>
  <Parent>
    <EquipmentBase/>
  </Parent>
  <DetailLevels>
    <NEAR>
      <radius>4</radius>
      <hyst>1</hyst>
    </NEAR>
    <MEDIUM>
      <radius>6</radius>
      <hyst>2</hyst>
    </MEDIUM>
    <FAR>
      <radius>8</radius>
      <hyst>3</hyst>
    </FAR>
  </DetailLevels>
</root>)");

  write_file(
      env.defs_dir / "ComponentOwner.def",
      R"(<root>
  <Components>
    <equipment>
      <Type>
        <Equipment/>
      </Type>
      <Utype>170</Utype>
    </equipment>
  </Components>
</root>)");

  KBEngine::XML xml((env.defs_dir / "ComponentOwner.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule owner("ComponentOwner", 65);
  ASSERT_TRUE(KBEngine::EntityDef::loadComponents(
      env.defs_dir.string() + "/",
      "ComponentOwner",
      &xml,
      root,
      &owner));

  auto* component_module = KBEngine::EntityDef::findScriptModule("Equipment", false);
  ASSERT_NE(component_module, nullptr);
  EXPECT_NE(owner.findComponentDescription("equipment"), nullptr);
  EXPECT_NE(owner.findBasePropertyDescription("equipment"), nullptr);
  EXPECT_NE(owner.findClientPropertyDescription("equipment"), nullptr);
  EXPECT_NE(component_module->findBasePropertyDescription("baseWeight"), nullptr);
  EXPECT_NE(component_module->findBasePropertyDescription("durability"), nullptr);
  EXPECT_NE(component_module->findClientPropertyDescription("durability"), nullptr);
  EXPECT_NE(component_module->findCellMethodDescription("SyncEquipment"), nullptr);

  const auto& detail = component_module->getDetailLevel();
  EXPECT_FLOAT_EQ(detail.level[DETAIL_LEVEL_NEAR].radius, 4.0f);
  EXPECT_FLOAT_EQ(detail.level[DETAIL_LEVEL_MEDIUM].radius, 11.0f);
  EXPECT_FLOAT_EQ(detail.level[DETAIL_LEVEL_FAR].radius, 21.0f);
}

TEST_F(EntityDefXmlLoadingTest, ParentClassFailsWhenDefinitionIsMissing)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.defs_dir / "MissingParent.def",
      R"(<root>
  <Parent>
    <NoSuchParent/>
  </Parent>
</root>)");

  KBEngine::XML xml((env.defs_dir / "MissingParent.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("MissingParent", 66);
  EXPECT_FALSE(KBEngine::EntityDef::loadParentClass(
      env.defs_dir.string() + "/",
      "MissingParent",
      &xml,
      root,
      &module));
}

TEST_F(EntityDefXmlLoadingTest, LoadDefPropertiesParsesFlagsDefaultsAndArrayTypes)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "PropertyMatrix.def",
      R"(<root>
  <Properties>
    <hitPoints>
      <Flags>BASE_AND_CLIENT</Flags>
      <Persistent>true</Persistent>
      <Type>UINT32</Type>
      <Identifier>true</Identifier>
      <Index>unique</Index>
      <DatabaseLength>8</DatabaseLength>
      <Default>77</Default>
      <DetailLevel>NEAR</DetailLevel>
      <Utype>501</Utype>
    </hitPoints>
    <vision>
      <Flags>CELL</Flags>
      <Type>ARRAY<of>UINT8</of></Type>
      <DetailLevel>UNKNOWN_LEVEL</DetailLevel>
    </vision>
  </Properties>
</root>)");

  KBEngine::XML xml((env.defs_dir / "PropertyMatrix.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("PropertyMatrix", 67);
  ASSERT_TRUE(KBEngine::EntityDef::loadDefPropertys(
      "PropertyMatrix",
      &xml,
      xml.enterNode(root, "Properties"),
      &module));

  auto* hit_points = module.findBasePropertyDescription("hitPoints");
  ASSERT_NE(hit_points, nullptr);
  EXPECT_NE(module.findClientPropertyDescription("hitPoints"), nullptr);
  EXPECT_TRUE(module.hasBase());
  EXPECT_TRUE(module.hasCell());
  EXPECT_TRUE(module.hasClient());
  EXPECT_EQ(hit_points->getUType(), 501);
  EXPECT_TRUE(hit_points->hasClient());
  EXPECT_TRUE(hit_points->isPersistent());
  EXPECT_EQ(hit_points->getDatabaseLength(), 8u);
  EXPECT_STREQ(hit_points->indexType(), "UNIQUE");
  EXPECT_EQ(hit_points->getDetailLevel(), DETAIL_LEVEL_NEAR);

  auto* vision = module.findCellPropertyDescription("vision");
  ASSERT_NE(vision, nullptr);
  EXPECT_EQ(vision->getDetailLevel(), DETAIL_LEVEL_FAR);
}

TEST_F(EntityDefXmlLoadingTest, LoadDefPropertiesRejectsMissingFlagsUnknownFlagsAndType)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "MissingFlags.def",
      R"(<root>
  <Properties>
    <broken>
      <Type>UINT32</Type>
    </broken>
  </Properties>
</root>)");

  KBEngine::XML missing_flags_xml((env.defs_dir / "MissingFlags.def").string().c_str());
  ASSERT_TRUE(missing_flags_xml.isGood());
  KBEngine::ScriptDefModule missing_flags_module("MissingFlags", 68);
  EXPECT_FALSE(KBEngine::EntityDef::loadDefPropertys(
      "MissingFlags",
      &missing_flags_xml,
      missing_flags_xml.enterNode(missing_flags_xml.getRootNode(), "Properties"),
      &missing_flags_module));

  write_file(
      env.defs_dir / "UnknownFlags.def",
      R"(<root>
  <Properties>
    <broken>
      <Flags>CLIENT</Flags>
      <Type>UINT32</Type>
    </broken>
  </Properties>
</root>)");

  KBEngine::XML unknown_flags_xml((env.defs_dir / "UnknownFlags.def").string().c_str());
  ASSERT_TRUE(unknown_flags_xml.isGood());
  KBEngine::ScriptDefModule unknown_flags_module("UnknownFlags", 69);
  EXPECT_FALSE(KBEngine::EntityDef::loadDefPropertys(
      "UnknownFlags",
      &unknown_flags_xml,
      unknown_flags_xml.enterNode(unknown_flags_xml.getRootNode(), "Properties"),
      &unknown_flags_module));

  write_file(
      env.defs_dir / "MissingType.def",
      R"(<root>
  <Properties>
    <broken>
      <Flags>BASE</Flags>
    </broken>
  </Properties>
</root>)");

  KBEngine::XML missing_type_xml((env.defs_dir / "MissingType.def").string().c_str());
  ASSERT_TRUE(missing_type_xml.isGood());
  KBEngine::ScriptDefModule missing_type_module("MissingType", 70);
  EXPECT_FALSE(KBEngine::EntityDef::loadDefPropertys(
      "MissingType",
      &missing_type_xml,
      missing_type_xml.enterNode(missing_type_xml.getRootNode(), "Properties"),
      &missing_type_module));
}

TEST_F(EntityDefXmlLoadingTest, CreateMethodDescriptionAllocatesAutoUtypesAndRejectsConflicts)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  KBEngine::ScriptDefModule module("MethodFactory", 71);

  auto* auto_method = KBEngine::EntityDef::createMethodDescription(
      &module,
      0,
      KBEngine::BASEAPP_TYPE,
      "AutoGenerated",
      KBEngine::MethodDescription::NO_EXPOSED);
  ASSERT_NE(auto_method, nullptr);
  EXPECT_GT(auto_method->getUType(), 0);
  ASSERT_TRUE(module.addBaseMethodDescription("AutoGenerated", auto_method));

  auto* existing = new KBEngine::MethodDescription(
      650,
      KBEngine::CELLAPP_TYPE,
      "Existing",
      KBEngine::MethodDescription::EXPOSED);
  ASSERT_TRUE(module.addCellMethodDescription("Existing", existing));

  auto* conflict = KBEngine::EntityDef::createMethodDescription(
      &module,
      650,
      KBEngine::BASEAPP_TYPE,
      "Conflict",
      KBEngine::MethodDescription::EXPOSED);
  EXPECT_EQ(conflict, nullptr);
}

TEST_F(EntityDefXmlLoadingTest, LoadDefCellAndBaseMethodsParseArraysExposeAndAutoUtypes)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "MethodMatrix.def",
      R"(<root>
  <CellMethods>
    <Pulse>
      <Exposed/>
      <Arg>ARRAY<of>UINT8</of></Arg>
    </Pulse>
  </CellMethods>
  <BaseMethods>
    <Spawn>
      <Arg>UINT16</Arg>
    </Spawn>
  </BaseMethods>
</root>)");

  KBEngine::XML xml((env.defs_dir / "MethodMatrix.def").string().c_str());
  ASSERT_TRUE(xml.isGood());
  auto* root = xml.getRootNode();
  ASSERT_NE(root, nullptr);

  KBEngine::ScriptDefModule module("MethodMatrix", 72);
  ASSERT_TRUE(KBEngine::EntityDef::loadDefCellMethods(
      "MethodMatrix",
      &xml,
      xml.enterNode(root, "CellMethods"),
      &module));
  ASSERT_TRUE(KBEngine::EntityDef::loadDefBaseMethods(
      "MethodMatrix",
      &xml,
      xml.enterNode(root, "BaseMethods"),
      &module));

  auto* pulse = module.findCellMethodDescription("Pulse");
  ASSERT_NE(pulse, nullptr);
  EXPECT_EQ(pulse->getArgSize(), 1);
  EXPECT_TRUE(pulse->isExposed());
  EXPECT_GT(pulse->getUType(), 0);

  auto* spawn = module.findBaseMethodDescription("Spawn");
  ASSERT_NE(spawn, nullptr);
  EXPECT_EQ(spawn->getArgSize(), 1);
  EXPECT_GT(spawn->getUType(), 0);
}

TEST_F(EntityDefXmlLoadingTest, LoadDefMethodsRejectInvalidTypesAndDuplicateUtypes)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  write_file(
      env.defs_dir / "BadCellMethod.def",
      R"(<root>
  <CellMethods>
    <Broken>
      <Arg>NOT_A_REAL_TYPE</Arg>
    </Broken>
  </CellMethods>
</root>)");

  KBEngine::XML bad_cell_xml((env.defs_dir / "BadCellMethod.def").string().c_str());
  ASSERT_TRUE(bad_cell_xml.isGood());
  KBEngine::ScriptDefModule bad_cell_module("BadCellMethod", 73);
  EXPECT_FALSE(KBEngine::EntityDef::loadDefCellMethods(
      "BadCellMethod",
      &bad_cell_xml,
      bad_cell_xml.enterNode(bad_cell_xml.getRootNode(), "CellMethods"),
      &bad_cell_module));

  write_file(
      env.defs_dir / "BadBaseMethod.def",
      R"(<root>
  <BaseMethods>
    <Broken>
      <Arg>NOT_A_REAL_TYPE</Arg>
    </Broken>
  </BaseMethods>
</root>)");

  KBEngine::XML bad_base_xml((env.defs_dir / "BadBaseMethod.def").string().c_str());
  ASSERT_TRUE(bad_base_xml.isGood());
  KBEngine::ScriptDefModule bad_base_module("BadBaseMethod", 74);
  EXPECT_FALSE(KBEngine::EntityDef::loadDefBaseMethods(
      "BadBaseMethod",
      &bad_base_xml,
      bad_base_xml.enterNode(bad_base_xml.getRootNode(), "BaseMethods"),
      &bad_base_module));

  auto* existing_client = new KBEngine::MethodDescription(
      777,
      KBEngine::CLIENT_TYPE,
      "ExistingClient",
      KBEngine::MethodDescription::NO_EXPOSED);
  KBEngine::ScriptDefModule duplicate_client_module("DuplicateClient", 75);
  ASSERT_TRUE(duplicate_client_module.addClientMethodDescription("ExistingClient", existing_client));

  write_file(
      env.defs_dir / "DuplicateClientMethod.def",
      R"(<root>
  <ClientMethods>
    <Echo>
      <Utype>777</Utype>
    </Echo>
  </ClientMethods>
</root>)");

  KBEngine::XML duplicate_client_xml((env.defs_dir / "DuplicateClientMethod.def").string().c_str());
  ASSERT_TRUE(duplicate_client_xml.isGood());
  EXPECT_FALSE(KBEngine::EntityDef::loadDefClientMethods(
      "DuplicateClient",
      &duplicate_client_xml,
      duplicate_client_xml.enterNode(duplicate_client_xml.getRootNode(), "ClientMethods"),
      &duplicate_client_module));
}

TEST_F(EntityDefXmlLoadingTest, ScriptModuleRegistrationLookupAndOldModuleLookupWork)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  auto* avatar = KBEngine::EntityDef::registerNewScriptDefModule("Avatar");
  ASSERT_NE(avatar, nullptr);
  EXPECT_EQ(avatar, KBEngine::EntityDef::registerNewScriptDefModule("Avatar"));
  EXPECT_EQ(KBEngine::EntityDef::findScriptModule("Avatar", false), avatar);
  EXPECT_EQ(KBEngine::EntityDef::findScriptModule(avatar->getUType(), false), avatar);
  EXPECT_EQ(KBEngine::EntityDef::findScriptModule("MissingModule", false), nullptr);
  EXPECT_EQ(KBEngine::EntityDef::findScriptModule(static_cast<KBEngine::ENTITY_SCRIPT_UID>(999), false), nullptr);

  auto* legacy = new KBEngine::ScriptDefModule("LegacyAvatar", 77);
  KBEngine::EntityDef::__oldScriptModules.push_back(legacy);
  KBEngine::EntityDef::__oldScriptTypeMappingUType["LegacyAvatar"] = 1;

  EXPECT_EQ(KBEngine::EntityDef::findOldScriptModule("LegacyAvatar", false), legacy);
  EXPECT_EQ(KBEngine::EntityDef::findOldScriptModule("MissingLegacy", false), nullptr);

  KBEngine::EntityDef::__oldScriptTypeMappingUType["BrokenLegacy"] = 10;
  EXPECT_EQ(KBEngine::EntityDef::findOldScriptModule("BrokenLegacy", false), nullptr);
}

TEST_F(EntityDefXmlLoadingTest, ScriptModuleLoadChecksAndSubclassChecksFollowComponentType)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.scripts_dir / "user_module.py", "class user_module:\n    pass\n");
  prepend_python_path(env.scripts_dir.string());

  auto* user_module = KBEngine::EntityDef::loadScriptModule("user_module");
  ASSERT_NE(user_module, nullptr);
  Py_DECREF(user_module);

  EXPECT_EQ(KBEngine::EntityDef::loadScriptModule("json"), nullptr);
  PyErr_Clear();

  auto* py_main = PyImport_AddModule("__main__");
  ASSERT_NE(py_main, nullptr);
  ASSERT_EQ(
      PyRun_SimpleString(
          "class EntityBase:\n"
          "    pass\n"
          "class DerivedEntity(EntityBase):\n"
          "    pass\n"
          "class ForeignEntity:\n"
          "    pass\n"),
      0);

  auto* entity_base = reinterpret_cast<PyTypeObject*>(PyObject_GetAttrString(py_main, "EntityBase"));
  auto* derived = PyObject_GetAttrString(py_main, "DerivedEntity");
  auto* foreign = PyObject_GetAttrString(py_main, "ForeignEntity");
  ASSERT_NE(entity_base, nullptr);
  ASSERT_NE(derived, nullptr);
  ASSERT_NE(foreign, nullptr);

  KBEngine::EntityDef::__scriptBaseTypes.clear();
  KBEngine::EntityDef::__scriptBaseTypes.push_back(entity_base);

  EXPECT_EQ(KBEngine::EntityDef::isSubClass(derived), "");
  EXPECT_FALSE(KBEngine::EntityDef::isSubClass(foreign).empty());

  Py_DECREF(entity_base);
  Py_DECREF(derived);
  Py_DECREF(foreign);
}

TEST_F(EntityDefXmlLoadingTest, LoadChecksAndComponentFlagsFollowRequestedComponentType)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  KBEngine::ScriptDefModule module("FlagProbe", 78);
  module.setBase(false);
  module.setCell(false);
  module.setClient(false);

  KBEngine::EntityDef::__loadComponentType = KBEngine::BASEAPP_TYPE;
  EXPECT_FALSE(KBEngine::EntityDef::isLoadScriptModule(&module));
  KBEngine::EntityDef::setScriptModuleHasComponentEntity(&module, true);
  EXPECT_TRUE(module.hasBase());
  EXPECT_FALSE(module.hasCell());
  EXPECT_FALSE(module.hasClient());
  EXPECT_TRUE(KBEngine::EntityDef::isLoadScriptModule(&module));

  module.setBase(false);
  module.setCell(false);
  module.setClient(false);
  KBEngine::EntityDef::__loadComponentType = KBEngine::CELLAPP_TYPE;
  KBEngine::EntityDef::setScriptModuleHasComponentEntity(&module, true);
  EXPECT_FALSE(module.hasBase());
  EXPECT_TRUE(module.hasCell());
  EXPECT_FALSE(module.hasClient());
  EXPECT_TRUE(KBEngine::EntityDef::isLoadScriptModule(&module));

  module.setBase(false);
  module.setCell(false);
  module.setClient(false);
  KBEngine::EntityDef::__loadComponentType = KBEngine::CLIENT_TYPE;
  KBEngine::EntityDef::setScriptModuleHasComponentEntity(&module, true);
  EXPECT_FALSE(module.hasBase());
  EXPECT_FALSE(module.hasCell());
  EXPECT_TRUE(module.hasClient());
  EXPECT_TRUE(KBEngine::EntityDef::isLoadScriptModule(&module));

  module.setBase(false);
  module.setCell(false);
  module.setClient(false);
  KBEngine::EntityDef::__loadComponentType = KBEngine::TOOL_TYPE;
  EXPECT_FALSE(KBEngine::EntityDef::isLoadScriptModule(&module));

  KBEngine::EntityDef::__loadComponentType = KBEngine::UNKNOWN_COMPONENT_TYPE;
  KBEngine::EntityDef::setScriptModuleHasComponentEntity(&module, true);
  EXPECT_FALSE(module.hasBase());
  EXPECT_TRUE(module.hasCell());
  EXPECT_FALSE(module.hasClient());
  EXPECT_TRUE(KBEngine::EntityDef::isLoadScriptModule(&module));
}

TEST_F(EntityDefXmlLoadingTest, CheckDefMethodHandlesCallerCheckMismatchAndMissingMethod)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(env.defs_dir / "types.xml", "<root/>\n");
  ASSERT_TRUE(KBEngine::DataTypes::initialize((env.defs_dir / "types.xml").string()));

  auto* py_main = PyImport_AddModule("__main__");
  ASSERT_NE(py_main, nullptr);
  ASSERT_EQ(
      PyRun_SimpleString(
          "class CellProbe:\n"
          "    def ExposedOk(self, callerID, value):\n"
          "        return value\n"
          "    def WrongArgs(self):\n"
          "        return None\n"
          "class MissingProbe:\n"
          "    def Other(self):\n"
          "        return None\n"),
      0);

  auto* cell_probe = PyObject_GetAttrString(py_main, "CellProbe");
  auto* missing_probe = PyObject_GetAttrString(py_main, "MissingProbe");
  ASSERT_NE(cell_probe, nullptr);
  ASSERT_NE(missing_probe, nullptr);

  KBEngine::ScriptDefModule caller_check_module("CallerCheck", 79);
  auto* exposed_method = new KBEngine::MethodDescription(
      801,
      KBEngine::CELLAPP_TYPE,
      "ExposedOk",
      KBEngine::MethodDescription::EXPOSED);
  ASSERT_TRUE(exposed_method->pushArgType(KBEngine::DataTypes::getDataType("UINT8")));
  ASSERT_TRUE(caller_check_module.addCellMethodDescription("ExposedOk", exposed_method));

  KBEngine::EntityDef::__loadComponentType = KBEngine::CELLAPP_TYPE;
  ASSERT_TRUE(KBEngine::EntityDef::checkDefMethod(&caller_check_module, cell_probe, "CallerCheck"));
  EXPECT_EQ(exposed_method->isExposed(), KBEngine::MethodDescription::EXPOSED_AND_CALLER_CHECK);

  KBEngine::ScriptDefModule wrong_args_module("WrongArgs", 80);
  auto* wrong_args_method = new KBEngine::MethodDescription(
      802,
      KBEngine::CELLAPP_TYPE,
      "WrongArgs",
      KBEngine::MethodDescription::NO_EXPOSED);
  ASSERT_TRUE(wrong_args_method->pushArgType(KBEngine::DataTypes::getDataType("UINT8")));
  ASSERT_TRUE(wrong_args_module.addCellMethodDescription("WrongArgs", wrong_args_method));
  EXPECT_FALSE(KBEngine::EntityDef::checkDefMethod(&wrong_args_module, cell_probe, "WrongArgs"));

  KBEngine::ScriptDefModule missing_method_module("MissingMethod", 81);
  auto* missing_method = new KBEngine::MethodDescription(
      803,
      KBEngine::CELLAPP_TYPE,
      "NoSuchMethod",
      KBEngine::MethodDescription::NO_EXPOSED);
  ASSERT_TRUE(missing_method_module.addCellMethodDescription("NoSuchMethod", missing_method));
  EXPECT_FALSE(KBEngine::EntityDef::checkDefMethod(&missing_method_module, missing_probe, "MissingMethod"));

  Py_DECREF(cell_probe);
  Py_DECREF(missing_probe);
}

TEST_F(EntityDefXmlLoadingTest, LoadAllEntityScriptModulesLoadsDerivedClassesAndSkipsOptionalMissingModules)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.scripts_dir / "entities.xml",
      "<root><Avatar hasCell=\"true\"/><Ghost hasCell=\"false\"/></root>\n");
  write_file(
      env.scripts_dir / "Avatar.py",
      "from __main__ import EntityBase\n"
      "class Avatar(EntityBase):\n"
      "    pass\n");
  prepend_python_path(env.scripts_dir.string());

  auto* py_main = PyImport_AddModule("__main__");
  ASSERT_NE(py_main, nullptr);
  ASSERT_EQ(
      PyRun_SimpleString(
          "class EntityBase:\n"
          "    pass\n"),
      0);

  auto* entity_base = reinterpret_cast<PyTypeObject*>(PyObject_GetAttrString(py_main, "EntityBase"));
  ASSERT_NE(entity_base, nullptr);

  KBEngine::g_componentType = KBEngine::CELLAPP_TYPE;
  KBEngine::EntityDef::__loadComponentType = KBEngine::CELLAPP_TYPE;
  std::vector<PyTypeObject*> script_base_types{entity_base};
  KBEngine::EntityDef::__scriptBaseTypes = script_base_types;

  auto* avatar = KBEngine::EntityDef::registerNewScriptDefModule("Avatar");
  auto* ghost = KBEngine::EntityDef::registerNewScriptDefModule("Ghost");
  avatar->setCell(true);
  ghost->setCell(false);

  ASSERT_TRUE(KBEngine::EntityDef::loadAllEntityScriptModules(
      env.scripts_dir.string() + "/",
      script_base_types));
  EXPECT_NE(avatar->getScriptType(), nullptr);
  EXPECT_EQ(ghost->getScriptType(), nullptr);

  Py_DECREF(entity_base);
}

TEST_F(EntityDefXmlLoadingTest, LoadAllEntityScriptModulesRejectsInvalidSubclass)
{
  EntityDefTestEnv env;
  ASSERT_TRUE(env.ready);

  write_file(
      env.scripts_dir / "entities.xml",
      "<root><BrokenForeign hasCell=\"true\"/></root>\n");
  write_file(
      env.scripts_dir / "BrokenForeign.py",
      "class BrokenForeign:\n"
      "    pass\n");
  prepend_python_path(env.scripts_dir.string());

  auto* py_main = PyImport_AddModule("__main__");
  ASSERT_NE(py_main, nullptr);
  ASSERT_EQ(
      PyRun_SimpleString(
          "class EntityBase:\n"
          "    pass\n"),
      0);

  auto* entity_base = reinterpret_cast<PyTypeObject*>(PyObject_GetAttrString(py_main, "EntityBase"));
  ASSERT_NE(entity_base, nullptr);

  KBEngine::g_componentType = KBEngine::CELLAPP_TYPE;
  KBEngine::EntityDef::__loadComponentType = KBEngine::CELLAPP_TYPE;
  std::vector<PyTypeObject*> script_base_types{entity_base};
  KBEngine::EntityDef::__scriptBaseTypes = script_base_types;

  auto* foreign = KBEngine::EntityDef::registerNewScriptDefModule("BrokenForeign");
  foreign->setCell(true);

  EXPECT_FALSE(KBEngine::EntityDef::loadAllEntityScriptModules(
      env.scripts_dir.string() + "/",
      script_base_types));

  Py_DECREF(entity_base);
}

}  // namespace
