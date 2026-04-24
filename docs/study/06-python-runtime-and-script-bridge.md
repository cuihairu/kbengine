# 6. Python 运行时与脚本桥接

> 两个项目都用 Python 做业务脚本，但集成深度不同。这一章回答：Python 在引擎里处于什么位置？C++ 和 Python 怎么桥接？热重载到底能不能改 .def？为什么不用 Lua？

## 6.1 本章核心问题

- 为什么游戏服务器选 Python 做业务语言？
- BigWorld 和 KBEngine 的 Python 集成有什么差异？
- C++ Entity 怎么变成 Python 可操作的对象？
- 远程方法调用怎么从 Python 表达式变成网络包？

### 6.1.1 先统一版本基线（避免跨章节语义漂移）

这一章如果不先写清 Python 版本，后面很多结论会被误读（语法、标准库、初始化 API、第三方库可用性都会受影响）。

| 项目 | 脚本运行时版本基线 | 依据 |
|------|--------------------|------|
| BigWorld 14.4.1 | **Python 2.7.7（内嵌源码）** | `BigWorld-Engine-14.4.1/programming/bigworld/third_party/python/Include/patchlevel.h` 中 `PY_VERSION = "2.7.7"` |
| KBEngine（当前仓库） | **Python 3（由构建环境提供，不固定小版本）** | CMake 通过 `find_package(Python3 REQUIRED COMPONENTS Development)`，并链接 `Python3::Python` |

对 KBEngine 再补一条关键实现细节：

- `kbe/src/lib/pyscript/script.cpp` 对 **`PY_VERSION_HEX >= 0x030B0000`（Python 3.11+）**走 `PyConfig + Py_InitializeFromConfig` 路径
- 低于该阈值走兼容初始化路径（仍是 Python 3 API 体系）

这意味着：

1. BigWorld 的脚本示例默认按 Python 2.7 语义理解（例如旧式字符串/部分标准库行为）
2. 当前 KBEngine 文档与示例应优先按 Python 3 语义编写
3. 涉及第三方库（如 Twisted）时，必须结合对应 Python 大版本讨论可用性
- 热重载的边界在哪里？

## 6.2 为什么选 Python 而不是 Lua

这不是一个"谁更快"的问题，而是一个工程决策。

### 脚本层不是性能瓶颈

- MMO 服务器热路径全在 C++：网络 I/O、AOI、序列化、寻路
- Python 脚本主要承接实体行为、入口回调和业务编排；常见更新频率由 `gameUpdateHertz` 控制，默认配置通常是双位数 Hz 量级
- 即使 LuaJIT 比 CPython 快 30-50x，从 2ms 优化到 0.1ms 在 100ms 预算里无意义
- 真正的性能约束：网络带宽、AOI 计算量、DB 写入延迟——都不是脚本层的事

### Python 的表现力是生产力乘数

MMO 脚本是几十万行业务代码，不是几百行嵌入式脚本：

| 能力 | Python | Lua |
|------|--------|-----|
| 内置 class | ✓ | ✗（需手写元表） |
| 装饰器 | ✓ | ✗ |
| 生成器 / with | ✓ | ✗ |
| 多重继承 | ✓ | 元表模拟 |
| 标准库 | json/xml/re/logging/unittest/hashlib 全内置 | 极小，全需第三方 |

### 其他因素

- **Twisted Deferred**：BigWorld 的 PyDeferred 直接基于 Twisted，提供异步回调链式处理。Lua 没有 Twisted
- **热重载**：CPython 内置 `PyImport_ReloadModule`。Lua 的 `require` 缓存模块，热重载需手动清 `package.loaded`
- **团队门槛**：MMO 团队是"少数 C++ 引擎程序员 + 多数脚本程序员 + 策划"，Python 可读性更高
- **历史时机**：BigWorld 架构设计于 2002-2004 年，Python 2.x 已成熟，LuaJIT 还不存在

**一句话：Lua 是给 C++ 程序员嵌进去写小脚本的，Python 是给团队写大工程的。** BigWorld/KBEngine 的脚本层代码量是工程级别，不是脚本级别。

## 6.3 Python 在两套项目里的核心位置

