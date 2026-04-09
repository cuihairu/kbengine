# API 分类与核对清单

> 这份清单用于把 `docs-vuepress/api` 下的页面按职责分层，并按分类逐批核对 API 文案、参数语义、返回值、同步边界与源码实现。

## 核对原则

- 先核对接口真实语义，再核对参数、返回值与调用边界。
- API 页只保留接口说明、注意点与跳转；复杂机制统一下沉到 `architecture/source-analysis`。
- 对 `Player 身份`、`控制权`、`可见性`、`ownerOnly`、`同步/广播`、`异步回调` 这些容易混淆的概念，必须拆开说明。
- CHM 原文、现有文案、源码行为三者冲突时，以源码实际行为为准，并在 API 页补充“源码解析”跳转。

## 分类总览

| 分类 | 页面范围 | 当前重点 | 状态 |
| --- | --- | --- | --- |
| 核心概念层 | [api/README](/api/README.md)、[basetypes](/api/basetypes.md)、[keywords](/api/keywords.md) | 基础类型、关键词、EntityCall/属性描述术语统一 | 未开始 |
| 客户端运行时层 | [client/KBEngine](/api/client/KBEngine.md)、[client/Entity](/api/client/Entity.md)、[bots/KBEngine](/api/bots/KBEngine.md)、[bots/Entity](/api/bots/Entity.md)、[bots/PyClientApp](/api/bots/PyClientApp.md) | `player`、`isPlayer`、`isControlled`、可见性回调、客户端移动与 ownerOnly 边界 | 进行中 |
| 服务端实体层 | [baseapp/Entity](/api/baseapp/Entity.md)、[baseapp/Proxy](/api/baseapp/Proxy.md)、[cellapp/Entity](/api/cellapp/Entity.md) | Base/Cell 实体职责、`controlledBy`、控制器、AOI、持久化、客户端绑定 | 进行中 |
| 服务端全局入口层 | [baseapp/KBEngine](/api/baseapp/KBEngine.md)、[cellapp/KBEngine](/api/cellapp/KBEngine.md)、[loginapp/KBEngine](/api/loginapp/KBEngine.md)、[dbmgr/KBEngine](/api/dbmgr/KBEngine.md)、[interfaces/KBEngine](/api/interfaces/KBEngine.md)、[logger/KBEngine](/api/logger/KBEngine.md) | 全局函数、全局数据字典、进程级回调、组件边界 | 进行中 |
| 交叉专题层 | 跨多个 API 页的同名概念 | 玩家身份/控制权、生命周期、计时器、全局数据、属性同步、空间与控制器 | 进行中 |

## 推荐核对顺序

1. 客户端运行时层
2. 服务端实体层
3. 服务端全局入口层
4. 核心概念层

这样排的原因很直接：

- 客户端 API 最容易出现“名字像控制权、实际是身份判断”的误读。
- 服务端实体层决定了客户端 API 的真实语义，第二批跟进最合适。
- 进程级 `KBEngine` 入口多，但大多是围绕实体、数据字典与回调展开，放在实体层之后核对更稳。
- `basetypes` 和 `keywords` 更像统一术语表，适合作为最后的收口整理。

## 分类明细

### 1. 核心概念层

页面：

- [API 总览](/api/README.md)
- [基本数据类型](/api/basetypes.md)
- [关键词释义](/api/keywords.md)

重点核对：

- 基础类型定义是否和 `.def`、脚本桥接、序列化实现一致
- `ENTITYCALL`、`CELLDATADICT`、`PyClient` 等术语是否只讲“表面名词”，没有交代运行时语义
- 关键词之间是否存在重复解释或互相矛盾

建议关联源码解析：

- [Entity 与实体系统](/architecture/source-analysis/entity-system.html)
- [脚本系统与 Python 桥接](/architecture/source-analysis/scripting.html)

### 2. 客户端运行时层

页面：

- [client/KBEngine](/api/client/KBEngine.md)
- [client/Entity](/api/client/Entity.md)
- [bots/KBEngine](/api/bots/KBEngine.md)
- [bots/Entity](/api/bots/Entity.md)
- [bots/PyClientApp](/api/bots/PyClientApp.md)

重点核对：

- `player()`、`isPlayer()` 与 `controlledBy` / `isControlled` 的边界
- `onEnterWorld()`、`onLeaveWorld()`、`onEnterSpace()`、`onLeaveSpace()` 的真实触发语义
- `baseCall()`、`cellCall()` 的调用限制与“只能访问玩家实体”这类边界条件
- `isOnGround`、移动接口、客户端可写属性的同步方向
- `ownerOnly` 属性在客户端的可见性与回调行为

