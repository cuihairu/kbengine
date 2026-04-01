#include <gtest/gtest.h>

#include "math/math.h"

TEST(MathBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(MathBootstrapTest, Vector3BasicOperations)
{
  // Vector3 is a typedef of G3D::Vector3
  Vector3 v(1.0f, 2.0f, 3.0f);
  EXPECT_FLOAT_EQ(v.x, 1.0f);
  EXPECT_FLOAT_EQ(v.y, 2.0f);
  EXPECT_FLOAT_EQ(v.z, 3.0f);
}