```
┌──────────────────────────────────────────────────┐
│                 Python 脚本层                      │
│   实体行为（onTick / onEnterSpace / onWitness）    │
│   业务逻辑（背包 / 任务 / 社交 / 战斗结算）         │
│   入口钩子（onInit / onBaseAppReady / onReadyForLogin）│
├──────────────────────────────────────────────────┤
│              C++ / Python 桥接层                   │
│   PyTypeObject / tp_call / PyObject 持有           │
│   属性拦截（setattr → C++ 类型检查 → 脏标记）       │
│   远程方法（tp_call → MemoryStream → Bundle → 网络）│
├──────────────────────────────────────────────────┤
│              C++ 引擎层                            │
│   EntityDef / Network / AOI / DB / Space           │
└──────────────────────────────────────────────────┘
```

EntityDef 的 .def 文件描述结构，Python 文件定义行为。脚本层是引擎和游戏业务之间**唯一的扩展接口**。

这里要特别澄清一个常见误解：Python 文件不是由 `.def` 自动生成的。KBEngine 的实际做法是先解析 `entities.xml + .def` 建立 `ScriptDefModule`，然后在 `loadAllEntityScriptModules()` 里按实体名 `PyImport_ImportModule(moduleName)` 去加载脚本模块，再要求模块内存在同名类。

## 6.4 继承链的差异

### KBEngine：两支继承链

```
ServerApp
  └── EntityApp<Entity>    ← Python 初始化在这里（installPyScript）
        ├── Baseapp
        └── Cellapp
  └── PythonApp            ← 也有 Python 初始化
        ├── Loginapp
        ├── Dbmgr
        ├── Logger
        └── Interfaces
ClientApp
  └── Bots
```

KBEngine 把 Python 运行时初始化合并进了 `EntityApp` 和 `PythonApp`。

```cpp
// 文件：kbe/src/lib/server/entity_app.h（简化）
template<class E>
class EntityApp : public ServerApp
{
    bool inInitialize() override
    {
        installPyScript();     // Python 解释器初始化
        installPyModules();    // KBEngine 模块注册
        installEntityDef();    // 加载 .def 实体定义
    }
};
```

因此“定义”和“脚本”的职责边界是：

- `.def`：手写，描述属性、方法、持久化、组件关系
- Python 模块：手写，描述实体行为
- 引擎不会替你生成 `Avatar.py`、`Monster.py` 这种业务脚本，只会在启动时校验它们是否存在且是否与 `.def` 对齐

### BigWorld：EntityApp 之上再加 ScriptApp

```
ServerApp
  └── EntityApp              ← 实体运行骨架
        └── ScriptApp        ← 多了一层：专门处理 Python 运行时
              ├── BaseApp
              ├── CellApp
              └── DBApp
```

BigWorld 把 Python 运行时抽成了独立的 `ScriptApp` 层：

```cpp
// 文件：programming/bigworld/lib/server/script_app.cpp（简化）
bool ScriptApp::initScript(const char* componentName,
        const char* scriptPath1, const char* scriptPath2)
{
    PyImportPaths paths;
    paths.addResPath(scriptPath1);
    paths.addResPath(scriptPath2);
    paths.addResPath(EntityDef::Constants::serverCommonPath());

    Script::init(paths, componentName);   // Python 初始化

    ScriptModule bigWorld = ScriptModule::getOrCreate("BigWorld");
    bigWorld.setAttribute("serverMode", ...);

    PyDeferred::staticInit();  // Twisted Deferred 初始化（BigWorld 独有）
}
```

ScriptApp 多做了什么：
- Python 路径初始化
- 脚本模块注册
- **PyDeferred 初始化**（Twisted Deferred 模式）
- **Personality 模块**（`onInit` / `onFini` 脚本回调）
- 脚本定时器系统

## 6.5 C++ 与 Python 的桥接：Entity 就是 PyObject

两个项目的核心设计相同：**C++ Entity 对象本身就是 PyObject**。

### KBEngine：ScriptObject 继承链

```cpp
// 文件：kbe/src/lib/pyscript/scriptobject.h（简化）
// BASE_SCRIPT_HREADER 宏展开后：
class Entity : public script::ScriptObject   // ScriptObject 继承自 PyObject
{
    static void _tp_dealloc(PyObject* self) {
        static_cast<Entity*>(self)->~Entity();
        Entity::_scriptType.tp_free(self);
    }
    static PyObject* _tp_getattro(PyObject* self, PyObject* name) {
        return static_cast<Entity*>(self)->onScriptGetAttribute(...);
    }
    static int _tp_setattro(PyObject* self, PyObject* name, PyObject* value) {
        return static_cast<Entity*>(self)->onScriptSetAttribute(...);
    }
    static PyTypeObject _scriptType;
    // ...
};
```

### BigWorld：PyObjectPlus 继承链

