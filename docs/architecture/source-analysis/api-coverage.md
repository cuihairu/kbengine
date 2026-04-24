# API 到源码解析覆盖矩阵

> 这一页用于追踪 `docs/api/**` 中的接口是否已经在源码解析中展开。`docs/api/**` 仍保持和 CHM 一致，只在确认原文或迁移错误时修正；源码解析页引用 API 作为接口契约，再解释实现、调用链和运行时边界。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| 已深入解析 | 已有专题段落，能说明源码入口、关键对象、调用链和边界。 |
| 部分覆盖 | 在主线中出现过，或解释了所属机制，但还没有逐接口源码走读。 |
| 未覆盖 | 当前源码解析目录还没有解释该 API 的实现或调用链。 |

当前矩阵按 API 文件维度维护。重复 API 名按所在文件分别统计，因为同名接口在不同组件里的语义可能不同。

## 总体快照

| 范围 | 数量 | 说明 |
| --- | ---: | --- |
| API 函数条目 | 255 | 来自 `docs/api/**` 的 `### def ...` 标题，不含索引页、审计页和 CHM 文件。 |
| 显式提及条目 | 121 | 机器初筛：接口名在 `docs/architecture/source-analysis/**` 中出现。 |
| 未显式提及条目 | 134 | 机器初筛：接口名没有出现在源码解析目录。 |

这个快照只做覆盖追踪，不代表最终质量判断。后续补文档时，应优先把“部分覆盖”和“未覆盖”项收敛为可链接的源码专题。

## 高优先级缺口

| 优先级 | 缺口 | 建议新增专题 |
| --- | --- | --- |
| P0 | `baseapp/KBEngine` 的实体创建、DB、原始 SQL、在线态查询 | `baseapp-kbengine-api.md` |
| P0 | `cellapp/Entity` 的 Trap、视野查询、传送、控制器回调 | `cell-entity-runtime-api.md` |
| P1 | `baseapp/Proxy` 的连接、文件/字符串流式传输、登录尝试回调 | `proxy-client-session-api.md` |
| P1 | `loginapp/interfaces/dbmgr/logger` 等非实体组件的脚本 API | `component-script-api.md` |
| P2 | watcher、文件描述符、`urlopen`、资源路径、app flags 等通用工具 API | `runtime-utility-api.md` |

## baseapp/Entity

