# 5. EntityDef 与实体定义系统

> 本文件是旧稿，已并入并由 [`05-entitydef-and-entity-definition.md`](/study/05-entitydef-and-entity-definition.html) 取代。下面内容仅保留作迁移参考，阅读主线请以新稿为准。

> 如果第 8 章讲的是“方法怎么调出去”，这一章讲的就是“这些方法、属性、组件定义最初从哪里来，又怎样变成一个真正可运行的实体对象”。

## 10.1 本章核心问题

- Entity 定义文件怎样决定运行时行为？
- `EntityDef` 如何连接脚本层、网络层与持久化层？
- 一个实体从定义到实例化，中间发生了什么？

## 10.2 先给结论：`EntityDef` 不是配置表，而是运行时骨架

很多人初看 `.def` 会把它理解成“给脚本补充一点元数据”。这不准确。

在 KBEngine 里，实体定义实际上同时决定：

- 哪些属性属于 Base / Cell / Client
- 哪些方法可以远程调用，以及属于哪一侧
- 哪些属性持久化
- 哪些属性同步给客户端
- 组件如何嵌套进实体
- 运行时如何按 `Utype / alias` 查找属性和方法

所以 `EntityDef` 不是“脚本旁边的一份配置”，而是：

- 脚本对象的运行时描述
- 网络协议描述
- 持久化描述

三者合一的一层。

## 10.3 `ScriptDefModule` 是实体定义在内存中的核心载体

源码入口：

- `kbe/src/lib/entitydef/scriptdef_module.cpp`
- `kbe/src/lib/entitydef/scriptdef_module.h`

`ScriptDefModule` 一眼看上去字段很多，但如果按职责拆开，其实很清楚：

### 属性描述表

- `cellPropertyDescr_`
- `basePropertyDescr_`
- `clientPropertyDescr_`
- `persistentPropertyDescr_`

### 方法描述表

- `methodCellDescr_`
- `methodBaseDescr_`
- `methodClientDescr_`
- 以及 exposed / uidmap / aliasmap

### 组件描述表

- `componentDescr_`
- `componentDescr_uidmap_`
- `componentPropertyDescr_`

### 运行时优化信息

- `usePropertyDescrAlias_`
- `useMethodDescrAlias_`
- `useComponentDescrAlias_`
- `pVolatileinfo_`

这说明 `ScriptDefModule` 并不是“只存脚本类名”。它是：

- 这个实体类型在引擎中的完整运行时元模型

## 10.4 属性是怎样被装进定义模块的

源码入口：

- `ScriptDefModule::addPropertyDescription`

这段代码非常值得仔细看，因为它直接体现了 KBEngine 对属性的基本分类方式。

加入一个属性时，系统会同时做几件事：

- 检查与方法名、组件名是否冲突
- 按所属侧放入 Cell / Base / Client 对应 map
- 通过 `utype` 放入 uid map
- 如果是持久化属性，再放入 `persistentPropertyDescr_`
- 如果是组件属性，还会登记到 `componentPropertyDescr_`
- 若属性带客户端同步标志，还会按 `detailLevel` 收集

这意味着“一个属性”在 KBEngine 里至少有四层身份：

1. Python 属性名
2. 所属侧
3. 协议字段 ID
4. 是否持久化 / 是否客户端可见 / 是否组件入口

所以你后面看到的很多行为，其实都不是运行时动态猜出来的，而是这里预先建好的索引决定的。

## 10.5 方法描述为什么也必须进入 `ScriptDefModule`

源码入口：

- `findMethodDescription`
- `findCellMethodDescription`
- `findBaseMethodDescription`
- `findClientMethodDescription`

这一层的意义在第 8 章已经看过一部分，但放在定义系统里看更完整：

- 远程调用靠 `MethodDescription`
- 方法参数检查靠 `MethodDescription`
- 方法 `Utype` 查找靠 `MethodDescription`
- exposed 方法集合也在这里收口

这意味着脚本里写的远程方法，不是简单从 Python 类上 `getattr` 之后直接调用，而是：

- 先从 `ScriptDefModule` 里拿到元描述
- 再决定怎么编码、怎么路由、怎么落地

## 10.6 alias 机制说明了定义系统并不只是“语义层”

源码入口：

- `ScriptDefModule::onLoaded`

这里做的一件非常关键的事情是：

- 给客户端相关属性分配 aliasID
- 给客户端方法分配 aliasID
- 若数量超限则放弃 alias 优化

这说明定义系统不只是“告诉你有哪些属性和方法”，它还会直接影响协议压缩方式。

也就是说：

- 同一份定义
- 不仅决定业务语义
- 还决定客户端同步包里用长 ID 还是短 alias

所以 `EntityDef` 和网络层从来就不是两层互不相干的系统。

## 10.7 实体真正实例化时发生了什么

源码入口：

- `kbe/src/lib/server/entity_app.h`
  `EntityApp<E>::createEntity`
