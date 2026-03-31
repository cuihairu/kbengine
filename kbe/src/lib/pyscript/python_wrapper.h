// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PYTHON_WRAPPER_H
#define KBE_PYTHON_WRAPPER_H

#if defined(_WIN32) && defined(_DEBUG)
#define KBE_RESTORE_DEBUG_MACRO_AFTER_PYTHON_H 1
#pragma push_macro("_DEBUG")
#undef _DEBUG
#endif

#include "Python.h"

#if defined(KBE_RESTORE_DEBUG_MACRO_AFTER_PYTHON_H)
#pragma pop_macro("_DEBUG")
#undef KBE_RESTORE_DEBUG_MACRO_AFTER_PYTHON_H
#endif

#endif // KBE_PYTHON_WRAPPER_H
