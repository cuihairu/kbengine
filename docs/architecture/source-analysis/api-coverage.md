# API 到源码解析覆盖矩阵

> 这一页用于追踪 KBEngine API 中的接口是否已经在源码解析中展开。KBEngine API 入口是 `docs/api/kbengine/**`。KBEngine API 仍保持和 CHM 一致，只在确认原文或迁移错误时修正；源码解析页引用 API 作为接口契约，再解释实现、调用链和运行时边界。

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
| API 函数条目 | 255 | 来自 `docs/api/kbengine/**` 的 `### def ...` 标题，不含索引页、审计页和 CHM 文件。 |
| 显式提及条目 | 121 | 机器初筛：接口名在 `docs/architecture/source-analysis/**` 中出现。 |
| 未显式提及条目 | 134 | 机器初筛：接口名没有出现在源码解析目录。 |

这个快照只做覆盖追踪，不代表最终质量判断。后续补文档时，应优先把“部分覆盖”和“未覆盖”项收敛为可链接的源码专题。

## 高优先级缺口

| 优先级 | 缺口 | 建议新增专题 |
| --- | --- | --- |
| P2 | `client/KBEngine` 的 `resetPassword / bindAccountEmail / newPassword` 在核心 `client_lib` 的直注册入口边界 | `client-login-and-space-data-api.md` 补充段 |

## baseapp/Entity

