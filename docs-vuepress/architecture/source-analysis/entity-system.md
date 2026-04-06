# 实体系统

> 这一页回答的不是“Entity 是什么”这种概念题，而是源码题：`.def` 文件怎样变成 `ScriptDefModule`，`ScriptDefModule` 又怎样变成一个真正运行中的 C++/Python 双栈实体。

## 实体系统的三层结构

从源码看，KBEngine 的实体系统至少分成三层：

- 定义层：`EntityDef` + `ScriptDefModule`
- 构造层：`EntityApp::createEntity()`
- 运行层：Base / Cell 两侧具体 `Entity` 子系统

这三层分别对应三个问题：

- 这个实体“允许有什么属性和方法”？
- 这个实体“怎样被实例化”？
- 这个实体“实例化后由谁托管、怎么参与网络和脚本运行时”？

## 第一步：EntityDef 先把 `.def` 读成运行时元数据

最关键的入口是：

- `kbe/src/lib/entitydef/entitydef.h`
- `kbe/src/lib/entitydef/entitydef.cpp`
- `kbe/src/lib/entitydef/scriptdef_module.h`
- `kbe/src/lib/entitydef/scriptdef_module.cpp`

`EntityDef::initialize()` 做的事情并不是“读配置”，而是建立整个实体运行时的元数据世界。

在 `kbe/src/lib/entitydef/entitydef.cpp` 中，这个初始化流程很清楚：

1. 记录当前加载组件类型
2. 初始化 `DataTypes`
3. 打开 `entities.xml`
4. 为每个实体名注册一个 `ScriptDefModule`
5. 读取 `entity_defs/<Entity>.def`
6. 加载属性、方法、detailLevel、volatileInfo、组件定义
7. 调用 `script::entitydef::initialize()`
8. 对非 `DBMGR_TYPE` 组件继续加载 Python 实体脚本

也就是说，`.def` 并不是给编辑器看的静态描述文件，而是后续 RPC、属性同步、持久化、客户端 SDK 生成的共同基础。

## 第二步：ScriptDefModule 是实体定义的真正承载体

`ScriptDefModule` 是理解实体系统时最该先读的类。

在 `kbe/src/lib/entitydef/scriptdef_module.h` 里，它内部维护了几组核心映射：

- `PROPERTYDESCRIPTION_MAP`
- `METHODDESCRIPTION_MAP`
- `COMPONENTDESCRIPTION_MAP`
- 各种 UID / alias 映射表

这说明一个实体定义并不是“属性列表 + 方法列表”这么简单，而是一整套运行时协议描述：

- 哪些属性属于 Base / Cell / Client
- 哪些方法可以从哪一侧调用
- 属性和方法的 UID / alias 是多少
- 这个实体是否持久化
- detailLevel / volatileInfo 如何影响同步
- 它是否还包含组件型子定义

因此 `ScriptDefModule` 更像“实体协议模块”，而不是简单的脚本类反射对象。

## 第三步：实体对象不是先 Python 再 C++，而是统一构造

很多读者第一次看会误以为：

- 先 new 一个 C++ Entity
- 再给它挂一个 Python 脚本对象

源码正好说明不是这样。

在 `kbe/src/lib/entitydef/scriptdef_module.cpp`：

```cpp
PyObject* ScriptDefModule::createObject(void)
{
    PyObject * pObject = PyType_GenericAlloc(scriptType_, 0);
    // ...
    return pObject;
}
```

先分配的是 Python 对象外壳，但它还没真正成为一个“可运行实体”。

真正把 Python 对象和 C++ 实体绑成一个东西的，是 `EntityApp::createEntity()`：

```cpp
// 文件：kbe/src/lib/server/entity_app.h
ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
PyObject* obj = sm->createObject();
E* entity = onCreateEntity(obj, sm, id);
```

而 `onCreateEntity()` 的默认实现是：

```cpp
return new(pyEntity) E(eid, sm);
```

这里的关键点在于：C++ `Entity` 直接 placement-new 到 Python 对象持有的那块对象内存上。它不是两个对象互相引用，而是一体化构造。