```cpp
// 文件：programming/bigworld/lib/pyscript/pyobject_plus.hpp（简化）
// Py_Header 宏展开后：
class Base : public PyObjectPlus   // PyObjectPlus 继承自 PyObject
{
    static void _tp_dealloc(PyObject* pObj) {
        static_cast<Base*>(pObj)->pyDel();
        delete static_cast<Base*>(pObj);
    }
    static PyObject* _tp_getattro(PyObject* pObj, PyObject* name) {
        return static_cast<Base*>(pObj)->pyGetAttribute(...);
    }
    static int _tp_setattro(PyObject* pObj, PyObject* name, PyObject* value) {
        return static_cast<Base*>(pObj)->pySetAttribute(...);
    }
    static PyTypeObject s_type_;
};
```

**核心原理**：Entity/Base 的内存布局以 `PyObject` 头部开始，C++ 对象和 Python 对象是**同一块内存**的两个视角。Python 层看到的是脚本类实例，C++ 层看到的是 Entity 对象。

## 6.6 统一构造：Placement New 模式

创建实体的关键步骤——先用 Python 类型分配内存，再用 placement new 构造 C++ 对象：

### BigWorld EntityType::newEntityBase

```cpp
// 文件：programming/bigworld/server/baseapp/entity_type.cpp:547（简化）
Base* EntityType::newEntityBase(EntityID id, DatabaseID dbID)
{
    // 1. 用 Python 类型的 tp_alloc 分配内存
    PyObject* pObject = PyType_GenericAlloc(pClass_, 0);

    // 静态断言：确保 Base/Proxy 没有虚函数（否则 vptr 会破坏内存布局）
    BW_STATIC_ASSERT(std::tr1::is_polymorphic<Base>::value == false);
    BW_STATIC_ASSERT(std::tr1::is_polymorphic<Proxy>::value == false);

    // 2. Placement new：在 Python 分配的内存上构造 C++ 对象
    if (this->isProxy())
        pNewBase = new(pObject) Proxy(id, dbID, this);
    else
        pNewBase = new(pObject) Base(id, dbID, this);

    return pNewBase;
}
```

### KBEngine ScriptDefModule::createObject

```cpp
// 文件：kbe/src/lib/entitydef/scriptdef_module.cpp:283
PyObject* ScriptDefModule::createObject(void)
{
    PyObject* pObject = PyType_GenericAlloc(scriptType_, 0);
    // scriptType_ 是从 Python 脚本导入的实体类（如 Avatar.py 定义的类）
    return pObject;
}
```

然后在 `EntityApp::onCreateEntity` 中用类似的 placement new 模式构造 Entity。

这里要把两边分开说：

- BigWorld 在 `entity_type.cpp` 里明确用 `BW_STATIC_ASSERT` 检查 `Base/Proxy` 不是多态类型。
- KBEngine 的源码里我没有找到对应的静态断言，但它同样依赖“先 `PyType_GenericAlloc`，再 placement new 构造 C++ 外壳”这套内存模型；`BASE_SCRIPT_HREADER` 里的 `_tp_dealloc` 也明确按这种对象布局回收。

所以更稳妥的结论是：两边都依赖同一种对象布局约束，但只有 BigWorld 在这里把“无多态”写成了显式静态断言。

## 6.7 属性拦截：setattr 触发 C++ 逻辑

当 Python 脚本执行 `entity.health = 100` 时，实际经过 C++ 拦截：

```
Python: entity.health = 100
  │
  ▼
_tp_setattro(entity, "health", 100)
  │
  ▼
Entity::onScriptSetAttribute("health", 100)
  │
  ├── 查找 PropertyDescription
  ├── DataType::isSameType(value) 做类型检查
  ├── PropertyDescription::onSetValue(...) 写入实际值
  └── onDefDataChanged(...) 把“属性已变化”继续传给后续链路
```

KBEngine 的 `onScriptSetAttribute` 做的事：

1. 在 `pPropertyDescrs_` 中查找属性名
2. 用 `DataType::isSameType()` 检查值类型
3. 调 `propertyDescription->onSetValue(this, value)` 真正写入
4. 如果成功，再通过 `onDefDataChanged(...)` 把变化向同步 / 持久化链路传播

也就是说，`onScriptSetAttribute` 本身更像“入口拦截器 + 类型闸门”，脏标记、广播粒度和是否写库，不是都直接硬编码在这一个函数里。

## 6.8 远程方法调用：tp_call 变成网络包

