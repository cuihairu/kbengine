#include <gtest/gtest.h>

#include <fmt/base.h>
#include <fmt/format.h>

TEST(FmtBootstrapTest, UsesExpectedVendoredVersion) {
  EXPECT_EQ(FMT_VERSION, 120100);
}

TEST(FmtBootstrapTest, FormatsBasicString) {
  EXPECT_EQ(fmt::format("kbe-{}", 17), "kbe-17");
}
