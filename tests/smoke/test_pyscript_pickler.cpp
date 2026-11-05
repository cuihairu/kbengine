#include <gtest/gtest.h>

#include "pyscript/python_wrapper.h"

#include <filesystem>
#include <string>
#include <vector>

#include "helper/debug_helper.h"
#include "pyscript/pickler.h"

namespace {

// A benign engine callback that mirrors the shape of registered `_upf` functions.
PyObject* test_upf_callback(PyObject* /*self*/, PyObject* args)
{
  // `args` is a borrowed reference owned by the call machinery; returning it
  // as the result transfers ownership, so take our own reference first.
  Py_INCREF(args);
  return args;
}

std::string bytes_to_string(PyObject* obj)
{
  return std::string(PyBytes_AsString(obj), PyBytes_Size(obj));
}

// Returns a new reference to a pickled bytes object of `obj` using the default
// protocol, matching Pickler::pickle().
PyObject* pickled_bytes(PyObject* obj)
{
  PyObject* pickle_module = PyImport_ImportModule("pickle");
  if (!pickle_module)
    return nullptr;
  PyObject* dumps = PyObject_GetAttrString(pickle_module, "dumps");
  Py_DECREF(pickle_module);
  if (!dumps)
    return nullptr;
  PyObject* data = PyObject_CallFunction(dumps, const_cast<char*>("(O)"), obj);
  Py_DECREF(dumps);
  return data;
}

// Installs a carrier class in the `__main__` module whose __reduce__ returns the
// given callback tuple. This faithfully reproduces the engine's pickling path,
// where custom objects (FixedArray, Vector3, ...) expose __reduce_ex__ that
// returns (getUnpickleFunc(name), args).
PyObject* make_reduce_carrier(const char* class_name, PyObject* callable)
{
  PyObject* main = PyImport_AddModule("__main__");
  if (!main)
    return nullptr;
  PyObject* md = PyModule_GetDict(main);

  std::string src = std::string("class ") + class_name + ":\n"
      "    def __reduce__(self):\n"
      "        return (_kbe_carrier_callable, (42,))\n";
  // Provide `callable` via a module-level name.
  int set = PyObject_SetAttrString(main, "_kbe_carrier_callable", callable);
  if (set != 0)
    return nullptr;

  PyObject* pySrc = PyUnicode_FromString(src.c_str());
  PyObject* pyCode = Py_CompileString(src.c_str(), "<pickler_carrier>", Py_file_input);
  Py_XDECREF(pySrc);
  if (!pyCode)
    return nullptr;
  PyObject* result = PyEval_EvalCode(pyCode, md, md);
  Py_DECREF(pyCode);
  if (!result)
    return nullptr;
  Py_DECREF(result);
  return PyObject_GetAttrString(main, class_name);
}

class PicklerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (!KBEngine::DebugHelper::isInit())
    {
      KBEngine::DebugHelper::initialize(KBEngine::UNKNOWN_COMPONENT_TYPE);
    }
    if (!Py_IsInitialized())
    {
      Py_Initialize();
    }

