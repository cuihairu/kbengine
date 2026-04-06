# 脚本运行时与热重载

> 这一页只回答三个问题：KBEngine 的 Python 运行时是谁装起来的，实体脚本对象是怎么从 `EntityDef` 变成运行中实例的，`reloadScript` 到底会重载什么、不会重载什么。

## 先给结论

KBEngine 的脚本运行时不是“启动时顺手把 Python 嵌进去”这么简单，而是分成了两条骨架：

- `PythonApp` 负责非实体型组件的脚本宿主，例如 `dbmgr`、`interfaces`、`loginapp`、`logger`
- `EntityApp<E>` 在脚本宿主之上再叠一层实体运行时，负责 `Baseapp`、`Cellapp` 的实体脚本、`EntityDef`、`callbackMgr` 和 `entities`

对应源码入口：

- `kbe/src/lib/server/python_app.h`
- `kbe/src/lib/server/python_app.cpp`
- `kbe/src/lib/server/entity_app.h`
- `kbe/src/lib/entitydef/entitydef.cpp`
- `kbe/src/lib/entitydef/scriptdef_module.cpp`

如果只记一条主线，可以记成：

```text
installPyScript
  → 建 Python 搜索路径并安装 KBEngine 模块
installPyModules
  → 把 C++ 暴露给 Python，导入入口脚本
installEntityDef
  → 读取 entities.xml / *.def，建立 ScriptDefModule
createEntity
  → ScriptDefModule::createObject 分配 PyObject
  → onCreateEntity 构造 C++ Entity 外壳
  → initializeEntity(dict) 建属性命名空间并执行脚本初始化
reloadScript
  → EntityDef 重新加载定义
  → EntityCall / EntityComponent / 已存活实体切到新类
  → 再次调用入口脚本 onInit(1)
```

## 第一层：`PythonApp` 负责把 Python 宿主装起来

`PythonApp::inInitialize()` 的顺序很直接：

```text
installPyScript()
  → installPyModules()
```

这条链定义在 `kbe/src/lib/server/python_app.cpp`。它说明 KBEngine 先搭“解释器和模块环境”，再加载业务入口脚本。

### `installPyScript()` 做的是路径和解释器安装

`PythonApp::installPyScript()` 会先检查 `Resmgr` 是否已经准备好以下目录：

- 用户资源目录
- 系统资源目录
- 用户脚本目录

然后根据组件类型拼出不同的 `sys.path` 风格路径：

- 所有组件都会先加 `common`、`data`、`user_type`
- `baseapp` 再加 `server_common`、`base`、`base/interfaces`、`base/components`
- `cellapp` 再加 `server_common`、`cell`、`cell/interfaces`、`cell/components`
- `dbmgr`、`interfaces`、`loginapp`、`logger` 各自再加自己的脚本子目录

最后它调用：

```cpp
getScript().install(..., "KBEngine", componentType_)
```

也就是说，`KBEngine` 这个 Python 模块并不是纯脚本包，而是 C++ 侧创建的宿主模块。`PythonApp` 成功安装后还会顺手安装 `PyMemoryStream`，这样脚本层才能直接操作引擎的流对象。

### `installPyModules()` 做的是“往 KBEngine 模块里塞能力”

`PythonApp::installPyModules()` 的职责不是扫描实体，而是给所有 Python 组件统一挂载基础能力：

- `MemoryStream`
- `publish`
- `scriptLogType`
- `getResFullPath`
- `hasRes`
- `open`
- `listPathRes`
- `matchPath`
- `addTimer`
- `delTimer`
- 文件描述符注册接口

然后按组件配置取 `entryScriptFile`，最后通过 `PyImport_Import(...)` 导入入口模块。

这里有一个很重要的边界：`PythonApp` 导入的是“组件入口脚本”，不是实体类定义本身。也就是说，`interfaces`、`dbmgr` 这一类组件的脚本生命周期，核心是入口模块，而不是 `EntityDef`。

