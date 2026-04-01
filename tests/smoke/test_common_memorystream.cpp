#include <gtest/gtest.h>

#include <string>

#include "common/memorystream.h"

TEST(CommonMemoryStreamBootstrapTest, SerializesBasicValues) {
  KBEngine::MemoryStream stream;

  stream << uint32_t(42);
  stream << std::string("KBEngine");
  stream << KBEngine::BASEAPP_TYPE;
  stream << KBEngine::ENTITYCALL_TYPE_BASE;
  stream << true;

  uint32_t number = 0;
  std::string text;
  KBEngine::COMPONENT_TYPE component = KBEngine::UNKNOWN_COMPONENT_TYPE;
  KBEngine::ENTITYCALL_TYPE entity_call = KBEngine::ENTITYCALL_TYPE_CELL;
  bool flag = false;

  stream >> number;
  stream >> text;
  stream >> component;
  stream >> entity_call;
  stream >> flag;

  EXPECT_EQ(number, 42u);
  EXPECT_EQ(text, "KBEngine");
  EXPECT_EQ(component, KBEngine::BASEAPP_TYPE);
  EXPECT_EQ(entity_call, KBEngine::ENTITYCALL_TYPE_BASE);
  EXPECT_TRUE(flag);
  EXPECT_EQ(stream.length(), 0u);
}

TEST(CommonMemoryStreamBootstrapTest, ReadsAndWritesBlobData) {
  KBEngine::MemoryStream stream;
  const std::string payload("abc\0xyz", 7);

  stream.appendBlob(payload);

  std::string decoded;
  ASSERT_EQ(stream.readBlob(decoded), payload.size());
  EXPECT_EQ(decoded, payload);
}

TEST(CommonMemoryStreamBootstrapTest, ThrowsOnWriteOverflow) {
  KBEngine::MemoryStream stream;
  stream.wpos(static_cast<int>(KBEngine::MemoryStream::MAX_SIZE - 1));

  EXPECT_THROW(stream.append("ab", 2), KBEngine::MemoryStreamWriteOverflow);
}

TEST(CommonMemoryStreamBootstrapTest, SupportsPoolLifecycle) {
  KBEngine::MemoryStream* stream = KBEngine::MemoryStream::createPoolObject("test");
  ASSERT_NE(stream, nullptr);

  (*stream) << uint8_t(7);
  EXPECT_TRUE(stream->isEnabledPoolObject());

  KBEngine::MemoryStream::reclaimPoolObject(stream);

  KBEngine::MemoryStream* reused = KBEngine::MemoryStream::createPoolObject("test-reuse");
  ASSERT_NE(reused, nullptr);
  EXPECT_EQ(reused->length(), 0u);
  KBEngine::MemoryStream::reclaimPoolObject(reused);
}