## 第四步：EntityApp::createEntity() 才是实例化总入口

真正值得逐行跟读的代码在 `kbe/src/lib/server/entity_app.h`：

- `EntityApp<E>::createEntity()`
- `EntityApp<E>::onCreateEntity()`

这条路径的逻辑非常典型：

1. 检查实体 ID 是否可分配
2. 通过 `EntityDef::findScriptModule(entityType)` 找到定义
3. 检查当前组件是否允许承载该实体
4. 用 `ScriptDefModule::createObject()` 分配 Python 对象
5. 分配或使用现有 `ENTITY_ID`
6. 调 `onCreateEntity()` 完成 C++ 实体构造
7. `initProperty()`
8. 放入 `pEntities_`
9. `initializeEntity(params)` 做脚本侧初始化

这说明实体创建并不是某个单点 API，而是一个“定义校验 → 对象分配 → 运行态注册 → 脚本初始化”的四阶段流程。

## 第五步：Base / Cell 两侧共用骨架，但语义不同

Base 和 Cell 都继承 `EntityApp<Entity>`，但并不意味着它们承载同一种实体语义。

在源码里可以直接看到：

- `kbe/src/server/baseapp/baseapp.h`
- `kbe/src/server/cellapp/cellapp.h`

两边都覆盖了：

- `onCreateEntity(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid)`

共享的部分是：

- 实体定义查找
- Python / C++ 统一构造
- 实体 ID 分配
- 实体容器管理

不同的部分是：

- Base 侧增加会话、Proxy、DB 协作、非空间逻辑
- Cell 侧增加空间、AOI、Witness、ghost、实时同步

所以“Base Entity”和“Cell Entity”不是两个完全独立的体系，而是建立在同一创建骨架上的两种运行时语义。

## 属性、方法、持久化为什么都依赖实体定义

实体系统最容易被低估的点是：它不只是创建对象，也定义了协议。

具体来说：

- 属性同步依赖 `PropertyDescription`
- RPC 依赖 `MethodDescription`
- 持久化依赖 persistent property 映射
- 客户端 SDK 生成依赖 client property / client method 描述

从 `ScriptDefModule` 的成员就能看出这一点：

- `getCellPropertyDescriptions()`
- `getBasePropertyDescriptions()`
- `getClientPropertyDescriptions()`
- `getPersistentPropertyDescriptions()`
- `getBaseMethodDescriptions()`
- `getCellMethodDescriptions()`
- `getClientMethodDescriptions()`

因此如果没先理解 `EntityDef` / `ScriptDefModule`，后面网络层、DB 层、客户端协议层都会看得像“魔法字符串”。

## 这一层和脚本桥接的关系

实体系统和脚本运行时不是两页无关内容。

它们的依赖顺序是：

1. `EntityDef` 先确定这个实体长什么样
2. `ScriptDefModule` 绑定到 Python `PyTypeObject`
3. `EntityApp::createEntity()` 用这个定义创建实例
4. `initializeEntity()` 把脚本初始数据灌进实例

也就是说，脚本桥接不是独立系统，而是实体定义系统的后半段。

## 读源码的最短路径

如果你准备在 IDE 里走一遍实体系统，建议只跟下面这条链：

1. `kbe/src/lib/entitydef/entitydef.cpp` → `EntityDef::initialize`
2. `kbe/src/lib/entitydef/scriptdef_module.cpp` → `ScriptDefModule::createObject`
3. `kbe/src/lib/server/entity_app.h` → `EntityApp::createEntity`
4. `kbe/src/server/baseapp/baseapp.cpp` / `kbe/src/server/cellapp/cellapp.cpp` → `onCreateEntity`
5. `kbe/src/lib/entitydef/entity_macro.h` → `initializeEntity`

这样读能把“定义”和“实例化”一次性串起来。

## 与主线章节的关系

如果你要看完整叙事和更多上下文，主线仍然在：

- `/study/05-entitydef-and-entity-definition.html`
- `/study/06-python-runtime-and-script-bridge.html`

这一页的目标不是替代主线，而是给 `architecture/source-analysis/` 补上一张真正可落到源码的实体阅读地图。