    // Debian/Ubuntu ships CPython's C-extension modules (_struct, _pickle, ...)
    // in <stdlib>/lib-dynload, which a bare Py_Initialize() from the build tree
    // leaves off sys.path, so stdlib imports such as pickle fail with
    // "No module named '_struct'".  Locate the directory and prepend it.
    {
      const std::string ver =
          std::to_string(PY_MAJOR_VERSION) + "." + std::to_string(PY_MINOR_VERSION);
      std::vector<std::filesystem::path> candidates;

      // Preferred: sys.path already contains <stdlib> (that is how pickle.py and
      // struct.py are found), so its lib-dynload sibling is the exact directory.
      PyObject* sysPath = PySys_GetObject("path");
      if (sysPath != NULL && PyList_Check(sysPath))
      {
        const Py_ssize_t numPaths = PyList_Size(sysPath);
        for (Py_ssize_t i = 0; i < numPaths; ++i)
        {
          PyObject* entry = PyList_GetItem(sysPath, i);
          if (entry == NULL || !PyUnicode_Check(entry))
            continue;
          const char* entryStr = PyUnicode_AsUTF8(entry);
          if (entryStr == NULL)
          {
            PyErr_Clear();
            continue;
          }
          candidates.emplace_back(std::filesystem::path(entryStr) / "lib-dynload");
        }
      }

      // Fallbacks for build trees where getpath() cannot fully resolve the
      // installed interpreter layout.
      if (wchar_t* prefix = Py_GetPrefix())
      {
        PyObject* prefixObj = PyUnicode_FromWideChar(prefix, -1);
        if (prefixObj != NULL)
        {
          const char* prefixStr = PyUnicode_AsUTF8(prefixObj);
          if (prefixStr != NULL)
          {
            candidates.emplace_back(
                std::filesystem::path(prefixStr) / "lib" / ("python" + ver) / "lib-dynload");
          }
          Py_DECREF(prefixObj);
        }
        else
        {
          PyErr_Clear();
        }
      }
      candidates.emplace_back(std::filesystem::path("/usr/lib/python" + ver + "/lib-dynload"));
      candidates.emplace_back(std::filesystem::path("/usr/local/lib/python" + ver + "/lib-dynload"));

      for (const auto& dynload : candidates)
      {
        if (!std::filesystem::is_directory(dynload))
          continue;
        PyObject* dynloadObj = PyUnicode_FromString(dynload.c_str());
        if (dynloadObj == NULL)
        {
          PyErr_Clear();
          continue;
        }
        if (sysPath != NULL && PyList_Check(sysPath) &&
            PySequence_Contains(sysPath, dynloadObj) == 0)
        {
          PyList_Insert(sysPath, 0, dynloadObj);
        }
        Py_DECREF(dynloadObj);
        break;
      }
    }

    // Temporary diagnostic: dump sys.path when _struct is still unresolvable so
    // the CI log reveals the build-tree interpreter layout.
    PyRun_SimpleString(
        "import sys, importlib.util as _u\n"
        "if _u.find_spec('_struct') is None:\n"
        "    sys.stderr.write('KBE_DIAG_SYSPATH: %r\\n' % sys.path)\n");

    ASSERT_TRUE(KBEngine::script::Pickler::initialize());
  }

  void TearDown() override
  {
    KBEngine::DebugHelper::finalise(true);
  }
};

TEST_F(PicklerTest, AllowsPrimitivePayload)
{
  // A plain dict/list payload must still round-trip through the restricted unpickler.
  PyObject* value = PyDict_New();
  ASSERT_NE(value, nullptr);
  PyObject* key = PyUnicode_FromString("k");
  ASSERT_NE(key, nullptr);
  PyObject* inner = PyList_New(3);
  ASSERT_NE(inner, nullptr);
  PyList_SET_ITEM(inner, 0, PyLong_FromLong(1));
  PyList_SET_ITEM(inner, 1, PyLong_FromLong(2));
  PyList_SET_ITEM(inner, 2, PyLong_FromLong(3));
  PyDict_SetItem(value, key, inner);
  Py_DECREF(key);
  Py_DECREF(inner);

  PyObject* data = pickled_bytes(value);
  ASSERT_NE(data, nullptr);
  PyObject* result = KBEngine::script::Pickler::unpickle(bytes_to_string(data));
  EXPECT_NE(result, nullptr);
  if (result)
  {
    int eq = PyObject_RichCompareBool(result, value, Py_EQ);
    EXPECT_EQ(eq, 1);
    Py_DECREF(result);
  }
  Py_DECREF(value);
  Py_DECREF(data);
}

