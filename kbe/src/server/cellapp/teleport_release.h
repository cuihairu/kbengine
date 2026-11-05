// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TELEPORT_RELEASE_H
#define KBE_TELEPORT_RELEASE_H

#include "pyscript/python_wrapper.h"

namespace KBEngine {

// Releases the Python references taken by a failed teleport re-deserialization.
//
// The caller must have already removed/destroyed the entity so that the entity
// map reference (if any) has been released, and must have previously taken an
// extra Py_INCREF on `entity` to keep it alive across the re-deserialization.
//
// `wasDestroyed` must reflect the entity's state before the destroy() call:
//   - true:  destroy() was idempotent (isDestroyed_ already set) and therefore
//            did NOT release the createObject base reference, so an extra
//            decrement is required to avoid leaking it.
//   - false: destroy() ran its body and released the base reference, so only
//            the caller's extra Py_INCREF needs to be balanced.
inline void releaseTeleportFailureEntityRefs(PyObject* entity, bool wasDestroyed)
{
	// Balance the caller's extra Py_INCREF.
	Py_DECREF(entity);

	// destroy() skips its base-reference release when the entity is already
	// destroyed; release that base reference here to avoid a permanent leak.
	if(wasDestroyed)
		Py_DECREF(entity);
}

} // namespace KBEngine

#endif // KBE_TELEPORT_RELEASE_H