当 Python 脚本执行 `entity.cell.onDamage(100)` 时：

```
Python: entity.cell.onDamage(100)
  │
  ▼
RemoteEntityMethod::tp_call(self, args=(100,))
  │
  ├── MethodDescription::checkArgs(args)    ← 类型检查
  ├── MethodDescription::addToStream(args)  ← 序列化到 MemoryStream
  ├── EntityCall::newCall(bundle)           ← 构造网络 Bundle
  ├── Bundle::append(stream)                ← 追加参数数据
  └── EntityCall::sendCall(bundle)          ← 通过网络发送
```

```cpp
// 文件：kbe/src/lib/entitydef/remote_entity_method.cpp:45（简化）
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args, PyObject* kwds)
{
    RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
    MethodDescription* methodDesc = rmethod->getDescription();
    EntityCallAbstract* entityCall = rmethod->getEntityCall();

    if (methodDesc->checkArgs(args))
    {
        MemoryStream mstream;
        methodDesc->addToStream(mstream, args);   // 序列化参数

        Network::Bundle* pSendBundle = Network::Bundle::create();
        entityCall->newCall(*pSendBundle);         // 构造消息头
        pSendBundle->append(mstream.data(), mstream.wpos());  // 追加负载
        entityCall->sendCall(pSendBundle);         // 发送
    }
    return Py_None;
}
```

**一次 Python 函数调用 = 一次跨进程网络消息**。`tp_call` 是这个转换的核心。

## 6.9 Twisted Deferred vs CallbackMgr

### 先把概念说清楚：Deferred 到底是什么

Deferred 不是线程，不是协程，它更像 JavaScript 的 Promise：

- 一个“未来结果容器”
- 一个“回调链调度器”

最小心智模型：

```text
PENDING（等待中）
   ├── callback(result)  -> SUCCESS 分支，按 addCallback 链执行
   └── errback(error)    -> FAILURE 分支，按 addErrback 链执行
```

也就是说，调用方现在拿不到结果，只先拿到一个 Deferred；结果未来到达时，由 Deferred 统一触发对应分支。

### BigWorld 的 PyDeferred 是怎么接上 Twisted 的

```cpp
// 文件：programming/bigworld/lib/entitydef/py_deferred.hpp（简化）
class PyDeferred
{
    // 初始化：导入 twisted.internet.defer.Deferred
    static bool staticInit()
    {
        PyObject* pModule = PyImport_ImportModule("twisted.internet.defer");
        s_classDeferred = PyObject_GetAttrString(pModule, "Deferred");
    }

    // 构造：创建一个 Deferred 实例
    PyDeferred() :
        pObject_(PyObject_CallFunctionObjArgs(s_classDeferred.get(), NULL))
    {}

    void addCallback(PyObject* callback);
    void addErrback(PyObject* errback);
    void callback(PyObject* result);
    void errback(PyObject* error);
};
```

BigWorld 的 TwoWay RPC/异步查询本质是：

1. 脚本发起调用，先返回 `Deferred`
2. 脚本注册 `addCallback/addErrback`
3. C++ 收到远端回复后，触发 `callback(...)` 或 `errback(...)`
4. Deferred 执行整条回调链

示意代码：

```python
# BigWorld 脚本层（示意）
d = self.base.someMethod.twoWay(arg)
d.addCallback(self.onSuccess)
d.addErrback(self.onError)
```

### 为什么 Deferred 会比“单个回调”更强

Deferred 的关键价值不是“能回调”，而是“可组合”：

- 可以串联多个异步步骤：`A -> B -> C`
- 可以把错误当作一等流程处理（errback 链）
- 一个回调返回新 Deferred 时，后续链会等待它完成再继续（扁平化异步嵌套）

如果只用普通 callback，很容易退化成“回调套回调”。

### KBEngine CallbackMgr（对照）

```cpp
// 文件：kbe/src/lib/server/callbackmgr.h（简化）
// 发起时：save(callbackID -> pyCallback)
// 回来时：take(callbackID) 并执行

int callbackID = callbackMgr_.save(pyCallback);
// ... 发送请求 ...
PyObject* pyCallback = callbackMgr_.take(callbackID);
PyObject_CallFunction(pyCallback, ...);
```

CallbackMgr 是“ID 到函数”的映射表，优点是直观、低心智负担；缺点是缺少 Deferred 那种链式组合和统一错误通道。

### 优缺点对比（你阅读时最该抓这张表）

