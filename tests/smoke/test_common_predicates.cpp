#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "common/stdfindif_handers.h"

TEST(CommonPredicatesBootstrapTest, FindsMatchingStringInVector)
{
  std::vector<std::string> values = {"alpha", "beta", "gamma"};
  const auto iter = std::find_if(values.begin(), values.end(),
    KBEngine::find_vec_string_exist_handle<char>("beta"));
  ASSERT_NE(iter, values.end());
  EXPECT_EQ(*iter, "beta");
}

TEST(CommonPredicatesBootstrapTest, FindsMatchingPointerValue)
{
  int a = 1;
  int b = 2;
  int c = 3;
  std::vector<int*> values = {&a, &b, &c};

  const auto iter = std::find_if(values.begin(), values.end(),
    KBEngine::findif_vector_obj_exist_handler<int*>(&b));
  ASSERT_NE(iter, values.end());
  EXPECT_EQ(*iter, &b);
}
