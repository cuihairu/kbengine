#include <gtest/gtest.h>

#include "common/refcountable.h"
#include "common/smartpointer.h"

namespace
{
class RefObject : public KBEngine::RefCountable
{
public:
  static int destroyed_count;

  RefObject()
  {
  }

  void onRefOver() const override
  {
    ++destroyed_count;
    delete const_cast<RefObject*>(this);
  }
};

int RefObject::destroyed_count = 0;
}

TEST(CommonSmartPointerBootstrapTest, IncrementsAndDecrementsRefCountAcrossCopies)
{
  RefObject::destroyed_count = 0;

  RefObject* raw = new RefObject();
  EXPECT_EQ(raw->getRefCount(), 0);

  {
    KBEngine::SmartPointer<RefObject> first(raw);
    EXPECT_EQ(raw->getRefCount(), 1);

    {
      KBEngine::SmartPointer<RefObject> second(first);
      EXPECT_EQ(raw->getRefCount(), 2);
      EXPECT_EQ(second.get(), raw);
    }

    EXPECT_EQ(raw->getRefCount(), 1);
  }

  EXPECT_EQ(RefObject::destroyed_count, 1);
}

TEST(CommonSmartPointerBootstrapTest, ClearReleasesManagedObject)
{
  RefObject::destroyed_count = 0;

  RefObject* raw = new RefObject();
  KBEngine::SmartPointer<RefObject> ptr(raw);
  EXPECT_EQ(raw->getRefCount(), 1);

  ptr.clear();

  EXPECT_EQ(RefObject::destroyed_count, 1);
  EXPECT_EQ(ptr.get(), nullptr);
}

TEST(CommonSmartPointerBootstrapTest, SupportsStealReferenceWithoutExtraIncrement)
{
  RefObject::destroyed_count = 0;

  RefObject* raw = new RefObject();
  raw->incRef();
  EXPECT_EQ(raw->getRefCount(), 1);

  {
    KBEngine::SmartPointer<RefObject> ptr(raw, KBEngine::ConstSmartPointer<RefObject>::STEAL_REF);
    EXPECT_EQ(raw->getRefCount(), 1);
    EXPECT_TRUE(static_cast<bool>(ptr));
  }

  EXPECT_EQ(RefObject::destroyed_count, 1);
}