| 维度 | Twisted Deferred（BigWorld） | CallbackMgr（KBEngine） |
|------|------------------------------|-------------------------|
| 编程模型 | 链式异步原语（类似 Promise） | 一次性回调登记 |
| 异步组合 | 强（可串联、可扁平化） | 弱（需手工拼装） |
| 错误处理 | 内建 errback 链 | 业务层自行分支处理 |
| 可读性（简单场景） | 一般 | 好 |
| 可读性（复杂场景） | 好（结构化） | 易碎（回调分散） |
| 学习成本 | 高（状态机/链语义） | 低 |
| 调试复杂度 | 中到高 | 低到中 |

### KBEngine CallbackMgr 的优缺点（单独看）

优点：

- **实现简单**：`save/take` 两步就能跑通异步回调，代码路径短
- **运行时开销低**：不需要维护 Deferred 状态机和回调链调度
- **定位直接**：按 `callbackID` 查链路，问题面比较收敛
- **上手成本低**：脚本开发者不必理解 Twisted 的语义细节

缺点：

- **组合能力弱**：多阶段异步流程需要手工编排，容易出现“回调地狱”
- **错误通道不统一**：成功/失败常靠业务层约定，规范容易漂移
- **生命周期管理更脆弱**：超时、重复回调、丢回调等边界要自行兜底
- **可维护性随复杂度下降**：流程一长，回调散落在多处，重构成本上升

一个实用判断标准：

- “一次请求一次响应”的短链路，用 CallbackMgr 性价比高
- “多步编排 + 失败恢复 + 重试”的长链路，建议抽象成更结构化的异步模型

### 实践建议

- 调用链短、场景简单：CallbackMgr 够用，维护成本更低
- 调用链长、有失败恢复/重试编排：Deferred 更合适

这也是两套引擎的典型取舍：BigWorld 追求异步表达力，KBEngine 追求实现和使用成本更低。

## 6.10 热重载机制与边界

### KBEngine 热重载

```cpp
// 文件：kbe/src/server/baseapp/baseapp.cpp（简化）
void Baseapp::reloadScript(bool fullReload)
{
    // 遍历所有 Entity
    Entities<Entity>::ENTITYS_MAP& entities = pEntities_->getEntities();
    for (auto& eiter : entities)
    {
        eiter->second->reload(fullReload);  // 切换 Python 类
    }
    // 调用入口脚本的 onInit(1)（1 表示重载）
    PyObject_CallMethod(getEntryScript().get(), "onInit", "i", 1);
}
```

Entity::reload 的核心操作：

```cpp
// 通过替换 __class__ 属性实现类切换
PyObject_SetAttrString(this, "__class__", newClass);
```

### BigWorld 热重载

BigWorld 通过 `ScriptApp::triggerOnInit(isReload=true)` 触发，调用 `BWPersonality.onInit(True)`。

`EntityType::reloadScript()` 重新导入 Python 类并更新 `pClass_`。

### 热重载的边界

| 能热更 | 不能热更 |
|--------|---------|
| 实体行为逻辑（Python 方法体） | EntityDef 定义（.def 文件的属性/方法签名） |
| 全局脚本函数 | 已存在的实体对象的 C++ 侧状态 |
| 脚本模块的顶层逻辑 | 网络协议中的 utype 分配 |
| | 数据库表结构 |

**.def 文件变更需要重启**。因为 .def 决定了三件事：Python 属性、网络协议 ID、数据库表列。运行时改 .def 意味着要同时改所有在线实体的内存布局、协议编解码、数据库结构——代价太大。

### 灰区

- **全局变量的重置语义**：热更后模块级全局变量会被重新初始化，已有的引用不受影响
- **已创建的实体**：旧实例的 `__class__` 被替换为新类，但 `__dict__` 保持不变

## 6.11 如何把一个 C++ 方法暴露成 `KBEngine.xxx()`

前面几节已经讲清楚了“`KBEngine` 模块是引擎在启动时由 C++ 注册给 Python 的”，但如果你现在真的要扩展引擎，最常见的问题其实是：

- 我想新增一个 `KBEngine.xxx()`，应该改哪里？
- 这个方法应该放公共层，还是放 `baseapp/cellapp` 专属层？
- Python 里 `import KBEngine` 之后，为什么能直接调到 C++？

这一节把完整实现步骤讲透。

### 先区分三类扩展，不要混写

第一类：模块函数

- 形态是 `KBEngine.time()`、`KBEngine.createEntity()`、`KBEngine.executeRawDatabaseCommand()`
- 本质是给 `KBEngine` 这个 Python 模块挂一个 `PyCFunction`

