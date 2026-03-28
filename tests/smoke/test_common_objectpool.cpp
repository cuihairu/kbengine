#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "common/objectpool.h"

namespace {

struct TestPoolObject : public KBEngine::PoolObject {
  void onReclaimObject() override {
    ++reclaim_count;
    value = 0;
  }

  void onEabledPoolObject() override {
    ++enable_count;
  }

  int value = 0;
  int reclaim_count = 0;
  int enable_count = 0;
};

} // namespace

TEST(CommonObjectPoolBootstrapTest, ReusesReclaimedObjects) {
  KBEngine::ObjectPool<TestPoolObject> pool("TestPool");

  TestPoolObject* first = pool.createObject("first");
  ASSERT_NE(first, nullptr);
  first->value = 42;
  EXPECT_TRUE(first->isEnabledPoolObject());
  EXPECT_EQ(first->poolObjectCreatePoint(), "first");

  pool.reclaimObject(first);
  EXPECT_EQ(pool.size(), 16u);
  EXPECT_FALSE(first->isEnabledPoolObject());
  EXPECT_EQ(first->value, 0);
  EXPECT_EQ(first->reclaim_count, 1);
  EXPECT_EQ(first->poolObjectCreatePoint(), "");

  TestPoolObject* second = pool.createObject("second");
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(pool.size(), 15u);
  EXPECT_NE(second, first);
  EXPECT_EQ(second->enable_count, 1);
  EXPECT_EQ(second->poolObjectCreatePoint(), "second");

  pool.reclaimObject(second);
}

TEST(CommonObjectPoolBootstrapTest, SmartPoolObjectReclaimsOnDestruction) {
  KBEngine::ObjectPool<TestPoolObject> pool("SmartPool");
  TestPoolObject* raw = pool.createObject("smart");

  {
    KBEngine::SmartPoolObject<TestPoolObject> holder(raw, pool);
    ASSERT_EQ(holder.get(), raw);
    holder->value = 7;
    EXPECT_EQ((*holder).value, 7);
  }

  EXPECT_EQ(pool.size(), 16u);
  EXPECT_FALSE(raw->isEnabledPoolObject());
  EXPECT_EQ(raw->value, 0);
  EXPECT_EQ(raw->reclaim_count, 1);

  TestPoolObject* reused = pool.createObject("reused");
  ASSERT_NE(reused, nullptr);
  EXPECT_EQ(pool.size(), 15u);
  pool.reclaimObject(reused);
}

TEST(CommonObjectPoolBootstrapTest, TracksLogPointsPerObjectCheckout) {
  KBEngine::ObjectPool<TestPoolObject> pool("LogPointPool");

  TestPoolObject* first = pool.createObject("alpha");
  TestPoolObject* second = pool.createObject("beta");

  EXPECT_EQ(pool.logPoints()["alpha"].count, 1);
  EXPECT_EQ(pool.logPoints()["beta"].count, 1);

  pool.reclaimObject(first);
  pool.reclaimObject(second);

  EXPECT_EQ(pool.logPoints()["alpha"].count, 0);
  EXPECT_EQ(pool.logPoints()["beta"].count, 0);
}
