// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPTSTDOUTERRHOOK_H
#define KBE_SCRIPTSTDOUTERRHOOK_H

#include "common/common.h"
#include "scriptobject.h"
#include "scriptstdouterr.h"

// PyEval_InitThreads() became a no-op long before 3.12 and is removed there.
#if PY_VERSION_HEX >= 0x030B0000
#ifndef PyEval_InitThreads
#define PyEval_InitThreads() ((void)0)
#endif
#endif

namespace KBEngine{ namespace script{

class ScriptStdOutErrHook : public ScriptStdOutErr
{
public:
	ScriptStdOutErrHook();
	~ScriptStdOutErrHook();

	virtual void error_msg(const char* msg, uint32 msglen);
	virtual void info_msg(const char* msg, uint32 msglen);

	INLINE void setHookBuffer(std::string* buffer);

	INLINE void setPrint(bool v);

protected:
	std::string* pBuffer_;
	std::string buffer_;
	bool isPrint_;
} ;

}
}

#ifdef CODE_INLINE
#include "scriptstdouterrhook.inl"
#endif

#endif // KBE_SCRIPTSTDOUTERRHOOK_H