第二类：脚本类型方法 / 属性

- 形态是 `entity.xxx()`、`proxy.client`、`Entity.id`
- 本质是给 `Entity/Proxy/Space/...` 这些 Python 类型注册脚本接口

第三类：纯 Python 业务脚本

- 形态是 `scripts/base/*.py`、`scripts/cell/*.py`、`scripts/common/*.py`
- 不需要改 C++ 注册表

你这次问的是第一类，也就是“新增一个 `KBEngine.xxx()` 模块函数”。

### 注册链路长什么样

最短的真实链路是：

```text
Python: import KBEngine
  │
  ▼
installPyModules()
  │
  ▼
APPEND_SCRIPT_MODULE_METHOD(module, xxx, __py_xxx, ...)
  │
  ▼
PyModule_AddObject(module, "xxx", PyCFunction_New(...))
  │
  ▼
Python: KBEngine.xxx(...)
  │
  ▼
C++: __py_xxx(PyObject* self, PyObject* args)
```

注册宏本身就在：

- `kbe/src/lib/pyscript/py_macros.h`

```cpp
#define APPEND_SCRIPT_MODULE_METHOD(MODULE, NAME, FUNC, FLAGS, SELF) \
    static PyMethodDef __pymethod_##NAME = {#NAME, (PyCFunction) FUNC, FLAGS, NULL}; \
    if(PyModule_AddObject(MODULE, #NAME, PyCFunction_New(&__pymethod_##NAME, SELF)) != 0) \
    { \
        SCRIPT_ERROR_CHECK(); \
        ERROR_MSG("append " #NAME " to pyscript error!\n"); \
    }
```

也就是说，`KBEngine.xxx` 并不是“Python 自动找到一个同名 `.py` 函数”，而是启动时由 C++ 把一个函数指针塞进了 Python 模块对象。

### 先决定这个方法属于哪一层

这一步最关键，决定了你应该把方法注册到哪里。

#### 情况一：所有服务端进程都需要

例如：

- `KBEngine.addTimer`
- `KBEngine.delTimer`
- `KBEngine.open`
- `KBEngine.listPathRes`

这类放在：

- `kbe/src/lib/server/python_app.cpp`

因为 `PythonApp::installPyModules()` 是 `loginapp/dbmgr/logger/interfaces` 等公共服务进程共享的模块初始化入口。

#### 情况二：所有实体型服务进程都需要

例如：

- `KBEngine.globalData`
- `KBEngine.entities`
- `KBEngine.getWatcher`

这类放在：

- `kbe/src/lib/server/entity_app.h`

因为 `EntityApp<E>::installPyModules()` 是 `baseapp/cellapp` 共享的实体型脚本入口。

#### 情况三：只给某个组件用

例如：

- `KBEngine.createEntity()`、`KBEngine.charge()` 只在 `baseapp`
- `KBEngine.raycast()`、`KBEngine.addSpaceGeometryMapping()` 只在 `cellapp`

这类分别放在：

- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/cellapp/cellapp.cpp`

一个直接例子：

```cpp
// kbe/src/server/baseapp/baseapp.cpp
APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),
    time, __py_gametime, METH_VARARGS, 0);
```

Python 里就能写：

```python
import KBEngine

now = KBEngine.time()
```

### 一个最小可运行例子：新增 `KBEngine.echo()`

下面用 `baseapp` 侧举例，做一个最小但完整的 API。

目标：

```python
import KBEngine
print(KBEngine.echo("hello"))
```

输出仍然是 `"hello"`。

#### 第一步：在头文件声明包装函数

文件：

- `kbe/src/server/baseapp/baseapp.h`

新增声明：

```cpp
static PyObject* __py_echo(PyObject* self, PyObject* args);
```

这类函数的签名基本固定：

- `self` 对模块函数通常没业务意义
- `args` 是 Python 传进来的参数元组
- 返回值必须是 `PyObject*`
- 出错时返回 `NULL`，让 Python 看到异常

#### 第二步：在模块初始化里注册这个名字

文件：

- `kbe/src/server/baseapp/baseapp.cpp`

在 `Baseapp::installPyModules()` 里增加：

```cpp
APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),
    echo, __py_echo, METH_VARARGS, 0);
