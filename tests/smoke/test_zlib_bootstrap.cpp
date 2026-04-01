#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

#include <zlib.h>

TEST(ZlibBootstrapTest, CompressAndUncompressRoundTrip) {
  const std::string original = "kbengine-cmake-zlib-bootstrap";

  uLongf compressed_size = compressBound(static_cast<uLong>(original.size()));
  std::vector<Bytef> compressed(compressed_size);

  ASSERT_EQ(compress(compressed.data(), &compressed_size,
                     reinterpret_cast<const Bytef*>(original.data()),
                     static_cast<uLong>(original.size())),
            Z_OK);

  uLongf restored_size = static_cast<uLongf>(original.size());
  std::vector<Bytef> restored(restored_size);

  ASSERT_EQ(uncompress(restored.data(), &restored_size,
                       compressed.data(), compressed_size),
            Z_OK);
  ASSERT_EQ(restored_size, original.size());

  EXPECT_EQ(std::memcmp(restored.data(), original.data(), original.size()), 0);
}