## 第二层：`EntityApp<E>` 在宿主上叠实体世界

`EntityApp<E>::inInitialize()` 比 `PythonApp` 多一步：

```text
installPyScript()
  → installPyModules()
  → installEntityDef()
```

这意味着 BaseApp / CellApp 的脚本运行时不是单纯“能 import Python 文件”就结束了，它还必须把实体定义系统建起来。

### `installPyModules()` 先暴露容器、全局数据和调试能力

`EntityApp<E>::installPyModules()` 里真正新增的运行时骨架主要有四类：

- `Entities<E>` 和 `EntityGarbages<E>` 的脚本封装
- `entities` 全局对象，直接注册到 `KBEngine` 模块
- `globalData`，通过 `GlobalDataClient` 暴露给脚本
- `PyWatcher` 支持，用于脚本侧查看 watcher 树

同时它还把一批通用接口注册进脚本模块：

- `kbassert`
- `publish`
- `scriptLogType`
- `getWatcher` / `getWatcherDir`
- 资源访问接口
- 调试常量和错误码

只有做完这一步，脚本层才真正有“运行时环境”可用，而不是只有一个空解释器。

### `installEntityDef()` 才是实体脚本系统的真正入口

`EntityApp<E>::installEntityDef()` 先执行：

```cpp
EntityDef::installScript(this->getScript().getModule())
EntityDef::initialize(scriptBaseTypes_, componentType_)
```

这两步的意义不同：

- `installScript` 把实体定义相关的脚本接口挂到当前 `KBEngine` 模块
- `initialize` 才真正读取 `entities.xml`、`entity_defs/*.def`、`types.xml` 并建立运行时元数据

随后它还会检查 `dbmgr` 配置里的账号实体类型是否存在，这说明实体定义加载不是“只要能解析 XML 就算成功”，而是要满足集群运行前提。

## 第三层：`EntityDef` 把静态定义变成运行时元数据

`EntityDef::initialize(...)` 在 `kbe/src/lib/entitydef/entitydef.cpp` 里完成了三件事。

### 1. 先加载类型系统

它先读取：

- `scripts/entity_defs/types.xml`

这一步由 `DataTypes::initialize(...)` 完成。也就是说属性系统、序列化系统需要的类型信息，在实体脚本真正加载前就已经定型。

### 2. 再遍历 `entities.xml` 和每个 `.def`

它会读取：

- `scripts/entities.xml`
- `scripts/entity_defs/<Entity>.def`

每碰到一个实体，就调用 `registerNewScriptDefModule(moduleName)` 建一个 `ScriptDefModule`，随后把属性、方法、alias、组件定义等内容装进去。

这一层的关键不是 XML 解析本身，而是 `ScriptDefModule` 成了运行时唯一可信的实体元数据载体。后面 RPC、属性同步、持久化、脚本实例化，都从这里取定义。

### 3. 最后再加载实体脚本模块

完成 `.def` 解析后，`EntityDef::initialize(...)` 还会继续走：

```cpp
loadAllEntityScriptModules(__entitiesPath, scriptBaseTypes)
```

所以真实顺序不是“先 import Python 类再看 .def”，而是：

```text
types.xml
  → entities.xml / *.def
  → script::entitydef::initialize()
  → loadAllEntityScriptModules(...)
```

这也是为什么本书前面必须先讲 `EntityDef`，再讲 RPC 和脚本回调。因为运行中的 Python 类，实际上是被 `.def` 约束和补完过的。

## 第四层：`ScriptDefModule` 负责把定义变成可实例化脚本类型

`ScriptDefModule` 的核心职责不是保存配置，而是维护一个“可实例化实体类型”：

- `scriptType_`
- 属性描述表
- 方法描述表
- alias 映射
- 组件描述
- 是否有 base / cell / client 部分

最容易被忽视的一点在 `ScriptDefModule::createObject()`：

