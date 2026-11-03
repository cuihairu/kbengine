// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "pickler.h"
#include "helper/profile.h"

namespace KBEngine{ namespace script {

PyObject* Pickler::picklerMethod_ = NULL;
PyObject* Pickler::unPicklerMethod_ = NULL;
PyObject* Pickler::restrictedUnpicklerClass_ = NULL;
PyObject* Pickler::pyPickleFuncTableModule_ = NULL;
bool Pickler::isInit = false;


//-------------------------------------------------------------------------------------
bool Pickler::initialize(void)
{
	if (isInit)
		return true;

	PyObject* cPickleModule = PyImport_ImportModule("pickle");

	if (cPickleModule)
	{
		picklerMethod_ = PyObject_GetAttrString(cPickleModule, "dumps");
		unPicklerMethod_ = PyObject_GetAttrString(cPickleModule, "Unpickler");
		Py_DECREF(cPickleModule);

		if (!picklerMethod_)
		{
			ERROR_MSG("Pickler::initialize: get dumps error!\n");
			SCRIPT_ERROR_CHECK();
			finalise();
			return false;
		}

		if (!unPicklerMethod_)
		{
			ERROR_MSG("Pickler::initialize: get Unpickler class error!\n");
			SCRIPT_ERROR_CHECK();
			finalise();
			return false;
		}
	}
	else
	{
		ERROR_MSG("Pickler::initialize: can't import pickle!\n");
		SCRIPT_ERROR_CHECK();
		finalise();
		return false;
	}

	isInit = picklerMethod_ && unPicklerMethod_;

	if (isInit)
	{
		// 初始化一个unpickle函数表模块， 所有自定义类的unpickle函数都需要在此注册
		pyPickleFuncTableModule_ = PyImport_AddModule("_upf");

		static struct PyModuleDef moduleDesc =
		{
			 PyModuleDef_HEAD_INIT,
			 "_upf",
			 "This module is created by KBEngine!",
			 -1,
			 NULL
		};

		PyObject* m = PyModule_Create(&moduleDesc);
		if (m == NULL)
		{
			isInit = false;
			SCRIPT_ERROR_CHECK();
			finalise();
			return false;
		}

		// Build a restricted Unpickler subclass that only allows engine callbacks
		// registered on the `_upf` module. This prevents attacker-controlled pickle
		// payloads from importing arbitrary globals (e.g. os.system), which would
		// otherwise allow remote code execution through PY_* argument deserialization.
		restrictedUnpicklerClass_ = createRestrictedUnpicklerClass();
		if (restrictedUnpicklerClass_ == NULL)
		{
			isInit = false;
			SCRIPT_ERROR_CHECK();
			finalise();
			return false;
		}
	}

	return isInit;
}

//-------------------------------------------------------------------------------------
PyObject* Pickler::createRestrictedUnpicklerClass(void)
{
	PyObject* pickleModule = PyImport_ImportModule("pickle");
	if (!pickleModule)
	{
		ERROR_MSG("Pickler::createRestrictedUnpicklerClass: can't import pickle!\n");
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* globals = PyDict_New();
	if (!globals)
	{
		Py_DECREF(pickleModule);
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* sysModule = PyImport_ImportModule("sys");
	if (!sysModule)
	{
		Py_DECREF(pickleModule);
		Py_DECREF(globals);
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyDict_SetItemString(globals, "pickle", pickleModule);
	PyDict_SetItemString(globals, "sys", sysModule);
	Py_DECREF(pickleModule);
	Py_DECREF(sysModule);

	const char* restrictedUnpicklerSrc =
		"class _RestrictedUnpickler(pickle.Unpickler):\n"
		"    _SAFE_BUILTINS = frozenset(('bool', 'bytearray', 'bytes', 'complex',\n"
		"                                 'dict', 'float', 'frozenset', 'int', 'list',\n"
		"                                 'range', 'set', 'slice', 'str', 'tuple'))\n"
		"    def find_class(self, module, name):\n"
		"        if module == '_upf':\n"
		"            m = sys.modules.get('_upf')\n"
		"            if m is not None and hasattr(m, name):\n"
		"                return getattr(m, name)\n"
		"        if module in ('builtins', '__builtin__') and name in self._SAFE_BUILTINS:\n"
		"            return getattr(__import__('builtins'), name)\n"
		"        if module == '_codecs' and name == 'encode':\n"
		"            return getattr(__import__('_codecs'), name)\n"
		"        raise pickle.UnpicklingError('forbidden global: %s.%s' % (module, name))\n";

	PyObject* pyCode = Py_CompileString(restrictedUnpicklerSrc, "<kbe_restricted_unpickler>", Py_file_input);
	if (!pyCode)
	{
		Py_DECREF(globals);
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* pyModule = PyModule_New("_kbe_restricted_unpickler");
	if (!pyModule)
	{
		Py_DECREF(pyCode);
		Py_DECREF(globals);
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* md = PyModule_GetDict(pyModule);
	PyObject* execResult = PyEval_EvalCode(pyCode, globals, md);
	Py_DECREF(pyCode);
	if (!execResult)
	{
		Py_DECREF(pyModule);
		Py_DECREF(globals);
		SCRIPT_ERROR_CHECK();
		return NULL;
	}
	Py_DECREF(execResult);

	PyObject* cls = PyObject_GetAttrString(pyModule, "_RestrictedUnpickler");
	Py_DECREF(pyModule);
	Py_DECREF(globals);
	if (!cls)
	{
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	return cls;
}

//-------------------------------------------------------------------------------------
void Pickler::finalise(void)
{
	Py_XDECREF(picklerMethod_);
	Py_XDECREF(unPicklerMethod_);
	Py_XDECREF(restrictedUnpicklerClass_);
	
	picklerMethod_ = NULL;
	unPicklerMethod_ = NULL;	
	restrictedUnpicklerClass_ = NULL;
	pyPickleFuncTableModule_ = NULL;
}

//-------------------------------------------------------------------------------------
PyObject* Pickler::getUnpickleFunc(const char* funcName)
{ 
	PyObject* pyfunc = PyObject_GetAttrString(pyPickleFuncTableModule_, funcName); 
	if(pyfunc == NULL)
	{
		SCRIPT_ERROR_CHECK();
	}

	return pyfunc;
}

//-------------------------------------------------------------------------------------
std::string Pickler::pickle(PyObject* pyobj)
{
	AUTO_SCOPED_PROFILE("pickle");

	PyObject* pyRet = PyObject_CallFunction(picklerMethod_, 
		const_cast<char*>("(O)"), pyobj);
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		std::string str;
		str.assign(PyBytes_AsString(pyRet), PyBytes_Size(pyRet));
		S_RELEASE(pyRet);
		return str;
	}
	
	return "";
}

//-------------------------------------------------------------------------------------
std::string Pickler::pickle(PyObject* pyobj, int8 protocol)
{
	AUTO_SCOPED_PROFILE("pickleEx");

	PyObject* pyRet = PyObject_CallFunction(picklerMethod_, 
		const_cast<char*>("(Oi)"), pyobj, protocol);
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		std::string str;
		str.assign(PyBytes_AsString(pyRet), PyBytes_Size(pyRet));
		S_RELEASE(pyRet);
		return str;
	}
	
	return "";
}

//-------------------------------------------------------------------------------------
PyObject* Pickler::unpickle(const std::string& str)
{
	AUTO_SCOPED_PROFILE("unpickle");

	PyObject* pyRet = NULL;

	if (restrictedUnpicklerClass_ == NULL)
	{
		ERROR_MSG("Pickler::unpickle: restricted unpickler not initialised!\n");
		return NULL;
	}

	PyObject* pyBytes = PyBytes_FromStringAndSize(str.data(), str.length());
	if (!pyBytes)
	{
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* bioMod = PyImport_ImportModule("io");
	PyObject* bytesIOCls = bioMod ? PyObject_GetAttrString(bioMod, "BytesIO") : NULL;
	Py_XDECREF(bioMod);
	if (!bytesIOCls)
	{
		Py_DECREF(pyBytes);
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* bio = PyObject_CallFunctionObjArgs(bytesIOCls, pyBytes, NULL);
	Py_DECREF(pyBytes);
	Py_DECREF(bytesIOCls);
	if (!bio)
	{
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	PyObject* inst = PyObject_CallFunctionObjArgs(restrictedUnpicklerClass_, bio, NULL);
	Py_DECREF(bio);
	if (!inst)
	{
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	pyRet = PyObject_CallMethod(inst, "load", NULL);
	Py_DECREF(inst);
	
	if (!pyRet)
	{
		std::string buff;
		for (size_t i = 0; i < str.length(); ++i)
		{
			if ((uchar)str[i] >= 32 && (uchar)str[i] <= 126)
				buff += str[i];
			else
				buff += fmt::format("\\x{:02x}", (uchar)str[i]);
		}

		ERROR_MSG(fmt::format("Pickler::unpickle: failed to unpickle[{}] len={}.\n",
			buff, str.length()));
	}
	
	SCRIPT_ERROR_CHECK();
	return pyRet;	
}

//-------------------------------------------------------------------------------------
void Pickler::registerUnpickleFunc(PyObject* pyFunc, const char* funcName)
{
	if(PyObject_SetAttrString(pyPickleFuncTableModule_, funcName, pyFunc) == -1)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}
}

//-------------------------------------------------------------------------------------

}
}
