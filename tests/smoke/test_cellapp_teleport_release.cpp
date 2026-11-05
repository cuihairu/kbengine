#include <gtest/gtest.h>

#include <Python.h>

#include "cellapp/teleport_release.h"

namespace {

// Verifies the reference-count contract of the failed-teleport teardown helper.
//
// Callers remove/destroy the entity (releasing the entity-map reference via
// pEntities_->erase()/destroy()) BEFORE invoking the helper. The helper only
// balances:
//   1. the extra Py_INCREF the message handler took to keep the entity alive
//      across re-deserialization, and
//   2. the createObject base reference when destroy() was idempotent (the
//      entity was already marked destroyed, so destroy() skipped its own
//      Py_DECREF).
//
// A fresh dict is used as the probe object because CPython's small ints are
// immortal (Py_INCREF/Py_DECREF are no-ops), which would make the count
// assertions meaningless.
class CellappTeleportReleaseTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!Py_IsInitialized())
    {
      Py_Initialize();
    }
  }
};

TEST_F(CellappTeleportReleaseTest, AlreadyDestroyedReleasesBaseAndIncref)
{
  // createObject base reference (1). The handler then took an extra Py_INCREF
  // before re-deserialization, so refcount = 2 entering the teardown. The map
  // reference was already released by destroyEntity()/erase(). Keep a guard.
  PyObject* obj = PyDict_New();
  ASSERT_NE(obj, nullptr);
  ASSERT_EQ(Py_REFCNT(obj), 1);  // base
  Py_INCREF(obj);                // handler's extra Py_INCREF
  Py_INCREF(obj);                // test guard
  ASSERT_EQ(Py_REFCNT(obj), 3);

  KBEngine::releaseTeleportFailureEntityRefs(obj, /*wasDestroyed=*/true);

  // Both the handler's Py_INCREF and the base reference are released; only the
  // test guard remains.
  EXPECT_EQ(Py_REFCNT(obj), 1);
  Py_DECREF(obj);
}

TEST_F(CellappTeleportReleaseTest, NotDestroyedReleasesOnlyIncref)
{
  // createObject base reference (1) + handler's extra Py_INCREF (1). destroy()
  // ran its body and released the base reference; the map reference was
  // released by erase(). Only the handler's Py_INCREF remains to be balanced.
  PyObject* obj = PyDict_New();
  ASSERT_NE(obj, nullptr);
  ASSERT_EQ(Py_REFCNT(obj), 1);  // base
  Py_INCREF(obj);                // handler's extra Py_INCREF
  Py_INCREF(obj);                // test guard
  ASSERT_EQ(Py_REFCNT(obj), 3);

  KBEngine::releaseTeleportFailureEntityRefs(obj, /*wasDestroyed=*/false);

  // Only the handler's Py_INCREF is released; the guard remains.
  EXPECT_EQ(Py_REFCNT(obj), 2);
  Py_DECREF(obj);  // guard
  Py_DECREF(obj);  // base reference (simulating destroy() body)
}

}  // namespace
