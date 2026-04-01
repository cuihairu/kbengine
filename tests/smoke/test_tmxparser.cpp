#include <gtest/gtest.h>

#include <string>

#include "TmxImage.h"
#include "TmxImageLayer.h"
#include "TmxLayer.h"
#include "TmxMap.h"
#include "TmxObject.h"
#include "TmxObjectGroup.h"
#include "TmxTile.h"
#include "TmxTileset.h"

TEST(TmxParserTest, ParsesMinimalMapWithTilesObjectsAndImages)
{
  const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<map version="1.2" orientation="orthogonal" width="2" height="1" tilewidth="32" tileheight="32">
  <properties>
    <property name="theme" value="forest"/>
  </properties>
  <tileset firstgid="1" name="terrain" tilewidth="32" tileheight="32" margin="0" spacing="0">
    <image source="terrain.png" width="64" height="32" trans="ff00ff"/>
    <tile id="0">
      <properties>
        <property name="walkable" value="1"/>
      </properties>
    </tile>
  </tileset>
  <layer name="Ground" width="2" height="1" opacity="0.5" visible="1">
    <properties>
      <property name="kind" value="base"/>
    </properties>
    <data encoding="csv">1,2</data>
  </layer>
  <imagelayer name="Backdrop" width="2" height="1" opacity="1" visible="1">
    <image source="bg.png" width="128" height="32"/>
    <properties>
      <property name="parallax" value="far"/>
    </properties>
  </imagelayer>
  <objectgroup name="Objects" width="64" height="32" visible="1">
    <properties>
      <property name="group" value="spawn"/>
    </properties>
    <object name="Spawn" type="Point" x="10" y="12" width="5" height="6">
      <properties>
        <property name="team" value="blue"/>
      </properties>
    </object>
  </objectgroup>
</map>)";

  Tmx::Map map;
  map.ParseText(xml);

  ASSERT_FALSE(map.HasError()) << map.GetErrorText();
  EXPECT_DOUBLE_EQ(map.GetVersion(), 1.2);
  EXPECT_EQ(map.GetOrientation(), Tmx::TMX_MO_ORTHOGONAL);
  EXPECT_EQ(map.GetWidth(), 2);
  EXPECT_EQ(map.GetHeight(), 1);
  EXPECT_EQ(map.GetTileWidth(), 32);
  EXPECT_EQ(map.GetTileHeight(), 32);
  EXPECT_EQ(map.GetProperties().GetLiteralProperty("theme"), "forest");

  ASSERT_EQ(map.GetNumTilesets(), 1);
  const Tmx::Tileset* tileset = map.GetTileset(0);
  ASSERT_NE(tileset, nullptr);
  EXPECT_EQ(tileset->GetName(), "terrain");
  EXPECT_EQ(tileset->GetFirstGid(), 1);
  ASSERT_NE(tileset->GetImage(), nullptr);
  EXPECT_EQ(tileset->GetImage()->GetSource(), "terrain.png");
  EXPECT_EQ(tileset->GetImage()->GetWidth(), 64);
  EXPECT_EQ(tileset->GetImage()->GetHeight(), 32);
  EXPECT_EQ(tileset->GetImage()->GetTransparentColor(), "ff00ff");
  ASSERT_NE(tileset->GetTile(0), nullptr);
  EXPECT_EQ(tileset->GetTile(0)->GetProperties().GetLiteralProperty("walkable"), "1");

  ASSERT_EQ(map.GetNumLayers(), 1);
  const Tmx::Layer* layer = map.GetLayer(0);
  ASSERT_NE(layer, nullptr);
  EXPECT_EQ(layer->GetName(), "Ground");
  EXPECT_EQ(layer->GetWidth(), 2);
  EXPECT_EQ(layer->GetHeight(), 1);
  EXPECT_EQ(layer->GetEncoding(), Tmx::TMX_ENCODING_CSV);
  EXPECT_TRUE(layer->IsVisible());
  EXPECT_EQ(layer->GetProperties().GetLiteralProperty("kind"), "base");
  EXPECT_EQ(layer->GetTileId(0, 0), 0u);
  EXPECT_EQ(layer->GetTileId(1, 0), 1u);
  EXPECT_EQ(layer->GetTileTilesetIndex(0, 0), 0);
  EXPECT_EQ(layer->GetTileTilesetIndex(1, 0), 0);

  ASSERT_EQ(map.GetNumImageLayers(), 1);
  const Tmx::ImageLayer* image_layer = map.GetImageLayer(0);
  ASSERT_NE(image_layer, nullptr);
  EXPECT_EQ(image_layer->GetName(), "Backdrop");
  ASSERT_NE(image_layer->GetImage(), nullptr);
  EXPECT_EQ(image_layer->GetImage()->GetSource(), "bg.png");
  EXPECT_EQ(image_layer->GetProperties().GetLiteralProperty("parallax"), "far");

  ASSERT_EQ(map.GetNumObjectGroups(), 1);
  const Tmx::ObjectGroup* object_group = map.GetObjectGroup(0);
  ASSERT_NE(object_group, nullptr);
  EXPECT_EQ(object_group->GetName(), "Objects");
  EXPECT_EQ(object_group->GetProperties().GetLiteralProperty("group"), "spawn");
  ASSERT_EQ(object_group->GetNumObjects(), 1);
  const Tmx::Object* object = object_group->GetObject(0);
  ASSERT_NE(object, nullptr);
  EXPECT_EQ(object->GetName(), "Spawn");
  EXPECT_EQ(object->GetType(), "Point");
  EXPECT_EQ(object->GetX(), 10);
  EXPECT_EQ(object->GetY(), 12);
  EXPECT_EQ(object->GetWidth(), 5);
  EXPECT_EQ(object->GetHeight(), 6);
  EXPECT_EQ(object->GetProperties().GetLiteralProperty("team"), "blue");
}

TEST(TmxParserTest, ReportsParseErrorsForMalformedXml)
{
  Tmx::Map map;
  map.ParseText("<map><layer></map>");

  EXPECT_TRUE(map.HasError());
  EXPECT_EQ(map.GetErrorCode(), Tmx::TMX_PARSING_ERROR);
  EXPECT_FALSE(map.GetErrorText().empty());
}
