#include <gtest/gtest.h>

#include <cstdint>

#include "common/memorystream_converter.h"

TEST(CommonEndianBootstrapTest, ReverseConvertSwapsByteOrder)
{
  std::uint32_t value = 0x11223344u;
  KBEngine::EndianConvertReverse(value);

#if KBENGINE_ENDIAN == KBENGINE_BIG_ENDIAN
  EXPECT_EQ(value, 0x11223344u);
#else
  EXPECT_EQ(value, 0x44332211u);
#endif
}

TEST(CommonEndianBootstrapTest, SingleByteConversionIsNoOp)
{
  std::uint8_t value = 0x7Fu;
  KBEngine::EndianConvert(value);
  EXPECT_EQ(value, 0x7Fu);
}