API 页面：[docs/api/baseapp/Entity.md](/api/baseapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [createCellEntity](/api/baseapp/Entity.md#createCellEntity), [onGetCell](/api/baseapp/Entity.md#onGetCell) | [实体系统：Base -> Cell 交接](/architecture/source-analysis/entity-system.md#base-entity-create-cell) |
| 已深入解析 | [fireEvent](/api/baseapp/Entity.md#fireEvent), [registerEvent](/api/baseapp/Entity.md#registerEvent), [deregisterEvent](/api/baseapp/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 部分覆盖 | [addTimer](/api/baseapp/Entity.md#addTimer), [delTimer](/api/baseapp/Entity.md#delTimer), [onTimer](/api/baseapp/Entity.md#onTimer) | [脚本层定时器](/architecture/source-analysis/scripting.md#cell-entity-script-timers) 已解释实体定时器模型，但 Base 实体定时器还缺少独立源码链。 |
| 部分覆盖 | [destroy](/api/baseapp/Entity.md#destroy), [onDestroy](/api/baseapp/Entity.md#onDestroy) | 实体生命周期主线有涉及，缺少 Base 销毁路径专题。 |
| 部分覆盖 | [writeToDB](/api/baseapp/Entity.md#writeToDB), [onWriteToDB](/api/baseapp/Entity.md#onWriteToDB), [onRestore](/api/baseapp/Entity.md#onRestore) | [持久化与数据库](/architecture/source-analysis/persistence.md) |
| 部分覆盖 | [createCellEntityInNewSpace](/api/baseapp/Entity.md#createCellEntityInNewSpace) | [空间创建主线](/architecture/source-analysis/space-aoi.md)，缺少该 API 的参数和消息链细化。 |
| 未覆盖 | [destroyCellEntity](/api/baseapp/Entity.md#destroyCellEntity), [getComponent](/api/baseapp/Entity.md#getComponent), [onCreateCellFailure](/api/baseapp/Entity.md#onCreateCellFailure), [onLoseCell](/api/baseapp/Entity.md#onLoseCell), [onPreArchive](/api/baseapp/Entity.md#onPreArchive) | 需要补 Base 实体生命周期、组件访问和归档专题。 |

## baseapp/KBEngine

API 页面：[docs/api/baseapp/KBEngine.md](/api/baseapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseAppData](/api/baseapp/KBEngine.md#baseAppData), [globalData](/api/baseapp/KBEngine.md#globalData), [onBaseAppData](/api/baseapp/KBEngine.md#onBaseAppData), [onGlobalData](/api/baseapp/KBEngine.md#onGlobalData) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 部分覆盖 | [createEntity](/api/baseapp/KBEngine.md#createEntity), [createEntityFromDBID](/api/baseapp/KBEngine.md#createEntityFromDBID), [lookUpEntityByDBID](/api/baseapp/KBEngine.md#lookUpEntityByDBID) | [实体系统](/architecture/source-analysis/entity-system.md) 与 [持久化与数据库](/architecture/source-analysis/persistence.md) 有主线说明，但还不是逐 API 走读。 |
| 部分覆盖 | [MemoryStream](/api/baseapp/KBEngine.md#MemoryStream) | 网络和持久化页多处使用 `MemoryStream`，缺少独立 API 解释。 |
| 部分覆盖 | [getResFullPath](/api/baseapp/KBEngine.md#getResFullPath), [hasRes](/api/baseapp/KBEngine.md#hasRes), [listPathRes](/api/baseapp/KBEngine.md#listPathRes), [matchPath](/api/baseapp/KBEngine.md#matchPath), [open](/api/baseapp/KBEngine.md#open) | 脚本安装与资源路径有背景说明，缺少资源 API 专题。 |
| 部分覆盖 | [publish](/api/baseapp/KBEngine.md#publish), [reloadScript](/api/baseapp/KBEngine.md#reloadScript), [scriptLogType](/api/baseapp/KBEngine.md#scriptLogType), [time](/api/baseapp/KBEngine.md#time), [onInit](/api/baseapp/KBEngine.md#onInit), [onFini](/api/baseapp/KBEngine.md#onFini), [onBaseAppReady](/api/baseapp/KBEngine.md#onBaseAppReady) | [脚本运行时与热重载](/architecture/source-analysis/scripting.md) 和 [启动入口](/architecture/source-analysis/entry-and-bootstrap.md) 有机制说明，缺少 API 级落点表。 |
| 未覆盖 | [addWatcher](/api/baseapp/KBEngine.md#addWatcher), [delWatcher](/api/baseapp/KBEngine.md#delWatcher), [getWatcher](/api/baseapp/KBEngine.md#getWatcher), [getWatcherDir](/api/baseapp/KBEngine.md#getWatcherDir) | 需要补 watcher 系统。 |
| 未覆盖 | [address](/api/baseapp/KBEngine.md#address), [debugTracing](/api/baseapp/KBEngine.md#debugTracing), [getAppFlags](/api/baseapp/KBEngine.md#getAppFlags), [setAppFlags](/api/baseapp/KBEngine.md#setAppFlags), [isShuttingDown](/api/baseapp/KBEngine.md#isShuttingDown), [quantumPassedPercent](/api/baseapp/KBEngine.md#quantumPassedPercent), [genUUID64](/api/baseapp/KBEngine.md#genUUID64) | 需要补组件运行态工具 API。 |
| 未覆盖 | [charge](/api/baseapp/KBEngine.md#charge), [onLoseChargeCB](/api/baseapp/KBEngine.md#onLoseChargeCB) | 需要补充值/Interfaces 交互链。 |
| 未覆盖 | [createEntityAnywhere](/api/baseapp/KBEngine.md#createEntityAnywhere), [createEntityRemotely](/api/baseapp/KBEngine.md#createEntityRemotely), [createEntityAnywhereFromDBID](/api/baseapp/KBEngine.md#createEntityAnywhereFromDBID), [createEntityRemotelyFromDBID](/api/baseapp/KBEngine.md#createEntityRemotelyFromDBID), [createEntityLocally](/api/baseapp/KBEngine.md#createEntityLocally), [deleteEntityByDBID](/api/baseapp/KBEngine.md#deleteEntityByDBID) | 需要补 BaseApp 实体创建和 DBID 系列 API。 |
| 未覆盖 | [executeRawDatabaseCommand](/api/baseapp/KBEngine.md#executeRawDatabaseCommand) | 需要补原始数据库命令链路。 |
| 未覆盖 | [registerReadFileDescriptor](/api/baseapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/api/baseapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/api/baseapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/api/baseapp/KBEngine.md#deregisterWriteFileDescriptor) | 需要补 dispatcher 文件描述符回调。 |
| 未覆盖 | [urlopen](/api/baseapp/KBEngine.md#urlopen) | 需要补 HTTP 异步请求 API。 |
| 未覆盖 | [onBaseAppShutDown](/api/baseapp/KBEngine.md#onBaseAppShutDown), [onCellAppDeath](/api/baseapp/KBEngine.md#onCellAppDeath), [onBaseAppDataDel](/api/baseapp/KBEngine.md#onBaseAppDataDel), [onGlobalDataDel](/api/baseapp/KBEngine.md#onGlobalDataDel), [onReadyForLogin](/api/baseapp/KBEngine.md#onReadyForLogin), [onReadyForShutDown](/api/baseapp/KBEngine.md#onReadyForShutDown), [onAutoLoadEntityCreate](/api/baseapp/KBEngine.md#onAutoLoadEntityCreate) | 需要补 BaseApp 组件回调专题。 |

## baseapp/Proxy

API 页面：[docs/api/baseapp/Proxy.md](/api/baseapp/Proxy.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [giveClientTo](/api/baseapp/Proxy.md#giveClientTo), [onClientEnabled](/api/baseapp/Proxy.md#onClientEnabled) | [Proxy.giveClientTo 客户端迁移链](/architecture/source-analysis/space-aoi.md#proxy-give-client-to) |
| 部分覆盖 | [getClientType](/api/baseapp/Proxy.md#getClientType) | `giveClientTo` 链路中涉及客户端类型迁移，缺少单独 API 说明。 |
| 未覆盖 | [disconnect](/api/baseapp/Proxy.md#disconnect), [getClientDatas](/api/baseapp/Proxy.md#getClientDatas), [streamFileToClient](/api/baseapp/Proxy.md#streamFileToClient), [streamStringToClient](/api/baseapp/Proxy.md#streamStringToClient), [onClientDeath](/api/baseapp/Proxy.md#onClientDeath), [onClientGetCell](/api/baseapp/Proxy.md#onClientGetCell), [onGiveClientToFailure](/api/baseapp/Proxy.md#onGiveClientToFailure), [onLogOnAttempt](/api/baseapp/Proxy.md#onLogOnAttempt), [onStreamComplete](/api/baseapp/Proxy.md#onStreamComplete) | 需要补 Proxy 会话、流式传输和登录尝试专题。 |

## bots/Entity

API 页面：[docs/api/bots/Entity.md](/api/bots/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [moveToPoint](/api/bots/Entity.md#moveToPoint), [cancelController](/api/bots/Entity.md#cancelController), [isPlayer](/api/bots/Entity.md#isPlayer), [onEnterWorld](/api/bots/Entity.md#onEnterWorld), [onLeaveWorld](/api/bots/Entity.md#onLeaveWorld), [onEnterSpace](/api/bots/Entity.md#onEnterSpace), [onLeaveSpace](/api/bots/Entity.md#onLeaveSpace) | [客户端实体生命周期](/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) 与 [客户端本地移动控制器](/architecture/source-analysis/networking.md#client-entity-move-ground-sync) |
| 已深入解析 | [fireEvent](/api/bots/Entity.md#fireEvent), [registerEvent](/api/bots/Entity.md#registerEvent), [deregisterEvent](/api/bots/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 未覆盖 | [getComponent](/api/bots/Entity.md#getComponent) | 需要补实体组件访问 API。 |

## bots/KBEngine

API 页面：[docs/api/bots/KBEngine.md](/api/bots/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 部分覆盖 | [callback](/api/bots/KBEngine.md#callback), [onInit](/api/bots/KBEngine.md#onInit) | [脚本运行时](/architecture/source-analysis/scripting.md) 有回调和初始化机制说明，但 bots API 未展开。 |
| 部分覆盖 | [getWatcher](/api/bots/KBEngine.md#getWatcher), [getWatcherDir](/api/bots/KBEngine.md#getWatcherDir), [scriptLogType](/api/bots/KBEngine.md#scriptLogType) | 仅工具接口背景，缺少 bots 侧源码链。 |
| 未覆盖 | [addBots](/api/bots/KBEngine.md#addBots), [cancelCallback](/api/bots/KBEngine.md#cancelCallback), [genUUID64](/api/bots/KBEngine.md#genUUID64), [urlopen](/api/bots/KBEngine.md#urlopen), [onFinish](/api/bots/KBEngine.md#onFinish) | 需要补 bots 组件 API。 |

## bots/PyClientApp

API 页面：[docs/api/bots/PyClientApp.md](/api/bots/PyClientApp.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/api/bots/PyClientApp.md#player) | [客户端句柄表](/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 未覆盖 | [getSpaceData](/api/bots/PyClientApp.md#getSpaceData) | 需要补客户端/bots 空间数据 API。 |

## cellapp/Entity

API 页面：[docs/api/cellapp/Entity.md](/api/cellapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addTimer](/api/cellapp/Entity.md#addTimer), [delTimer](/api/cellapp/Entity.md#delTimer), [onTimer](/api/cellapp/Entity.md#onTimer) | [Cell 实体定时器](/architecture/source-analysis/scripting.md#cell-entity-script-timers) |
| 已深入解析 | [moveToPoint](/api/cellapp/Entity.md#moveToPoint), [moveToEntity](/api/cellapp/Entity.md#moveToEntity), [navigate](/api/cellapp/Entity.md#navigate), [navigatePathPoints](/api/cellapp/Entity.md#navigatePathPoints), [cancelController](/api/cellapp/Entity.md#cancelController) | [Cell 移动控制器链](/architecture/source-analysis/space-aoi.md#cell-entity-move-controllers) |
| 已深入解析 | [fireEvent](/api/cellapp/Entity.md#fireEvent), [registerEvent](/api/cellapp/Entity.md#registerEvent), [deregisterEvent](/api/cellapp/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [onEnterTrap](/api/cellapp/Entity.md#onEnterTrap), [onEnteredView](/api/cellapp/Entity.md#onEnteredView), [onLeaveTrap](/api/cellapp/Entity.md#onLeaveTrap), [onLoseControlledBy](/api/cellapp/Entity.md#onLoseControlledBy), [onMove](/api/cellapp/Entity.md#onMove), [onMoveOver](/api/cellapp/Entity.md#onMoveOver), [onMoveFailure](/api/cellapp/Entity.md#onMoveFailure), [onSpaceGone](/api/cellapp/Entity.md#onSpaceGone), [onTurn](/api/cellapp/Entity.md#onTurn), [onTeleport](/api/cellapp/Entity.md#onTeleport), [onTeleportFailure](/api/cellapp/Entity.md#onTeleportFailure), [onTeleportSuccess](/api/cellapp/Entity.md#onTeleportSuccess), [onUpdateBegin](/api/cellapp/Entity.md#onUpdateBegin), [onUpdateEnd](/api/cellapp/Entity.md#onUpdateEnd), [onWitnessed](/api/cellapp/Entity.md#onWitnessed) | [Cell Entity 回调触发点矩阵](/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) |
| 已深入解析 | [setViewRadius](/api/cellapp/Entity.md#setViewRadius), [getViewRadius](/api/cellapp/Entity.md#getViewRadius), [getViewHystArea](/api/cellapp/Entity.md#getViewHystArea), [entitiesInView](/api/cellapp/Entity.md#entitiesInView), [getWitnesses](/api/cellapp/Entity.md#getWitnesses) | [Witness 运行态 API](/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis) |
| 已深入解析 | [canNavigate](/api/cellapp/Entity.md#canNavigate), [destroySpace](/api/cellapp/Entity.md#destroySpace), [debugView](/api/cellapp/Entity.md#debugView), [entitiesInRange](/api/cellapp/Entity.md#entitiesInRange), [getRandomPoints](/api/cellapp/Entity.md#getRandomPoints) | [空间运行态 API](/architecture/source-analysis/space-aoi.md#cell-entity-space-runtime-apis) |
| 已深入解析 | [accelerate](/api/cellapp/Entity.md#accelerate), [getComponent](/api/cellapp/Entity.md#getComponent) | [控制器调速与组件访问 API](/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis) |
| 部分覆盖 | [clientEntity](/api/cellapp/Entity.md#clientEntity), [isReal](/api/cellapp/Entity.md#isReal), [onGetWitness](/api/cellapp/Entity.md#onGetWitness), [onLoseWitness](/api/cellapp/Entity.md#onLoseWitness), [onRestore](/api/cellapp/Entity.md#onRestore), [onWriteToDB](/api/cellapp/Entity.md#onWriteToDB), [writeToDB](/api/cellapp/Entity.md#writeToDB) | [空间/AOI](/architecture/source-analysis/space-aoi.md) 与 [持久化](/architecture/source-analysis/persistence.md) 有主线说明，缺少逐接口 API 小节。 |
| 部分覆盖 | [destroy](/api/cellapp/Entity.md#destroy), [onDestroy](/api/cellapp/Entity.md#onDestroy) | 实体生命周期有背景说明，缺少 Cell 销毁路径专题。 |
| 部分覆盖 | [addYawRotator](/api/cellapp/Entity.md#addYawRotator), [addProximity](/api/cellapp/Entity.md#addProximity), [teleport](/api/cellapp/Entity.md#teleport) | [Cell Entity 回调触发点矩阵](/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) 已解释它们触发的回调链，但还缺少独立 API 参数与返回值走读。 |

## cellapp/KBEngine

API 页面：[docs/api/cellapp/KBEngine.md](/api/cellapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [cellAppData](/api/cellapp/KBEngine.md#cellAppData), [globalData](/api/cellapp/KBEngine.md#globalData), [onCellAppData](/api/cellapp/KBEngine.md#onCellAppData), [onGlobalData](/api/cellapp/KBEngine.md#onGlobalData) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 部分覆盖 | [MemoryStream](/api/cellapp/KBEngine.md#MemoryStream), [createEntity](/api/cellapp/KBEngine.md#createEntity), [publish](/api/cellapp/KBEngine.md#publish), [reloadScript](/api/cellapp/KBEngine.md#reloadScript), [scriptLogType](/api/cellapp/KBEngine.md#scriptLogType), [time](/api/cellapp/KBEngine.md#time), [onInit](/api/cellapp/KBEngine.md#onInit), [onSpaceData](/api/cellapp/KBEngine.md#onSpaceData), [onSpaceGeometryLoaded](/api/cellapp/KBEngine.md#onSpaceGeometryLoaded) | 相关机制在 [脚本运行时](/architecture/source-analysis/scripting.md)、[空间/AOI](/architecture/source-analysis/space-aoi.md) 中出现，缺少 API 级源码段落。 |
| 部分覆盖 | [getResFullPath](/api/cellapp/KBEngine.md#getResFullPath), [hasRes](/api/cellapp/KBEngine.md#hasRes), [listPathRes](/api/cellapp/KBEngine.md#listPathRes), [matchPath](/api/cellapp/KBEngine.md#matchPath), [open](/api/cellapp/KBEngine.md#open) | 缺少资源 API 专题。 |
| 未覆盖 | [addSpaceGeometryMapping](/api/cellapp/KBEngine.md#addSpaceGeometryMapping), [delSpaceData](/api/cellapp/KBEngine.md#delSpaceData), [getSpaceData](/api/cellapp/KBEngine.md#getSpaceData), [getSpaceGeometryMapping](/api/cellapp/KBEngine.md#getSpaceGeometryMapping), [setSpaceData](/api/cellapp/KBEngine.md#setSpaceData), [onAllSpaceGeometryLoaded](/api/cellapp/KBEngine.md#onAllSpaceGeometryLoaded) | 需要补空间数据和空间几何 API。 |
| 未覆盖 | [addWatcher](/api/cellapp/KBEngine.md#addWatcher), [delWatcher](/api/cellapp/KBEngine.md#delWatcher), [getWatcher](/api/cellapp/KBEngine.md#getWatcher), [getWatcherDir](/api/cellapp/KBEngine.md#getWatcherDir) | 需要补 watcher 系统。 |
| 未覆盖 | [address](/api/cellapp/KBEngine.md#address), [debugTracing](/api/cellapp/KBEngine.md#debugTracing), [getAppFlags](/api/cellapp/KBEngine.md#getAppFlags), [setAppFlags](/api/cellapp/KBEngine.md#setAppFlags), [isShuttingDown](/api/cellapp/KBEngine.md#isShuttingDown), [genUUID64](/api/cellapp/KBEngine.md#genUUID64) | 需要补组件运行态工具 API。 |
| 未覆盖 | [executeRawDatabaseCommand](/api/cellapp/KBEngine.md#executeRawDatabaseCommand), [urlopen](/api/cellapp/KBEngine.md#urlopen), [raycast](/api/cellapp/KBEngine.md#raycast) | 需要补 DB、HTTP、物理查询 API。 |
| 未覆盖 | [registerReadFileDescriptor](/api/cellapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/api/cellapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/api/cellapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/api/cellapp/KBEngine.md#deregisterWriteFileDescriptor) | 需要补 dispatcher 文件描述符回调。 |
| 未覆盖 | [onCellAppDataDel](/api/cellapp/KBEngine.md#onCellAppDataDel), [onGlobalDataDel](/api/cellapp/KBEngine.md#onGlobalDataDel), [onReadyForLogin](/api/cellapp/KBEngine.md#onReadyForLogin) | 需要补 CellApp 组件回调专题。 |

## client/Entity

API 页面：[docs/api/client/Entity.md](/api/client/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseCall](/api/client/Entity.md#baseCall), [cellCall](/api/client/Entity.md#cellCall) | [客户端远程调用链](/architecture/source-analysis/networking.md#client-entity-remote-calls) |
| 已深入解析 | [isPlayer](/api/client/Entity.md#isPlayer), [onEnterWorld](/api/client/Entity.md#onEnterWorld), [onLeaveWorld](/api/client/Entity.md#onLeaveWorld), [onEnterSpace](/api/client/Entity.md#onEnterSpace), [onLeaveSpace](/api/client/Entity.md#onLeaveSpace) | [客户端实体生命周期](/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) |
| 已深入解析 | [fireEvent](/api/client/Entity.md#fireEvent), [registerEvent](/api/client/Entity.md#registerEvent), [deregisterEvent](/api/client/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 部分覆盖 | [onDestroy](/api/client/Entity.md#onDestroy) | 客户端实体离开世界和销毁链有涉及，缺少独立 API 小节。 |
| 未覆盖 | [getComponent](/api/client/Entity.md#getComponent) | 需要补实体组件访问 API。 |

## client/KBEngine

API 页面：[docs/api/client/KBEngine.md](/api/client/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/api/client/KBEngine.md#player), [findEntity](/api/client/KBEngine.md#findEntity) | [客户端句柄表](/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 部分覆盖 | [login](/api/client/KBEngine.md#login), [reloginBaseapp](/api/client/KBEngine.md#reloginBaseapp) | 登录链路和 SDK 事件总线有涉及，缺少客户端 API 逐步调用链。 |
| 未覆盖 | [createAccount](/api/client/KBEngine.md#createAccount), [resetPassword](/api/client/KBEngine.md#resetPassword), [bindAccountEmail](/api/client/KBEngine.md#bindAccountEmail), [newPassword](/api/client/KBEngine.md#newPassword), [getSpaceData](/api/client/KBEngine.md#getSpaceData) | 需要补客户端账号和空间数据 API。 |

## dbmgr/KBEngine

API 页面：[docs/api/dbmgr/KBEngine.md](/api/dbmgr/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 部分覆盖 | [addTimer](/api/dbmgr/KBEngine.md#addTimer), [delTimer](/api/dbmgr/KBEngine.md#delTimer) | 通用脚本定时器机制有说明，但 DBMgr 组件 API 未展开。 |
| 未覆盖 | [executeRawDatabaseCommand](/api/dbmgr/KBEngine.md#executeRawDatabaseCommand), [urlopen](/api/dbmgr/KBEngine.md#urlopen), [onDBMgrReady](/api/dbmgr/KBEngine.md#onDBMgrReady), [onDBMgrShutDown](/api/dbmgr/KBEngine.md#onDBMgrShutDown), [onReadyForShutDown](/api/dbmgr/KBEngine.md#onReadyForShutDown), [onSelectAccountDBInterface](/api/dbmgr/KBEngine.md#onSelectAccountDBInterface) | 需要补 DBMgr 脚本 API 和账号 DB 接口选择链。 |

## interfaces/KBEngine

API 页面：[docs/api/interfaces/KBEngine.md](/api/interfaces/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 部分覆盖 | [addTimer](/api/interfaces/KBEngine.md#addTimer), [delTimer](/api/interfaces/KBEngine.md#delTimer) | 通用脚本定时器机制有说明，但 Interfaces 组件 API 未展开。 |
| 未覆盖 | [accountLoginResponse](/api/interfaces/KBEngine.md#accountLoginResponse), [createAccountResponse](/api/interfaces/KBEngine.md#createAccountResponse), [chargeResponse](/api/interfaces/KBEngine.md#chargeResponse), [executeRawDatabaseCommand](/api/interfaces/KBEngine.md#executeRawDatabaseCommand), [urlopen](/api/interfaces/KBEngine.md#urlopen), [onInterfaceAppReady](/api/interfaces/KBEngine.md#onInterfaceAppReady), [onInterfaceAppShutDown](/api/interfaces/KBEngine.md#onInterfaceAppShutDown), [onRequestCreateAccount](/api/interfaces/KBEngine.md#onRequestCreateAccount), [onRequestAccountLogin](/api/interfaces/KBEngine.md#onRequestAccountLogin), [onRequestCharge](/api/interfaces/KBEngine.md#onRequestCharge) | 需要补 Interfaces 作为外部账号/充值接入组件的源码链。 |

## logger/KBEngine

API 页面：[docs/api/logger/KBEngine.md](/api/logger/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 部分覆盖 | [addTimer](/api/logger/KBEngine.md#addTimer), [delTimer](/api/logger/KBEngine.md#delTimer) | 通用脚本定时器机制有说明，但 Logger 组件 API 未展开。 |
| 未覆盖 | [urlopen](/api/logger/KBEngine.md#urlopen), [onLoggerAppReady](/api/logger/KBEngine.md#onLoggerAppReady), [onLoggerAppShutDown](/api/logger/KBEngine.md#onLoggerAppShutDown), [onLogWrote](/api/logger/KBEngine.md#onLogWrote), [onReadyForShutDown](/api/logger/KBEngine.md#onReadyForShutDown) | 需要补 Logger 脚本 API 和日志写入回调。 |

## loginapp/KBEngine

API 页面：[docs/api/loginapp/KBEngine.md](/api/loginapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 部分覆盖 | [addTimer](/api/loginapp/KBEngine.md#addTimer), [delTimer](/api/loginapp/KBEngine.md#delTimer) | 通用脚本定时器机制有说明，但 LoginApp 组件 API 未展开。 |
| 未覆盖 | [urlopen](/api/loginapp/KBEngine.md#urlopen), [onLoginAppReady](/api/loginapp/KBEngine.md#onLoginAppReady), [onLoginAppShutDown](/api/loginapp/KBEngine.md#onLoginAppShutDown), [onRequestLogin](/api/loginapp/KBEngine.md#onRequestLogin), [onLoginCallbackFromDB](/api/loginapp/KBEngine.md#onLoginCallbackFromDB), [onRequestCreateAccount](/api/loginapp/KBEngine.md#onRequestCreateAccount), [onCreateAccountCallbackFromDB](/api/loginapp/KBEngine.md#onCreateAccountCallbackFromDB) | 需要补 LoginApp 登录、创建账号、DB 回调链。 |

## 关键词与基础类型

API 页面：[docs/api/keywords.md](/api/keywords.md)、[docs/api/basetypes.md](/api/basetypes.md)

| 状态 | 项目 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | `EntityCall`, `entity`, `base`, `cell`, `client` | [实体系统](/architecture/source-analysis/entity-system.md)、[网络与消息系统](/architecture/source-analysis/networking.md) |
| 已深入解析 | `View`, `Witness`, `Space`, `real`, `ghost`, `cellapp`, `baseapp` | [空间与 AOI](/architecture/source-analysis/space-aoi.md)、[进程模型](/architecture/source-analysis/process-model.md) |
| 部分覆盖 | `entities.xml`, 基础数据类型, `ENTITYCALL`, `.def` 类型系统 | [实体系统](/architecture/source-analysis/entity-system.md)、[脚本运行时](/architecture/source-analysis/scripting.md) 有主线说明，但还缺少逐类型源码解析。 |
| 未覆盖 | `KBE_ROOT`, `KBE_RES_PATH`, `KBE_HYBRID_PATH`, `kbengine_defaults.xml`, `kbengine.xml`, `vector3` | 需要补配置、资源路径和 Math 类型专题。 |

## 后续维护规则

- 新增源码解析专题时，应同步更新本页，把对应接口从“未覆盖”移动到“部分覆盖”或“已深入解析”。
- 若发现 API 页与 CHM 不一致，只修 `docs/api/**` 的错误；不要在 API 页中扩写源码解析。
- 如果某个 API 只适合保留 CHM 原文、不值得展开源码专题，也应在本页标注原因，避免重复排查。