```

到这里，模块名 `KBEngine.echo` 已经建立好了，但真正逻辑还没实现。

#### 第三步：实现包装函数

还是在：

- `kbe/src/server/baseapp/baseapp.cpp`

实现：

```cpp
PyObject* Baseapp::__py_echo(PyObject* self, PyObject* args)
{
    const char* text = nullptr;

    if (!PyArg_ParseTuple(args, "s", &text))
        return NULL;

    return PyUnicode_FromString(text);
}
```

这里有三个关键点：

1. 用 `PyArg_ParseTuple()` 解析 Python 参数。
2. 参数解析失败时直接 `return NULL`，由 Python C API 继续抛异常。
3. 返回值要构造成新的 Python 对象，比如 `PyLong_FromLong`、`PyUnicode_FromString`、`PyBool_FromLong`。

#### 第四步：在 Python 里调用验证

例如：

```python
import KBEngine

def onBaseAppReady(isBootstrap):
    print(KBEngine.echo("hello"))
```

如果注册和实现都没问题，脚本层就能直接调用到这个 C++ 方法。

### 一个更贴近 KBEngine 风格的例子：包装层只做参数转换

实际工程里，不建议把复杂逻辑都写进 `__py_xxx`。

更好的结构是：

```cpp
PyObject* Baseapp::__py_echo(PyObject* self, PyObject* args)
{
    const char* text = nullptr;

    if (!PyArg_ParseTuple(args, "s", &text))
        return NULL;

    std::string result = Baseapp::getSingleton().echo(text);
    return PyUnicode_FromString(result.c_str());
}
```

再把真正逻辑放进普通成员函数：

```cpp
std::string Baseapp::echo(const std::string& text)
{
    return text;
}
```

这样分层的好处是：

- `__py_xxx` 只负责 Python/C++ 边界
- 业务逻辑可单测、可复用
- 包装层不会堆成一团难以维护的 `PyObject*` 操作

### 常见返回值怎么构造

新增模块函数时，经常会用到这些返回方式：

```cpp
return PyLong_FromLong(123);                    // int
return PyLong_FromUnsignedLong(v);             // unsigned int
return PyLong_FromUnsignedLongLong(v);         // uint64
return PyUnicode_FromString("ok");             // str
Py_RETURN_TRUE;                                // bool True
Py_RETURN_FALSE;                               // bool False
Py_RETURN_NONE;                                // None
```

如果要返回 tuple / list / dict，就要手工创建 Python 对象并填充。

### 参数解析时最容易踩的坑

#### 坑一：只注册了函数名，忘了实现

这种会在链接期或运行期暴露，取决于声明和定义是否完整。

#### 坑二：函数注册到了错误的组件

例如你把 `baseapp` 专属接口注册到 `PythonApp::installPyModules()`，那 `loginapp/dbmgr` 也会看到这个名字，但底层语义可能根本不成立。

#### 坑三：包装函数里直接塞太多业务

这会让：

- 参数解析
- 错误处理
- 引用计数
- 真实业务逻辑

全部混在一起，后期很难维护。

#### 坑四：返回裸 C++ 值，没转成 `PyObject*`

Python 只认识 `PyObject*`。C++ 基本类型必须显式封装。

#### 坑五：没想清楚进程作用域

文档里最容易被忽略的一点是：**同样叫 `import KBEngine`，不同进程看到的模块接口并不完全相同。**

例如：

- `baseapp` 里有 `KBEngine.createEntity`
- `cellapp` 里有 `KBEngine.raycast`
- `dbmgr` 里不一定有这两个

所以新增接口前，先问一句：

- “这个 API 应该在哪些组件可见？”

### 如果不是模块函数，而是想给 `Entity/Proxy` 加方法呢

那就不是这一节的 `APPEND_SCRIPT_MODULE_METHOD` 了。

你要改的是脚本类型注册链，也就是：

- `Entity::installScript(...)`
- `Proxy::installScript(...)`
- `SCRIPT_METHOD_DECLARE(...)`
- `SCRIPT_GETSET_DECLARE(...)`

换句话说：

- `KBEngine.xxx()` 是“模块级 API”
- `entity.xxx()` / `proxy.xxx()` 是“脚本对象类型 API”

两套机制不要混。

### 实战时最稳妥的步骤

如果你准备真正新增一个接口，建议按这个顺序：

1. 先搜仓库里最接近的现成接口，例如 `KBEngine.time()` 或 `KBEngine.address()`
2. 确认它属于公共层、实体层还是组件专属层
3. 抄同层已有接口的声明、注册、实现骨架
4. 让 `__py_xxx` 只做参数解析和返回值封装
5. 把核心逻辑放到普通 C++ 函数
6. 最后再补 Python 文档和类型桩

最接近的对照入口有：

- `kbe/src/lib/server/python_app.cpp`
- `kbe/src/lib/server/entity_app.h`
- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/cellapp/cellapp.cpp`