- `kbe/src/lib/entitydef/scriptdef_module.cpp`
  `ScriptDefModule::createObject`

`EntityApp<E>::createEntity` 是这一章最关键的函数之一。它把“定义世界”推进到“运行世界”。

它的流程可以概括成：

1. 按 `entityType` 找 `ScriptDefModule`
2. 检查当前组件侧是否允许创建这个实体
   例如 Base 侧必须 `hasBase()`，Cell 侧必须 `hasCell()`
3. 调 `sm->createObject()` 分配 Python 对象
4. 分配或使用已有 `ENTITY_ID`
5. 通过 `onCreateEntity(obj, sm, id)` 在这块 Python 对象内存上构造 C++ 实体对象
6. `initProperty()`
7. 把实体加入实体容器
8. 若需要，执行 `initializeEntity(params)`

这里非常值得记住的一点是：

- KBEngine 实体不是“先有脚本对象，再外挂一个 C++ 句柄”
- 而是 Python 对象与 C++ 实体对象在一个统一构造流程里结合起来

这也是它为什么能把属性访问、脚本方法、网络元数据绑定得这么紧。

## 10.8 `initializeEntity` 说明“初始值来源”也被定义系统统一了

相关入口：

- `kbe/src/lib/entitydef/entity_macro.h`
  `initializeEntity`
  `initProperty`
- `kbe/src/server/baseapp/baseapp.cpp`
  `onQueryAccountCBFromDbmgr`
- `kbe/src/server/cellapp/cellapp.cpp`
  创建 cell 实体的路径

虽然 `initializeEntity` 展开在宏里，不适合第一次就全读，但它至少能让你建立一个正确认识：

- 实体初始化不是单纯“调用 Python `__init__`”
- 而是按定义系统逐项安装属性、组件、默认值和传入数据

这些数据的来源可能不同：

- 新建实体时来自脚本传参
- 从 DB 恢复时来自持久化流
- Base 创建 Cell 时来自 `cellData` 流
- 组件子数据则继续沿组件定义递归展开

所以 KBEngine 实体的“初始化”本质上是一套定义驱动的数据装配过程。

## 10.9 持久化与定义系统是怎样接上的

相关入口：

- `baseapp/entity.cpp`
  `addPersistentsDataToStream`
- `createDictDataFromPersistentStream`
  在账号恢复等路径中使用
- `ScriptDefModule::persistentPropertyDescr_`

这一层最关键的理解是：

- 不是脚本随便挑几个字段自己去写库
- 而是定义系统先告诉引擎“哪些字段是 persistent”
- Base 侧再按这份 persistent 描述统一编码与恢复

所以定义系统同时承担：

- 运行时属性模型
- 持久化字段白名单

这就是为什么改 `.def` 文件常常不是局部修改，而会同时影响：

- DB 表结构
- 写库流
- 恢复流
- 客户端同步

## 10.10 组件系统让“实体定义”不再只是平铺字段

源码入口：

- `kbe/src/lib/entitydef/entity_component.cpp`
- `kbe/src/lib/entitydef/entity_component_call.cpp`

组件系统的意义不是把代码拆好看，而是把定义系统推进成“实体中还可再挂一层子定义”。

从实现上看，组件会带来几件事：

- 组件本身也是一种属性
- 组件内部可以有自己的属性描述和方法描述
- 组件方法调用时可能还要先写一个组件 property UID / alias
- 持久化和恢复也会递归进入组件内部

这说明 KBEngine 的实体定义不是一张平面表，而更像一棵小型对象树。

## 10.11 一个建议的源码走读路径

如果你要把这一章真正跟通，我建议按这个顺序：

1. `entitydef.cpp`
   看 `.def` 怎样加载成属性 / 方法描述
2. `scriptdef_module.cpp`
   看描述怎样被收进 `ScriptDefModule`
3. `entity_app.h`
   看 `createEntity`
4. `entity_macro.h`
   看 `initProperty / initializeEntity`
5. `entity_component.cpp`
   看组件如何递归装配
6. `baseapp/entity.cpp`
   看 persistent 流怎样由定义控制

跟的时候要一直问自己四个问题：

- 这个字段属于 Base、Cell 还是 Client？
- 这个字段是否持久化？
- 这个字段是否参与客户端同步？
- 这个字段是否只是组件入口，而内部还有自己的子定义？

## 10.12 本章小结

这一章最重要的结论是：

- `.def` 文件不是辅助配置，而是实体运行时元模型
- `ScriptDefModule` 是这份元模型在内存中的核心表示
- 属性 / 方法 / 组件 / 持久化 / alias 都在这里汇合
- `EntityApp::createEntity` 把定义世界推进成真实运行对象
- 初始化、持久化、远程调用、客户端同步都建立在同一份定义之上

只有把这层看清楚，后面读 Base / Cell 实体源码时，才不会把那些 `findPropertyDescription`、`findMethodDescription` 当作零散工具函数。