TEST_F(PicklerTest, AllowsUpfCallbackPayload)
{
  // A payload referencing an engine-registered `_upf` callback must be accepted.
  PyObject* upf = PyImport_AddModule("_upf");
  ASSERT_NE(upf, nullptr);
  static PyMethodDef def = {"TestUpfCallback", test_upf_callback, METH_VARARGS, 0};
  PyObject* cb = PyCFunction_New(&def, nullptr);
  ASSERT_NE(cb, nullptr);
  ASSERT_EQ(PyObject_SetAttrString(upf, "TestUpfCallback", cb), 0);

  // Build an object whose __reduce__ returns (cb, (42,)), exactly like the
  // engine's FixedArray/Vector3 __reduce_ex__ implementations.
  PyObject* carrier_cls = make_reduce_carrier("KbeCarrier", cb);
  ASSERT_NE(carrier_cls, nullptr);
  PyObject* carrier = PyObject_CallNoArgs(carrier_cls);
  ASSERT_NE(carrier, nullptr);
  PyObject* data = pickled_bytes(carrier);
  ASSERT_NE(data, nullptr);

  PyObject* result = KBEngine::script::Pickler::unpickle(bytes_to_string(data));
  EXPECT_NE(result, nullptr);
  if (result)
  {
    // test_upf_callback returns its args tuple (42,).
    EXPECT_TRUE(PyTuple_Check(result));
    Py_DECREF(result);
  }
  Py_DECREF(carrier);
  Py_DECREF(carrier_cls);
  Py_DECREF(data);
}

TEST_F(PicklerTest, BlocksArbitraryGlobal)
{
  // A payload referencing os.system must be rejected (no remote code execution).
  PyObject* os_module = PyImport_ImportModule("os");
  ASSERT_NE(os_module, nullptr);
  PyObject* system = PyObject_GetAttrString(os_module, "system");
  Py_DECREF(os_module);
  ASSERT_NE(system, nullptr);

  PyObject* data = pickled_bytes(system);
  ASSERT_NE(data, nullptr);
  PyObject* result = KBEngine::script::Pickler::unpickle(bytes_to_string(data));
  EXPECT_EQ(result, nullptr);
  PyErr_Clear();
  Py_DECREF(system);
  Py_DECREF(data);
}


TEST_F(PicklerTest, AllowsOrderedDictPayload)
{
  // A payload containing collections.OrderedDict must round-trip through the
  // restricted unpickler (legitimate engine data, e.g. PYTHON properties).
  PyObject* collections_module = PyImport_ImportModule("collections");
  ASSERT_NE(collections_module, nullptr);
  PyObject* ordered_dict = PyObject_GetAttrString(collections_module, "OrderedDict");
  Py_DECREF(collections_module);
  ASSERT_NE(ordered_dict, nullptr);

  PyObject* value = PyObject_CallFunction(ordered_dict, nullptr);
  Py_DECREF(ordered_dict);
  ASSERT_NE(value, nullptr);

  PyObject* data = pickled_bytes(value);
  ASSERT_NE(data, nullptr);
  PyObject* result = KBEngine::script::Pickler::unpickle(bytes_to_string(data));
  EXPECT_NE(result, nullptr);
  if (result)
  {
    int eq = PyObject_RichCompareBool(result, value, Py_EQ);
    EXPECT_EQ(eq, 1);
    Py_DECREF(result);
  }
  Py_DECREF(value);
  Py_DECREF(data);
}

TEST_F(PicklerTest, BlocksUpfDunderEscape)
{
  // A payload referencing a dunder attribute on _upf (e.g. __loader__) must be
  // rejected, not returned as a module internal object.  We inject a raw pickle
  // stream using the GLOBAL opcode (protocol 0) to force find_class('_upf',
  // '__loader__').
  const char raw[] = "c_upf\n__loader__\n.\n";
  PyObject* result = KBEngine::script::Pickler::unpickle(std::string(raw, sizeof(raw) - 1));
  EXPECT_EQ(result, nullptr);
  PyErr_Clear();
}
}  // namespace