```cpp
PyObject * pObject = PyType_GenericAlloc(scriptType_, 0);
```

这说明 KBEngine 创建实体脚本对象时，先做的是 Python 对象分配，而不是直接调用业务构造器。也就是说：

- 运行时实体首先是一个 Python 类型实例
- C++ 的 `Entity` 外壳随后通过 placement new 叠到这个对象上

这正是 KBEngine 脚本桥接的关键设计点：实体不是“C++ 对象拥有一个 Python 成员”，而是同一个对象同时承载 C++ 实体壳和 Python 实例语义。

## 第五层：`createEntity()` 才真正把定义落成运行中实体

真正的实例化发生在 `EntityApp<E>::createEntity(...)`。

它的顺序非常固定：

1. 检查 `EntityID` 是否可分配
2. 用 `EntityDef::findScriptModule(entityType)` 找到 `ScriptDefModule`
3. 校验当前组件是否允许创建这个实体部分
4. `sm->createObject()` 分配 Python 对象
5. 分配或接收 `ENTITY_ID`
6. `onCreateEntity(obj, sm, id)` 构造 C++ `Entity`
7. `entity->initProperty()`
8. 把实体放进 `pEntities_`
9. 如果需要，执行 `entity->initializeEntity(params)`

这一段非常关键，因为它把“元数据、对象、容器、脚本初始化”明确拆开了。

### `initializeEntity()` 并不只是调一次 Python 钩子

`initializeEntity()` 的宏实现定义在 `kbe/src/lib/entitydef/entity_macro.h`，真实逻辑是：

```text
createNamespace(dictData, persistentData)
  → initializeScript()
```

其中：

- `createNamespace(...)` 负责把字典数据灌进实体属性空间，必要时处理 `cellData` 和组件属性
- `initializeScript()` 会先做组件挂接，再调用 `onInitializeScript()`

因此“创建实体”不是简单的 `__init__` 调用，而是：

- 先建立底层属性命名空间
- 再让脚本对象进入可运行状态

这也是很多生命周期钩子要放到属性灌装之后的原因。

## 第六层：回调系统不是语法糖，而是跨异步链路的托管层

脚本里看到的很多异步回调，底层并不是直接把 `PyObject*` 绑在线程或网络请求上，而是统一走 `CallbackMgr`。

核心文件：

- `kbe/src/lib/server/callbackmgr.h`

这个管理器本质上是：

- 给回调分配一个 `CALLBACK_ID`
- 把回调对象和超时时间存进 `cbMap_`
- 在结果返回时通过 `take(callbackID)` 取回
- 超时后由 `tick()` 统一清理

对脚本层最重要的含义有两点：

1. 很多跨组件、跨数据库、跨网络的异步 API，在线路上传的其实不是 Python 回调本身，而是 `callbackID`
2. 某些场景下回调还可以序列化到流里，因为 `CallbackMgr<PyObjectPtr>` 提供了 `addToStream()` 和 `createFromStream()`

这也是为什么在 BaseApp、CellApp、Interfaces 这些组件里，会大量看到 `callbackMgr().save(pyCallback)` / `callbackMgr().take(callbackID)`。

## 第七层：脚本定时器和回调管理器是两套东西

`PythonApp` 里还有一条很容易和 `callbackMgr` 混淆的线：`ScriptTimers`。

在 `python_app.cpp` 里：

- `PythonApp` 构造时初始化 `ScriptTimers`
- `addTimer` / `delTimer` 暴露给 `KBEngine` 模块
- `ScriptTimerHandler` 在超时后直接调用 Python 回调

所以两者边界要分清：

- `ScriptTimers` 解决“本地脚本定时执行”
- `CallbackMgr` 解决“异步请求完成后按 ID 取回回调”

这两个机制都会回到 Python，但它们并不是同一个托管层。

## 第八层：`reloadScript` 重载的是定义、类型和入口，不是整个进程状态

