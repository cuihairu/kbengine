#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "helper/debug_helper.h"
#include "navigation/navigation_mesh_handle.h"

namespace {

namespace fs = std::filesystem;

// 回归测试：4f09902（navmesh 瓦片解码边界检查）
// 构造合法 NavMeshSetHeader + 恶意 NavMeshTileHeader（dataSize<=0 或越界），
// 验证 tryReadNavmesh 拒绝而非越界读/分配，_create 返回 false 且不崩溃。
struct TestNavMeshHeader {
  int version;
  int tileCount;
  dtNavMeshParams params;
};

struct TestNavMeshTileHeader {
  dtTileRef tileRef;
  int dataSize;
};

void write_binary(const fs::path& path, const char* data, size_t size) {
  std::ofstream out(path, std::ios::binary);
  ASSERT_TRUE(out.is_open()) << path;
  out.write(data, static_cast<std::streamsize>(size));
  out.close();
}

void append(std::string& buf, const void* data, size_t size) {
  buf.append(static_cast<const char*>(data), size);
}

bool try_load_malicious_tile(const fs::path& path, int tile_data_size) {
  KBEngine::NavMeshSetHeader header{};
  header.version = KBEngine::NavMeshHandle::RCN_NAVMESH_VERSION;
  // tileCount=2 使失败后的 NavMeshSetHeaderEx 回退解析（读本字段为 version=2）也失败，
  // 从而确保 _create 因恶意瓦片解码被拒绝而返回 false。
  header.tileCount = 2;
  header.params.orig[0] = 0.0f;
  header.params.orig[1] = 0.0f;
  header.params.orig[2] = 0.0f;
  header.params.tileWidth = 1.0f;
  header.params.tileHeight = 1.0f;
  header.params.maxTiles = 1;
  header.params.maxPolys = 1;

  KBEngine::NavMeshTileHeader tile{};
  tile.tileRef = 1;
  tile.dataSize = tile_data_size;

  std::string buf;
  append(buf, &header, sizeof(header));
  append(buf, &tile, sizeof(tile));
  // 若 dataSize 在合理范围内，放入少量填充字节验证后续边界检查。
  if (tile_data_size > 0 && tile_data_size < 1024) {
    const char filler[64] = {0};
    append(buf, filler, sizeof(filler));
  }

  write_binary(path, buf.data(), buf.size());

  KBEngine::NavMeshHandle handle;
  return KBEngine::NavMeshHandle::_create(
      0, path.string(), path.string(), &handle);
}

class NavigationNavmeshDecodeTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (!KBEngine::DebugHelper::isInit()) {
      KBEngine::DebugHelper::initialize(KBEngine::UNKNOWN_COMPONENT_TYPE);
    }
    dir_ = fs::temp_directory_path() / "kbengine_navmesh_decode_tests";
    fs::remove_all(dir_);
    fs::create_directories(dir_);
  }

  void TearDown() override {
    fs::remove_all(dir_);
    KBEngine::DebugHelper::finalise(true);
  }

  fs::path dir_;
};

TEST_F(NavigationNavmeshDecodeTest, RejectsZeroSizeTile) {
  EXPECT_FALSE(try_load_malicious_tile(dir_ / "zero.navmesh", 0));
}

TEST_F(NavigationNavmeshDecodeTest, RejectsNegativeSizeTile) {
  EXPECT_FALSE(try_load_malicious_tile(dir_ / "neg.navmesh", -1));
}

TEST_F(NavigationNavmeshDecodeTest, RejectsOverrunTile) {
  EXPECT_FALSE(try_load_malicious_tile(dir_ / "overrun.navmesh", 4096));
}

}  // namespace