建议关联源码解析：

- [网络与消息系统](/architecture/source-analysis/networking.html#client-entity-isplayer-control)
- [空间、AOI 与视野系统](/architecture/source-analysis/space-aoi.html)

当前已完成：

- [client/KBEngine.player](/api/client/KBEngine.md#player)
- [client/KBEngine.findEntity](/api/client/KBEngine.md#findEntity)
- [client/KBEngine.entities](/api/client/KBEngine.md#entities)
- [client/KBEngine.entity_id](/api/client/KBEngine.md#entity_id)
- [client/KBEngine.spaceID](/api/client/KBEngine.md#spaceID)
- [client/Entity.baseCall](/api/client/Entity.md#baseCall)
- [client/Entity.cellCall](/api/client/Entity.md#cellCall)
- [client/Entity.isPlayer](/api/client/Entity.md#isPlayer)
- [client/Entity.onEnterWorld](/api/client/Entity.md#onEnterWorld)
- [client/Entity.onLeaveWorld](/api/client/Entity.md#onLeaveWorld)
- [client/Entity.onEnterSpace](/api/client/Entity.md#onEnterSpace)
- [client/Entity.onLeaveSpace](/api/client/Entity.md#onLeaveSpace)
- [client/Entity.base](/api/client/Entity.md#base)
- [client/Entity.cell](/api/client/Entity.md#cell)
- [client/Entity.clientapp](/api/client/Entity.md#clientapp)
- [client/Entity.isOnGround](/api/client/Entity.md#isOnGround)
- [client/Entity.inWorld](/api/client/Entity.md#inWorld)
- [bots/PyClientApp.player](/api/bots/PyClientApp.md#player)
- [bots/PyClientApp.id](/api/bots/PyClientApp.md#id)
- [bots/PyClientApp.entities](/api/bots/PyClientApp.md#entities)
- [bots/Entity.isPlayer](/api/bots/Entity.md#isPlayer)
- [bots/Entity.onEnterWorld](/api/bots/Entity.md#onEnterWorld)
- [bots/Entity.onLeaveWorld](/api/bots/Entity.md#onLeaveWorld)
- [bots/Entity.onEnterSpace](/api/bots/Entity.md#onEnterSpace)
- [bots/Entity.onLeaveSpace](/api/bots/Entity.md#onLeaveSpace)
- [bots/Entity.base](/api/bots/Entity.md#base)
- [bots/Entity.cell](/api/bots/Entity.md#cell)
- [bots/Entity.clientapp](/api/bots/Entity.md#clientapp)
- [bots/Entity.moveToPoint](/api/bots/Entity.md#moveToPoint)
- [bots/Entity.cancelController](/api/bots/Entity.md#cancelController)
- [bots/Entity.isOnGround](/api/bots/Entity.md#isOnGround)

下一批待核对：

- `cellapp/Entity.addTimer()` / `delTimer()`
- `cellapp/Entity.moveToPoint()` / `navigate()` / `cancelController()`

### 3. 服务端实体层

页面：

- [baseapp/Entity](/api/baseapp/Entity.md)
- [baseapp/Proxy](/api/baseapp/Proxy.md)
- [cellapp/Entity](/api/cellapp/Entity.md)

重点核对：

- Base / Cell / Proxy 各自职责边界
- `controlledBy`、`giveClientTo`、客户端迁移与控制权切换
- 控制器接口、导航接口、AOI / Witness / View 回调
- `writeToDB()`、cell 备份、灾难恢复与真实持久化链路

建议关联源码解析：

- [Entity 与实体系统](/architecture/source-analysis/entity-system.html)
- [空间、AOI 与视野系统](/architecture/source-analysis/space-aoi.html)
- [持久化](/architecture/source-analysis/persistence.html)

当前已完成：

- [cellapp/Entity.controlledBy](/api/cellapp/Entity.md#controlledBy)
- [baseapp/Entity.createCellEntity](/api/baseapp/Entity.md#createCellEntity)
- [baseapp/Entity.onGetCell](/api/baseapp/Entity.md#onGetCell)
- [baseapp/Proxy.giveClientTo](/api/baseapp/Proxy.md#giveClientTo)
- [cellapp/Entity.addTimer](/api/cellapp/Entity.md#addTimer)
- [cellapp/Entity.delTimer](/api/cellapp/Entity.md#delTimer)
- [cellapp/Entity.moveToPoint](/api/cellapp/Entity.md#moveToPoint)
- [cellapp/Entity.navigate](/api/cellapp/Entity.md#navigate)
- [cellapp/Entity.cancelController](/api/cellapp/Entity.md#cancelController)

优先核对：

- `baseapp/Entity.writeToDB()`
- `baseapp/Entity.destroy()`
- `cellapp/Entity.addProximity()`
- `cellapp/Entity.writeToDB()`

### 4. 服务端全局入口层

页面：

- [baseapp/KBEngine](/api/baseapp/KBEngine.md)
- [cellapp/KBEngine](/api/cellapp/KBEngine.md)
- [loginapp/KBEngine](/api/loginapp/KBEngine.md)
- [dbmgr/KBEngine](/api/dbmgr/KBEngine.md)
- [interfaces/KBEngine](/api/interfaces/KBEngine.md)
- [logger/KBEngine](/api/logger/KBEngine.md)

重点核对：

- 每个进程级 `KBEngine` 模块到底提供哪些“只在本组件可用”的接口
- `globalData`、`baseAppData`、`cellAppData` 的同步方向、回调时机与使用边界
- 账号、数据库、外部接口、日志相关 API 的真实职责

建议关联源码解析：

- [网络与消息系统](/architecture/source-analysis/networking.html)
- [持久化](/architecture/source-analysis/persistence.html)

当前已完成：

- [baseapp/KBEngine.baseAppData](/api/baseapp/KBEngine.md#baseAppData)
- [baseapp/KBEngine.globalData](/api/baseapp/KBEngine.md#globalData)
- [baseapp/KBEngine.onBaseAppData](/api/baseapp/KBEngine.md#onBaseAppData)
- [baseapp/KBEngine.onBaseAppDataDel](/api/baseapp/KBEngine.md#onBaseAppDataDel)
- [baseapp/KBEngine.onGlobalData](/api/baseapp/KBEngine.md#onGlobalData)
- [baseapp/KBEngine.onGlobalDataDel](/api/baseapp/KBEngine.md#onGlobalDataDel)
- [cellapp/KBEngine.cellAppData](/api/cellapp/KBEngine.md#cellAppData)
- [cellapp/KBEngine.globalData](/api/cellapp/KBEngine.md#globalData)
- [cellapp/KBEngine.onCellAppData](/api/cellapp/KBEngine.md#onCellAppData)
- [cellapp/KBEngine.onCellAppDataDel](/api/cellapp/KBEngine.md#onCellAppDataDel)
- [cellapp/KBEngine.onGlobalData](/api/cellapp/KBEngine.md#onGlobalData)
- [cellapp/KBEngine.onGlobalDataDel](/api/cellapp/KBEngine.md#onGlobalDataDel)

优先核对：

- `loginapp/KBEngine` 入口回调与账号流程接口
- `dbmgr/KBEngine` 数据库入口与回调接口
- `interfaces/KBEngine` 外部接口桥接
- `logger/KBEngine` 进程日志入口

### 5. 交叉专题层

这一层不按单页推进，而是按“同一个概念横跨多个组件”推进。

当前专题：

- 玩家身份与控制权
- 实体生命周期与视野可见性
- 定时器与回调
- 全局数据字典同步
- 属性同步、广播与 `ownerOnly`
- 移动、控制器与导航

已打通的专题入口：

- [client/KBEngine.player](/api/client/KBEngine.md#player)
- [client/KBEngine.findEntity](/api/client/KBEngine.md#findEntity)
- [client/Entity.baseCall](/api/client/Entity.md#baseCall)
- [client/Entity.cellCall](/api/client/Entity.md#cellCall)
- [client/Entity.isPlayer](/api/client/Entity.md#isPlayer)
- [client/Entity.onEnterWorld](/api/client/Entity.md#onEnterWorld)
- [client/Entity.onEnterSpace](/api/client/Entity.md#onEnterSpace)
- [client/Entity.base](/api/client/Entity.md#base)
- [client/Entity.cell](/api/client/Entity.md#cell)
- [client/Entity.clientapp](/api/client/Entity.md#clientapp)
- [client/Entity.isOnGround](/api/client/Entity.md#isOnGround)
- [client/Entity.inWorld](/api/client/Entity.md#inWorld)
- [bots/PyClientApp.player](/api/bots/PyClientApp.md#player)
- [bots/PyClientApp.entities](/api/bots/PyClientApp.md#entities)
- [bots/Entity.isPlayer](/api/bots/Entity.md#isPlayer)
- [bots/Entity.onEnterWorld](/api/bots/Entity.md#onEnterWorld)
- [bots/Entity.onEnterSpace](/api/bots/Entity.md#onEnterSpace)
- [bots/Entity.base](/api/bots/Entity.md#base)
- [bots/Entity.cell](/api/bots/Entity.md#cell)
- [bots/Entity.clientapp](/api/bots/Entity.md#clientapp)
- [bots/Entity.moveToPoint](/api/bots/Entity.md#moveToPoint)
- [bots/Entity.cancelController](/api/bots/Entity.md#cancelController)
- [bots/Entity.isOnGround](/api/bots/Entity.md#isOnGround)
- [client/KBEngine.entity_id](/api/client/KBEngine.md#entity_id)
- [client/KBEngine.spaceID](/api/client/KBEngine.md#spaceID)
- [baseapp/Entity.createCellEntity](/api/baseapp/Entity.md#createCellEntity)
- [baseapp/Entity.onGetCell](/api/baseapp/Entity.md#onGetCell)
- [cellapp/Entity.controlledBy](/api/cellapp/Entity.md#controlledBy)
- [cellapp/Entity.addTimer](/api/cellapp/Entity.md#addTimer)
- [cellapp/Entity.delTimer](/api/cellapp/Entity.md#delTimer)
- [cellapp/Entity.moveToPoint](/api/cellapp/Entity.md#moveToPoint)
- [cellapp/Entity.navigate](/api/cellapp/Entity.md#navigate)
- [cellapp/Entity.cancelController](/api/cellapp/Entity.md#cancelController)
- [baseapp/KBEngine.baseAppData](/api/baseapp/KBEngine.md#baseAppData)
- [baseapp/KBEngine.globalData](/api/baseapp/KBEngine.md#globalData)
- [baseapp/KBEngine.onBaseAppData](/api/baseapp/KBEngine.md#onBaseAppData)
- [baseapp/KBEngine.onBaseAppDataDel](/api/baseapp/KBEngine.md#onBaseAppDataDel)
- [baseapp/KBEngine.onGlobalData](/api/baseapp/KBEngine.md#onGlobalData)
- [baseapp/KBEngine.onGlobalDataDel](/api/baseapp/KBEngine.md#onGlobalDataDel)
- [cellapp/KBEngine.cellAppData](/api/cellapp/KBEngine.md#cellAppData)
- [cellapp/KBEngine.globalData](/api/cellapp/KBEngine.md#globalData)
- [cellapp/KBEngine.onCellAppData](/api/cellapp/KBEngine.md#onCellAppData)
- [cellapp/KBEngine.onCellAppDataDel](/api/cellapp/KBEngine.md#onCellAppDataDel)
- [cellapp/KBEngine.onGlobalData](/api/cellapp/KBEngine.md#onGlobalData)
- [cellapp/KBEngine.onGlobalDataDel](/api/cellapp/KBEngine.md#onGlobalDataDel)
- [源码解析：`Entity.createCellEntity()` 不是本地建对象，而是一次 Base -> Cell 交接](/architecture/source-analysis/entity-system.html#base-entity-create-cell)
- [源码解析：`isPlayer()`、`player()` 和 `controlledBy()` 的关系](/architecture/source-analysis/networking.html#client-entity-isplayer-control)
- [源码解析：客户端实体进入/离开世界与空间回调](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)
- [源码解析：`baseCall()` 与 `cellCall()` 的真实限制条件](/architecture/source-analysis/networking.html#client-entity-remote-calls)
- [源码解析：客户端 `moveToPoint()`、`cancelController()` 与 `isOnGround` 同步](/architecture/source-analysis/networking.html#client-entity-move-ground-sync)
- [源码解析：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)
- [baseapp/Proxy.giveClientTo](/api/baseapp/Proxy.md#giveClientTo)
- [源码解析：`Proxy.giveClientTo()` 转移的是客户端绑定与 Witness 链路](/architecture/source-analysis/space-aoi.html#proxy-give-client-to)
- [源码解析：Cell 实体的 `addTimer()` / `delTimer()` 实际上操作的是 `ScriptID -> TimerHandle` 映射](/architecture/source-analysis/scripting.html#cell-entity-script-timers)
- [源码解析：Cell 实体的 `moveToPoint()` / `navigate()` / `cancelController()` 共享同一套控制器链](/architecture/source-analysis/space-aoi.html#cell-entity-move-controllers)
- [源码解析：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

## 当前执行策略

- 先把“一个术语在多个页面写法不一致”的问题收敛掉
- 再把每个分类里最容易误解的 API 按“API 页简述 + 源码解析页深讲 + API 页跳转”补齐
- 最后再回到 `basetypes`、`keywords` 做统一术语清理