### `EntityApp<E>::reloadScript()` 的真实顺序

`EntityApp<E>::reloadScript(fullReload)` 的源码很短，但语义很重：

```text
EntityDef::reload(fullReload)
  → onReloadScript(fullReload)
  → entryScript.onInit(1)
```

其中 `EntityDef::reload(fullReload)` 会分两种模式：

- `fullReload = true`：把现有 `ScriptDefModule` 转存到 old 容器，`finalise(true)` 后重新 `initialize(...)`
- `fullReload = false`：只重新加载实体脚本模块，不重建整套定义

这说明 `reloadScript` 不是一个“统一热更新开关”，而是分成了“重建定义”和“仅重载模块”两档。

### `onReloadScript()` 只会刷新一部分运行中对象

`EntityApp<E>::onReloadScript(fullReload)` 会遍历：

- `EntityCall::entityCalls`
- `EntityComponent::entity_components`

分别调用它们的 `reload()`。

BaseApp / CellApp 还会额外遍历当前存活实体，逐个执行：

```cpp
entity->reload(fullReload)
```

而实体自己的 `reload()` 在 `entity_macro.h` 里可以看到两件关键事：

- `fullReload` 时重新找新的 `ScriptDefModule`
- 把 `__class__` 切到新的 `scriptType_`

这说明 KBEngine 的热重载核心不是“重跑构造函数”，而是让现有 Python 实例切换到新的类定义，并重新绑定属性描述。

### 热重载的边界

从这条实现可以看出几个明确边界：

- 进程不会因为 `reloadScript` 重建
- 已有实体不会重新分配 `ENTITY_ID`
- 运行中的实体容器不会被整体丢弃
- 真正被刷新的，是 `EntityDef` 元数据、`EntityCall` / `EntityComponent` 包装、实体的 Python 类绑定，以及入口脚本的 `onInit(1)`

换句话说，`reloadScript` 更接近“切换运行时定义并通知现存对象重新挂到新定义上”，而不是完全意义上的状态迁移系统。

## 第九层：为什么 `Baseapp` / `Cellapp` 和 `interfaces` / `dbmgr` 的脚本感受完全不同

源码层面的原因很简单：

- `interfaces`、`dbmgr` 这类组件继承的是 `PythonApp`
- `baseapp`、`cellapp` 继承的是 `EntityApp<E>`

所以前者的脚本模型更像“入口模块 + 若干导出接口”，后者则是“入口模块 + 实体定义系统 + 实体容器 + 脚本实例生命周期”。

这也是为什么：

- 在 `interfaces` 里你更多看到回调、数据库、入口函数式逻辑
- 在 `baseapp` / `cellapp` 里你看到的是实体实例、属性空间、实体调用和生命周期钩子

如果不先区分这两层，再去看“为什么某个钩子只在 Base/Cell 世界里成立”，很容易把整个脚本系统理解乱。

## 阅读这页后，下一步应该接哪几页

- 如果你想继续看“实体定义如何约束 RPC 和属性同步”，接着读 `/study/05-entitydef-and-entity-definition.html`
- 如果你想继续看“回调、定时器、事件在脚本层怎么体现”，接着读 `/study/18-hooks-callbacks-timers-and-events.html`
- 如果你想继续看“热更新在运维和故障处理里的边界”，接着读 `/study/21-hotupdate-fault-tolerance-and-ops.html`

## 本页涉及的关键源码

- `kbe/src/lib/server/python_app.h`
- `kbe/src/lib/server/python_app.cpp`
- `kbe/src/lib/server/entity_app.h`
- `kbe/src/lib/entitydef/entitydef.cpp`
- `kbe/src/lib/entitydef/scriptdef_module.cpp`
- `kbe/src/lib/server/callbackmgr.h`
- `kbe/src/lib/entitydef/entity_macro.h`
- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/cellapp/cellapp.cpp`