## 6.12 关键源码入口

### KBEngine

| 概念 | 文件 | 关键类/方法 |
|------|------|------------|
| Python 初始化 | `kbe/src/lib/pyscript/script.cpp` | `Script::install()` |
| ScriptObject | `kbe/src/lib/pyscript/scriptobject.h` | `BASE_SCRIPT_HREADER` 宏 |
| PythonApp | `kbe/src/lib/server/python_app.cpp` | `installPyScript()` / `installPyModules()` |
| CallbackMgr | `kbe/src/lib/server/callbackmgr.h` | `save()` / `take()` |
| ScriptTimers | `kbe/src/lib/server/script_timers.h` | `addTimer()` / `delTimer()` |
| 远程方法 | `kbe/src/lib/entitydef/remote_entity_method.cpp` | `tp_call()` |
| 热重载 | `kbe/src/server/baseapp/baseapp.cpp` | `reloadScript()` |
| 模块函数注册 | `kbe/src/lib/pyscript/py_macros.h` | `APPEND_SCRIPT_MODULE_METHOD` |
| 公共模块入口 | `kbe/src/lib/server/python_app.cpp` | `installPyModules()` |
| 实体型模块入口 | `kbe/src/lib/server/entity_app.h` | `EntityApp<E>::installPyModules()` |

### BigWorld

| 概念 | 文件 | 关键类/方法 |
|------|------|------------|
| Script 初始化 | `lib/pyscript/script.cpp` | `Script::init()` |
| PyObjectPlus | `lib/pyscript/pyobject_plus.hpp` | `Py_Header` 宏 |
| ScriptApp | `lib/server/script_app.cpp` | `initScript()` |
| PyDeferred | `lib/entitydef/py_deferred.hpp` | `staticInit()` / `addCallback()` |
| PythonServer | `lib/server/python_server.cpp` | Telnet 调试服务 |
| Personality | `lib/pyscript/personality.cpp` | `callOnInit()` |
| 实体创建 | `server/baseapp/entity_type.cpp` | `newEntityBase()` |

## 6.13 源码走读路径

### 路径一：跟踪 Python 初始化

1. KBEngine: `kbe/src/lib/pyscript/script.cpp` — `install()` → `Py_Initialize()`
2. KBEngine: `kbe/src/lib/server/python_app.cpp` — `installPyScript()` 构建 Python 路径
3. BigWorld: `lib/server/script_app.cpp` — `initScript()` → `Script::init()` + `PyDeferred::staticInit()`

### 路径二：理解 Entity 与 PyObject 的统一构造

1. BigWorld: `server/baseapp/entity_type.cpp:547` — `newEntityBase()` 的 placement new
2. BigWorld: `server/baseapp/base.cpp:608` — 构造函数传 `pType->pClass()` 给 PyObjectPlus
3. KBEngine: `kbe/src/lib/entitydef/scriptdef_module.cpp:283` — `createObject()`

### 路径三：理解远程方法调用的桥接

1. KBEngine: `kbe/src/lib/entitydef/remote_entity_method.cpp` — `tp_call()` 的完整链路
2. 对比 BigWorld: MailBox 的 `PY_TYPEOBJECT_WITH_CALL` 实现

### 路径四：理解热重载

1. KBEngine: `kbe/src/server/baseapp/baseapp.cpp` — `reloadScript()` → Entity::reload()
2. BigWorld: `lib/server/script_app.cpp` — `triggerOnInit(isReload=true)`

## 6.14 小结

- **Python 被选中是因为工程生产力，不是运行速度**——标准库、表现力、热重载、团队门槛
- **Entity 就是 PyObject**：C++ 对象和 Python 对象共享同一块内存，用 placement new 统一构造
- **BigWorld 多了 ScriptApp 层**和 **PyDeferred（Twisted Deferred）**——更完整的异步编程能力
- **KBEngine 用 CallbackMgr 替代 Deferred**——更简单直接，但牺牲了组合能力
- **远程方法调用的本质**：`tp_call` 把 Python 函数调用序列化成网络包
- **热重载只改行为不改结构**：Python 方法体可以热更，.def 文件变更必须重启
- **无虚函数约束**：Entity/Base/Proxy 不能有虚函数，否则 vptr 会破坏 PyObject 内存布局