API 页面：[docs/api/kbengine/baseapp/Entity.md](/api/kbengine/baseapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [createCellEntity](/api/kbengine/baseapp/Entity.md#createCellEntity), [onGetCell](/api/kbengine/baseapp/Entity.md#onGetCell) | [实体系统：Base -> Cell 交接](/architecture/source-analysis/entity-system.md#base-entity-create-cell) |
| 已深入解析 | [fireEvent](/api/kbengine/baseapp/Entity.md#fireEvent), [registerEvent](/api/kbengine/baseapp/Entity.md#registerEvent), [deregisterEvent](/api/kbengine/baseapp/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [addTimer](/api/kbengine/baseapp/Entity.md#addTimer), [delTimer](/api/kbengine/baseapp/Entity.md#delTimer), [onTimer](/api/kbengine/baseapp/Entity.md#onTimer) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [destroy](/api/kbengine/baseapp/Entity.md#destroy), [onDestroy](/api/kbengine/baseapp/Entity.md#onDestroy) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [writeToDB](/api/kbengine/baseapp/Entity.md#writeToDB), [onWriteToDB](/api/kbengine/baseapp/Entity.md#onWriteToDB), [onRestore](/api/kbengine/baseapp/Entity.md#onRestore), [onPreArchive](/api/kbengine/baseapp/Entity.md#onPreArchive) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) 与 [持久化与数据库](/architecture/source-analysis/persistence.md) |
| 已深入解析 | [createCellEntityInNewSpace](/api/kbengine/baseapp/Entity.md#createCellEntityInNewSpace), [destroyCellEntity](/api/kbengine/baseapp/Entity.md#destroyCellEntity), [onCreateCellFailure](/api/kbengine/baseapp/Entity.md#onCreateCellFailure), [onLoseCell](/api/kbengine/baseapp/Entity.md#onLoseCell) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [getComponent](/api/kbengine/baseapp/Entity.md#getComponent) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |

## baseapp/KBEngine

API 页面：[docs/api/kbengine/baseapp/KBEngine.md](/api/kbengine/baseapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseAppData](/api/kbengine/baseapp/KBEngine.md#baseAppData), [globalData](/api/kbengine/baseapp/KBEngine.md#globalData), [onBaseAppData](/api/kbengine/baseapp/KBEngine.md#onBaseAppData), [onGlobalData](/api/kbengine/baseapp/KBEngine.md#onGlobalData) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [createEntity](/api/kbengine/baseapp/KBEngine.md#createEntity), [createEntityLocally](/api/kbengine/baseapp/KBEngine.md#createEntityLocally), [createEntityAnywhere](/api/kbengine/baseapp/KBEngine.md#createEntityAnywhere), [createEntityRemotely](/api/kbengine/baseapp/KBEngine.md#createEntityRemotely), [createEntityFromDBID](/api/kbengine/baseapp/KBEngine.md#createEntityFromDBID), [createEntityAnywhereFromDBID](/api/kbengine/baseapp/KBEngine.md#createEntityAnywhereFromDBID), [createEntityRemotelyFromDBID](/api/kbengine/baseapp/KBEngine.md#createEntityRemotelyFromDBID), [deleteEntityByDBID](/api/kbengine/baseapp/KBEngine.md#deleteEntityByDBID), [lookUpEntityByDBID](/api/kbengine/baseapp/KBEngine.md#lookUpEntityByDBID) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [MemoryStream](/api/kbengine/baseapp/KBEngine.md#MemoryStream) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [getResFullPath](/api/kbengine/baseapp/KBEngine.md#getResFullPath), [hasRes](/api/kbengine/baseapp/KBEngine.md#hasRes), [listPathRes](/api/kbengine/baseapp/KBEngine.md#listPathRes), [matchPath](/api/kbengine/baseapp/KBEngine.md#matchPath), [open](/api/kbengine/baseapp/KBEngine.md#open) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [publish](/api/kbengine/baseapp/KBEngine.md#publish), [reloadScript](/api/kbengine/baseapp/KBEngine.md#reloadScript), [scriptLogType](/api/kbengine/baseapp/KBEngine.md#scriptLogType), [time](/api/kbengine/baseapp/KBEngine.md#time), [onInit](/api/kbengine/baseapp/KBEngine.md#onInit), [onBaseAppReady](/api/kbengine/baseapp/KBEngine.md#onBaseAppReady), [onReadyForLogin](/api/kbengine/baseapp/KBEngine.md#onReadyForLogin), [onReadyForShutDown](/api/kbengine/baseapp/KBEngine.md#onReadyForShutDown), [onBaseAppShutDown](/api/kbengine/baseapp/KBEngine.md#onBaseAppShutDown), [onCellAppDeath](/api/kbengine/baseapp/KBEngine.md#onCellAppDeath), [onAutoLoadEntityCreate](/api/kbengine/baseapp/KBEngine.md#onAutoLoadEntityCreate) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [addWatcher](/api/kbengine/baseapp/KBEngine.md#addWatcher), [delWatcher](/api/kbengine/baseapp/KBEngine.md#delWatcher), [getWatcher](/api/kbengine/baseapp/KBEngine.md#getWatcher), [getWatcherDir](/api/kbengine/baseapp/KBEngine.md#getWatcherDir) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [address](/api/kbengine/baseapp/KBEngine.md#address), [debugTracing](/api/kbengine/baseapp/KBEngine.md#debugTracing), [getAppFlags](/api/kbengine/baseapp/KBEngine.md#getAppFlags), [setAppFlags](/api/kbengine/baseapp/KBEngine.md#setAppFlags), [isShuttingDown](/api/kbengine/baseapp/KBEngine.md#isShuttingDown), [genUUID64](/api/kbengine/baseapp/KBEngine.md#genUUID64) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [quantumPassedPercent](/api/kbengine/baseapp/KBEngine.md#quantumPassedPercent) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [charge](/api/kbengine/baseapp/KBEngine.md#charge), [onLoseChargeCB](/api/kbengine/baseapp/KBEngine.md#onLoseChargeCB) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [executeRawDatabaseCommand](/api/kbengine/baseapp/KBEngine.md#executeRawDatabaseCommand) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [registerReadFileDescriptor](/api/kbengine/baseapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/api/kbengine/baseapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/api/kbengine/baseapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/api/kbengine/baseapp/KBEngine.md#deregisterWriteFileDescriptor) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [urlopen](/api/kbengine/baseapp/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [onBaseAppDataDel](/api/kbengine/baseapp/KBEngine.md#onBaseAppDataDel), [onGlobalDataDel](/api/kbengine/baseapp/KBEngine.md#onGlobalDataDel) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [onFini](/api/kbengine/baseapp/KBEngine.md#onFini) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) 已说明：当前源码树未查到明确触发点，实际可追关闭链是 `onReadyForShutDown` 与 `onBaseAppShutDown(state)`。 |

## baseapp/Proxy

API 页面：[docs/api/kbengine/baseapp/Proxy.md](/api/kbengine/baseapp/Proxy.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [giveClientTo](/api/kbengine/baseapp/Proxy.md#giveClientTo), [onClientEnabled](/api/kbengine/baseapp/Proxy.md#onClientEnabled) | [Proxy.giveClientTo 客户端迁移链](/architecture/source-analysis/space-aoi.md#proxy-give-client-to) |
| 已深入解析 | [getClientType](/api/kbengine/baseapp/Proxy.md#getClientType), [disconnect](/api/kbengine/baseapp/Proxy.md#disconnect), [getClientDatas](/api/kbengine/baseapp/Proxy.md#getClientDatas), [streamFileToClient](/api/kbengine/baseapp/Proxy.md#streamFileToClient), [streamStringToClient](/api/kbengine/baseapp/Proxy.md#streamStringToClient), [onClientDeath](/api/kbengine/baseapp/Proxy.md#onClientDeath), [onClientGetCell](/api/kbengine/baseapp/Proxy.md#onClientGetCell), [onGiveClientToFailure](/api/kbengine/baseapp/Proxy.md#onGiveClientToFailure), [onLogOnAttempt](/api/kbengine/baseapp/Proxy.md#onLogOnAttempt), [onStreamComplete](/api/kbengine/baseapp/Proxy.md#onStreamComplete) | [Proxy 会话与流式传输 API](/architecture/source-analysis/proxy-client-session-api.md) |

## bots/Entity

API 页面：[docs/api/kbengine/bots/Entity.md](/api/kbengine/bots/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [moveToPoint](/api/kbengine/bots/Entity.md#moveToPoint), [cancelController](/api/kbengine/bots/Entity.md#cancelController), [isPlayer](/api/kbengine/bots/Entity.md#isPlayer), [onEnterWorld](/api/kbengine/bots/Entity.md#onEnterWorld), [onLeaveWorld](/api/kbengine/bots/Entity.md#onLeaveWorld), [onEnterSpace](/api/kbengine/bots/Entity.md#onEnterSpace), [onLeaveSpace](/api/kbengine/bots/Entity.md#onLeaveSpace) | [客户端实体生命周期](/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) 与 [客户端本地移动控制器](/architecture/source-analysis/networking.md#client-entity-move-ground-sync) |
| 已深入解析 | [fireEvent](/api/kbengine/bots/Entity.md#fireEvent), [registerEvent](/api/kbengine/bots/Entity.md#registerEvent), [deregisterEvent](/api/kbengine/bots/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [getComponent](/api/kbengine/bots/Entity.md#getComponent) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |

## bots/KBEngine

API 页面：[docs/api/kbengine/bots/KBEngine.md](/api/kbengine/bots/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addBots](/api/kbengine/bots/KBEngine.md#addBots), [scriptLogType](/api/kbengine/bots/KBEngine.md#scriptLogType), [onInit](/api/kbengine/bots/KBEngine.md#onInit), [onFinish](/api/kbengine/bots/KBEngine.md#onFinish) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [callback](/api/kbengine/bots/KBEngine.md#callback), [cancelCallback](/api/kbengine/bots/KBEngine.md#cancelCallback), [getWatcher](/api/kbengine/bots/KBEngine.md#getWatcher), [getWatcherDir](/api/kbengine/bots/KBEngine.md#getWatcherDir), [urlopen](/api/kbengine/bots/KBEngine.md#urlopen), [genUUID64](/api/kbengine/bots/KBEngine.md#genUUID64) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## bots/PyClientApp

API 页面：[docs/api/kbengine/bots/PyClientApp.md](/api/kbengine/bots/PyClientApp.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/api/kbengine/bots/PyClientApp.md#player) | [客户端句柄表](/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 已深入解析 | [getSpaceData](/api/kbengine/bots/PyClientApp.md#getSpaceData) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |

## cellapp/Entity

API 页面：[docs/api/kbengine/cellapp/Entity.md](/api/kbengine/cellapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addTimer](/api/kbengine/cellapp/Entity.md#addTimer), [delTimer](/api/kbengine/cellapp/Entity.md#delTimer), [onTimer](/api/kbengine/cellapp/Entity.md#onTimer) | [Cell 实体定时器](/architecture/source-analysis/scripting.md#cell-entity-script-timers) |
| 已深入解析 | [moveToPoint](/api/kbengine/cellapp/Entity.md#moveToPoint), [moveToEntity](/api/kbengine/cellapp/Entity.md#moveToEntity), [navigate](/api/kbengine/cellapp/Entity.md#navigate), [navigatePathPoints](/api/kbengine/cellapp/Entity.md#navigatePathPoints), [cancelController](/api/kbengine/cellapp/Entity.md#cancelController) | [Cell 移动控制器链](/architecture/source-analysis/space-aoi.md#cell-entity-move-controllers) |
| 已深入解析 | [fireEvent](/api/kbengine/cellapp/Entity.md#fireEvent), [registerEvent](/api/kbengine/cellapp/Entity.md#registerEvent), [deregisterEvent](/api/kbengine/cellapp/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [onEnterTrap](/api/kbengine/cellapp/Entity.md#onEnterTrap), [onEnteredView](/api/kbengine/cellapp/Entity.md#onEnteredView), [onLeaveTrap](/api/kbengine/cellapp/Entity.md#onLeaveTrap), [onLoseControlledBy](/api/kbengine/cellapp/Entity.md#onLoseControlledBy), [onMove](/api/kbengine/cellapp/Entity.md#onMove), [onMoveOver](/api/kbengine/cellapp/Entity.md#onMoveOver), [onMoveFailure](/api/kbengine/cellapp/Entity.md#onMoveFailure), [onSpaceGone](/api/kbengine/cellapp/Entity.md#onSpaceGone), [onTurn](/api/kbengine/cellapp/Entity.md#onTurn), [onTeleport](/api/kbengine/cellapp/Entity.md#onTeleport), [onTeleportFailure](/api/kbengine/cellapp/Entity.md#onTeleportFailure), [onTeleportSuccess](/api/kbengine/cellapp/Entity.md#onTeleportSuccess), [onUpdateBegin](/api/kbengine/cellapp/Entity.md#onUpdateBegin), [onUpdateEnd](/api/kbengine/cellapp/Entity.md#onUpdateEnd), [onWitnessed](/api/kbengine/cellapp/Entity.md#onWitnessed) | [Cell Entity 回调触发点矩阵](/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) |
| 已深入解析 | [setViewRadius](/api/kbengine/cellapp/Entity.md#setViewRadius), [getViewRadius](/api/kbengine/cellapp/Entity.md#getViewRadius), [getViewHystArea](/api/kbengine/cellapp/Entity.md#getViewHystArea), [entitiesInView](/api/kbengine/cellapp/Entity.md#entitiesInView), [getWitnesses](/api/kbengine/cellapp/Entity.md#getWitnesses) | [Witness 运行态 API](/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis) |
| 已深入解析 | [canNavigate](/api/kbengine/cellapp/Entity.md#canNavigate), [destroySpace](/api/kbengine/cellapp/Entity.md#destroySpace), [debugView](/api/kbengine/cellapp/Entity.md#debugView), [entitiesInRange](/api/kbengine/cellapp/Entity.md#entitiesInRange), [getRandomPoints](/api/kbengine/cellapp/Entity.md#getRandomPoints) | [空间运行态 API](/architecture/source-analysis/space-aoi.md#cell-entity-space-runtime-apis) |
| 已深入解析 | [accelerate](/api/kbengine/cellapp/Entity.md#accelerate), [getComponent](/api/kbengine/cellapp/Entity.md#getComponent) | [控制器调速与组件访问 API](/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis) |
| 已深入解析 | [clientEntity](/api/kbengine/cellapp/Entity.md#clientEntity), [isReal](/api/kbengine/cellapp/Entity.md#isReal), [onGetWitness](/api/kbengine/cellapp/Entity.md#onGetWitness), [onLoseWitness](/api/kbengine/cellapp/Entity.md#onLoseWitness) | [Witness 运行态 API](/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis) |
| 已深入解析 | [destroy](/api/kbengine/cellapp/Entity.md#destroy), [onDestroy](/api/kbengine/cellapp/Entity.md#onDestroy) | [Cell 销毁与摘链](/architecture/source-analysis/space-aoi.md#cell-entity-destroy-lifecycle) |
| 已深入解析 | [onRestore](/api/kbengine/cellapp/Entity.md#onRestore), [onWriteToDB](/api/kbengine/cellapp/Entity.md#onWriteToDB), [writeToDB](/api/kbengine/cellapp/Entity.md#writeToDB) | [持久化与数据库](/architecture/source-analysis/persistence.md) |
| 已深入解析 | [addYawRotator](/api/kbengine/cellapp/Entity.md#addYawRotator), [addProximity](/api/kbengine/cellapp/Entity.md#addProximity), [teleport](/api/kbengine/cellapp/Entity.md#teleport) | [控制器调速与组件访问 API](/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis) 与 [Cell Entity 回调触发点矩阵](/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) |

## cellapp/KBEngine

API 页面：[docs/api/kbengine/cellapp/KBEngine.md](/api/kbengine/cellapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [cellAppData](/api/kbengine/cellapp/KBEngine.md#cellAppData), [globalData](/api/kbengine/cellapp/KBEngine.md#globalData), [onCellAppData](/api/kbengine/cellapp/KBEngine.md#onCellAppData), [onGlobalData](/api/kbengine/cellapp/KBEngine.md#onGlobalData) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [MemoryStream](/api/kbengine/cellapp/KBEngine.md#MemoryStream), [createEntity](/api/kbengine/cellapp/KBEngine.md#createEntity), [publish](/api/kbengine/cellapp/KBEngine.md#publish), [reloadScript](/api/kbengine/cellapp/KBEngine.md#reloadScript), [scriptLogType](/api/kbengine/cellapp/KBEngine.md#scriptLogType), [time](/api/kbengine/cellapp/KBEngine.md#time), [onInit](/api/kbengine/cellapp/KBEngine.md#onInit), [onSpaceData](/api/kbengine/cellapp/KBEngine.md#onSpaceData), [onSpaceGeometryLoaded](/api/kbengine/cellapp/KBEngine.md#onSpaceGeometryLoaded), [onAllSpaceGeometryLoaded](/api/kbengine/cellapp/KBEngine.md#onAllSpaceGeometryLoaded) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [getResFullPath](/api/kbengine/cellapp/KBEngine.md#getResFullPath), [hasRes](/api/kbengine/cellapp/KBEngine.md#hasRes), [listPathRes](/api/kbengine/cellapp/KBEngine.md#listPathRes), [matchPath](/api/kbengine/cellapp/KBEngine.md#matchPath), [open](/api/kbengine/cellapp/KBEngine.md#open) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [addSpaceGeometryMapping](/api/kbengine/cellapp/KBEngine.md#addSpaceGeometryMapping), [delSpaceData](/api/kbengine/cellapp/KBEngine.md#delSpaceData), [getSpaceData](/api/kbengine/cellapp/KBEngine.md#getSpaceData), [getSpaceGeometryMapping](/api/kbengine/cellapp/KBEngine.md#getSpaceGeometryMapping), [setSpaceData](/api/kbengine/cellapp/KBEngine.md#setSpaceData) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [addWatcher](/api/kbengine/cellapp/KBEngine.md#addWatcher), [delWatcher](/api/kbengine/cellapp/KBEngine.md#delWatcher), [getWatcher](/api/kbengine/cellapp/KBEngine.md#getWatcher), [getWatcherDir](/api/kbengine/cellapp/KBEngine.md#getWatcherDir) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [address](/api/kbengine/cellapp/KBEngine.md#address), [debugTracing](/api/kbengine/cellapp/KBEngine.md#debugTracing), [getAppFlags](/api/kbengine/cellapp/KBEngine.md#getAppFlags), [setAppFlags](/api/kbengine/cellapp/KBEngine.md#setAppFlags), [isShuttingDown](/api/kbengine/cellapp/KBEngine.md#isShuttingDown), [genUUID64](/api/kbengine/cellapp/KBEngine.md#genUUID64) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [executeRawDatabaseCommand](/api/kbengine/cellapp/KBEngine.md#executeRawDatabaseCommand), [raycast](/api/kbengine/cellapp/KBEngine.md#raycast) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [urlopen](/api/kbengine/cellapp/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [registerReadFileDescriptor](/api/kbengine/cellapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/api/kbengine/cellapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/api/kbengine/cellapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/api/kbengine/cellapp/KBEngine.md#deregisterWriteFileDescriptor) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [onCellAppDataDel](/api/kbengine/cellapp/KBEngine.md#onCellAppDataDel), [onGlobalDataDel](/api/kbengine/cellapp/KBEngine.md#onGlobalDataDel), [onReadyForLogin](/api/kbengine/cellapp/KBEngine.md#onReadyForLogin) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |

## client/Entity

API 页面：[docs/api/kbengine/client/Entity.md](/api/kbengine/client/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseCall](/api/kbengine/client/Entity.md#baseCall), [cellCall](/api/kbengine/client/Entity.md#cellCall) | [客户端远程调用链](/architecture/source-analysis/networking.md#client-entity-remote-calls) |
| 已深入解析 | [isPlayer](/api/kbengine/client/Entity.md#isPlayer), [onEnterWorld](/api/kbengine/client/Entity.md#onEnterWorld), [onLeaveWorld](/api/kbengine/client/Entity.md#onLeaveWorld), [onEnterSpace](/api/kbengine/client/Entity.md#onEnterSpace), [onLeaveSpace](/api/kbengine/client/Entity.md#onLeaveSpace) | [客户端实体生命周期](/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) |
| 已深入解析 | [fireEvent](/api/kbengine/client/Entity.md#fireEvent), [registerEvent](/api/kbengine/client/Entity.md#registerEvent), [deregisterEvent](/api/kbengine/client/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [onDestroy](/api/kbengine/client/Entity.md#onDestroy) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |
| 已深入解析 | [getComponent](/api/kbengine/client/Entity.md#getComponent) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |

## client/KBEngine

API 页面：[docs/api/kbengine/client/KBEngine.md](/api/kbengine/client/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/api/kbengine/client/KBEngine.md#player), [findEntity](/api/kbengine/client/KBEngine.md#findEntity) | [客户端句柄表](/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 已深入解析 | [login](/api/kbengine/client/KBEngine.md#login), [reloginBaseapp](/api/kbengine/client/KBEngine.md#reloginBaseapp) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |
| 已深入解析 | [createAccount](/api/kbengine/client/KBEngine.md#createAccount), [getSpaceData](/api/kbengine/client/KBEngine.md#getSpaceData) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |
| 部分覆盖 | [resetPassword](/api/kbengine/client/KBEngine.md#resetPassword), [bindAccountEmail](/api/kbengine/client/KBEngine.md#bindAccountEmail), [newPassword](/api/kbengine/client/KBEngine.md#newPassword) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) 已解释服务端链、客户端回包与 SDK 模板对照，但当前核心 `client_lib` 里仍未看到直接 Python 模块注册点。 |

## dbmgr/KBEngine

API 页面：[docs/api/kbengine/dbmgr/KBEngine.md](/api/kbengine/dbmgr/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [executeRawDatabaseCommand](/api/kbengine/dbmgr/KBEngine.md#executeRawDatabaseCommand), [onDBMgrReady](/api/kbengine/dbmgr/KBEngine.md#onDBMgrReady), [onDBMgrShutDown](/api/kbengine/dbmgr/KBEngine.md#onDBMgrShutDown), [onReadyForShutDown](/api/kbengine/dbmgr/KBEngine.md#onReadyForShutDown), [onSelectAccountDBInterface](/api/kbengine/dbmgr/KBEngine.md#onSelectAccountDBInterface) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/kbengine/dbmgr/KBEngine.md#addTimer), [delTimer](/api/kbengine/dbmgr/KBEngine.md#delTimer), [urlopen](/api/kbengine/dbmgr/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## interfaces/KBEngine

API 页面：[docs/api/kbengine/interfaces/KBEngine.md](/api/kbengine/interfaces/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [accountLoginResponse](/api/kbengine/interfaces/KBEngine.md#accountLoginResponse), [createAccountResponse](/api/kbengine/interfaces/KBEngine.md#createAccountResponse), [chargeResponse](/api/kbengine/interfaces/KBEngine.md#chargeResponse), [executeRawDatabaseCommand](/api/kbengine/interfaces/KBEngine.md#executeRawDatabaseCommand), [onInterfaceAppReady](/api/kbengine/interfaces/KBEngine.md#onInterfaceAppReady), [onInterfaceAppShutDown](/api/kbengine/interfaces/KBEngine.md#onInterfaceAppShutDown), [onRequestCreateAccount](/api/kbengine/interfaces/KBEngine.md#onRequestCreateAccount), [onRequestAccountLogin](/api/kbengine/interfaces/KBEngine.md#onRequestAccountLogin), [onRequestCharge](/api/kbengine/interfaces/KBEngine.md#onRequestCharge) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/kbengine/interfaces/KBEngine.md#addTimer), [delTimer](/api/kbengine/interfaces/KBEngine.md#delTimer), [urlopen](/api/kbengine/interfaces/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## logger/KBEngine

API 页面：[docs/api/kbengine/logger/KBEngine.md](/api/kbengine/logger/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoggerAppReady](/api/kbengine/logger/KBEngine.md#onLoggerAppReady), [onLoggerAppShutDown](/api/kbengine/logger/KBEngine.md#onLoggerAppShutDown), [onLogWrote](/api/kbengine/logger/KBEngine.md#onLogWrote), [onReadyForShutDown](/api/kbengine/logger/KBEngine.md#onReadyForShutDown) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/kbengine/logger/KBEngine.md#addTimer), [delTimer](/api/kbengine/logger/KBEngine.md#delTimer), [urlopen](/api/kbengine/logger/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## loginapp/KBEngine

API 页面：[docs/api/kbengine/loginapp/KBEngine.md](/api/kbengine/loginapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoginAppReady](/api/kbengine/loginapp/KBEngine.md#onLoginAppReady), [onLoginAppShutDown](/api/kbengine/loginapp/KBEngine.md#onLoginAppShutDown), [onRequestLogin](/api/kbengine/loginapp/KBEngine.md#onRequestLogin), [onLoginCallbackFromDB](/api/kbengine/loginapp/KBEngine.md#onLoginCallbackFromDB), [onRequestCreateAccount](/api/kbengine/loginapp/KBEngine.md#onRequestCreateAccount), [onCreateAccountCallbackFromDB](/api/kbengine/loginapp/KBEngine.md#onCreateAccountCallbackFromDB) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/kbengine/loginapp/KBEngine.md#addTimer), [delTimer](/api/kbengine/loginapp/KBEngine.md#delTimer), [urlopen](/api/kbengine/loginapp/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## 关键词与基础类型

API 页面：[docs/api/kbengine/keywords.md](/api/kbengine/keywords.md)、[docs/api/kbengine/basetypes.md](/api/kbengine/basetypes.md)

| 状态 | 项目 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | `EntityCall`, `entity`, `base`, `cell`, `client` | [实体系统](/architecture/source-analysis/entity-system.md)、[网络与消息系统](/architecture/source-analysis/networking.md) |
| 已深入解析 | `View`, `Witness`, `Space`, `real`, `ghost`, `cellapp`, `baseapp` | [空间与 AOI](/architecture/source-analysis/space-aoi.md)、[进程模型](/architecture/source-analysis/process-model.md) |
| 已深入解析 | `entities.xml`, 基础数据类型, `ENTITYCALL`, `.def` 类型系统 | [类型系统与实体定义文件](/architecture/source-analysis/entitydef-type-system.md) |
| 已深入解析 | `KBE_ROOT`, `KBE_RES_PATH`, `kbengine_defaults.xml`, `kbengine.xml`, `vector3` | [运行时配置与基础类型](/architecture/source-analysis/runtime-config-and-types.md) |
| 已深入解析 | `KBE_HYBRID_PATH` | [运行时配置与基础类型](/architecture/source-analysis/runtime-config-and-types.md) 已说明：当前源码树未命中该名字，实际贯穿实现的是 `KBE_BIN_PATH`。 |

## 后续维护规则

- 新增源码解析专题时，应同步更新本页，把对应接口从“未覆盖”移动到“部分覆盖”或“已深入解析”。
- 若发现 KBEngine API 页与 CHM 不一致，只修 `docs/api/kbengine/**` 的错误；不要在 API 页中扩写源码解析。
- 如果某个 API 只适合保留 CHM 原文、不值得展开源码专题，也应在本页标注原因，避免重复排查。
