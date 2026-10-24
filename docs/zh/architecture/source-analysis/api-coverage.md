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

API 页面：[docs/api/kbengine/baseapp/Entity.md](/zh/api/kbengine/baseapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [createCellEntity](/zh/api/kbengine/baseapp/Entity.md#createCellEntity), [onGetCell](/zh/api/kbengine/baseapp/Entity.md#onGetCell) | [实体系统：Base -> Cell 交接](/zh/architecture/source-analysis/entity-system.md#base-entity-create-cell) |
| 已深入解析 | [fireEvent](/zh/api/kbengine/baseapp/Entity.md#fireEvent), [registerEvent](/zh/api/kbengine/baseapp/Entity.md#registerEvent), [deregisterEvent](/zh/api/kbengine/baseapp/Entity.md#deregisterEvent) | [事件系统](/zh/architecture/source-analysis/events.md) |
| 已深入解析 | [addTimer](/zh/api/kbengine/baseapp/Entity.md#addTimer), [delTimer](/zh/api/kbengine/baseapp/Entity.md#delTimer), [onTimer](/zh/api/kbengine/baseapp/Entity.md#onTimer) | [Base 实体生命周期](/zh/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [destroy](/zh/api/kbengine/baseapp/Entity.md#destroy), [onDestroy](/zh/api/kbengine/baseapp/Entity.md#onDestroy) | [Base 实体生命周期](/zh/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [writeToDB](/zh/api/kbengine/baseapp/Entity.md#writeToDB), [onWriteToDB](/zh/api/kbengine/baseapp/Entity.md#onWriteToDB), [onRestore](/zh/api/kbengine/baseapp/Entity.md#onRestore), [onPreArchive](/zh/api/kbengine/baseapp/Entity.md#onPreArchive) | [Base 实体生命周期](/zh/architecture/source-analysis/base-entity-lifecycle.md) 与 [持久化与数据库](/zh/architecture/source-analysis/persistence.md) |
| 已深入解析 | [createCellEntityInNewSpace](/zh/api/kbengine/baseapp/Entity.md#createCellEntityInNewSpace), [destroyCellEntity](/zh/api/kbengine/baseapp/Entity.md#destroyCellEntity), [onCreateCellFailure](/zh/api/kbengine/baseapp/Entity.md#onCreateCellFailure), [onLoseCell](/zh/api/kbengine/baseapp/Entity.md#onLoseCell) | [Base 实体生命周期](/zh/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [getComponent](/zh/api/kbengine/baseapp/Entity.md#getComponent) | [Base 实体生命周期](/zh/architecture/source-analysis/base-entity-lifecycle.md) |

## baseapp/KBEngine

API 页面：[docs/api/kbengine/baseapp/KBEngine.md](/zh/api/kbengine/baseapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseAppData](/zh/api/kbengine/baseapp/KBEngine.md#baseAppData), [globalData](/zh/api/kbengine/baseapp/KBEngine.md#globalData), [onBaseAppData](/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppData), [onGlobalData](/zh/api/kbengine/baseapp/KBEngine.md#onGlobalData) | [全局数据同步链](/zh/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [createEntity](/zh/api/kbengine/baseapp/KBEngine.md#createEntity), [createEntityLocally](/zh/api/kbengine/baseapp/KBEngine.md#createEntityLocally), [createEntityAnywhere](/zh/api/kbengine/baseapp/KBEngine.md#createEntityAnywhere), [createEntityRemotely](/zh/api/kbengine/baseapp/KBEngine.md#createEntityRemotely), [createEntityFromDBID](/zh/api/kbengine/baseapp/KBEngine.md#createEntityFromDBID), [createEntityAnywhereFromDBID](/zh/api/kbengine/baseapp/KBEngine.md#createEntityAnywhereFromDBID), [createEntityRemotelyFromDBID](/zh/api/kbengine/baseapp/KBEngine.md#createEntityRemotelyFromDBID), [deleteEntityByDBID](/zh/api/kbengine/baseapp/KBEngine.md#deleteEntityByDBID), [lookUpEntityByDBID](/zh/api/kbengine/baseapp/KBEngine.md#lookUpEntityByDBID) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [MemoryStream](/zh/api/kbengine/baseapp/KBEngine.md#MemoryStream) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [getResFullPath](/zh/api/kbengine/baseapp/KBEngine.md#getResFullPath), [hasRes](/zh/api/kbengine/baseapp/KBEngine.md#hasRes), [listPathRes](/zh/api/kbengine/baseapp/KBEngine.md#listPathRes), [matchPath](/zh/api/kbengine/baseapp/KBEngine.md#matchPath), [open](/zh/api/kbengine/baseapp/KBEngine.md#open) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [publish](/zh/api/kbengine/baseapp/KBEngine.md#publish), [reloadScript](/zh/api/kbengine/baseapp/KBEngine.md#reloadScript), [scriptLogType](/zh/api/kbengine/baseapp/KBEngine.md#scriptLogType), [time](/zh/api/kbengine/baseapp/KBEngine.md#time), [onInit](/zh/api/kbengine/baseapp/KBEngine.md#onInit), [onBaseAppReady](/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppReady), [onReadyForLogin](/zh/api/kbengine/baseapp/KBEngine.md#onReadyForLogin), [onReadyForShutDown](/zh/api/kbengine/baseapp/KBEngine.md#onReadyForShutDown), [onBaseAppShutDown](/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppShutDown), [onCellAppDeath](/zh/api/kbengine/baseapp/KBEngine.md#onCellAppDeath), [onAutoLoadEntityCreate](/zh/api/kbengine/baseapp/KBEngine.md#onAutoLoadEntityCreate) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [addWatcher](/zh/api/kbengine/baseapp/KBEngine.md#addWatcher), [delWatcher](/zh/api/kbengine/baseapp/KBEngine.md#delWatcher), [getWatcher](/zh/api/kbengine/baseapp/KBEngine.md#getWatcher), [getWatcherDir](/zh/api/kbengine/baseapp/KBEngine.md#getWatcherDir) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [address](/zh/api/kbengine/baseapp/KBEngine.md#address), [debugTracing](/zh/api/kbengine/baseapp/KBEngine.md#debugTracing), [getAppFlags](/zh/api/kbengine/baseapp/KBEngine.md#getAppFlags), [setAppFlags](/zh/api/kbengine/baseapp/KBEngine.md#setAppFlags), [isShuttingDown](/zh/api/kbengine/baseapp/KBEngine.md#isShuttingDown), [genUUID64](/zh/api/kbengine/baseapp/KBEngine.md#genUUID64) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [quantumPassedPercent](/zh/api/kbengine/baseapp/KBEngine.md#quantumPassedPercent) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [charge](/zh/api/kbengine/baseapp/KBEngine.md#charge), [onLoseChargeCB](/zh/api/kbengine/baseapp/KBEngine.md#onLoseChargeCB) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [executeRawDatabaseCommand](/zh/api/kbengine/baseapp/KBEngine.md#executeRawDatabaseCommand) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [registerReadFileDescriptor](/zh/api/kbengine/baseapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/zh/api/kbengine/baseapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/zh/api/kbengine/baseapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/zh/api/kbengine/baseapp/KBEngine.md#deregisterWriteFileDescriptor) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [urlopen](/zh/api/kbengine/baseapp/KBEngine.md#urlopen) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [onBaseAppDataDel](/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppDataDel), [onGlobalDataDel](/zh/api/kbengine/baseapp/KBEngine.md#onGlobalDataDel) | [全局数据同步链](/zh/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [onFini](/zh/api/kbengine/baseapp/KBEngine.md#onFini) | [BaseApp 运行时 API](/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md) 已说明：当前源码树未查到明确触发点，实际可追关闭链是 `onReadyForShutDown` 与 `onBaseAppShutDown(state)`。 |

## baseapp/Proxy

API 页面：[docs/api/kbengine/baseapp/Proxy.md](/zh/api/kbengine/baseapp/Proxy.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [giveClientTo](/zh/api/kbengine/baseapp/Proxy.md#giveClientTo), [onClientEnabled](/zh/api/kbengine/baseapp/Proxy.md#onClientEnabled) | [Proxy.giveClientTo 客户端迁移链](/zh/architecture/source-analysis/space-aoi.md#proxy-give-client-to) |
| 已深入解析 | [getClientType](/zh/api/kbengine/baseapp/Proxy.md#getClientType), [disconnect](/zh/api/kbengine/baseapp/Proxy.md#disconnect), [getClientDatas](/zh/api/kbengine/baseapp/Proxy.md#getClientDatas), [streamFileToClient](/zh/api/kbengine/baseapp/Proxy.md#streamFileToClient), [streamStringToClient](/zh/api/kbengine/baseapp/Proxy.md#streamStringToClient), [onClientDeath](/zh/api/kbengine/baseapp/Proxy.md#onClientDeath), [onClientGetCell](/zh/api/kbengine/baseapp/Proxy.md#onClientGetCell), [onGiveClientToFailure](/zh/api/kbengine/baseapp/Proxy.md#onGiveClientToFailure), [onLogOnAttempt](/zh/api/kbengine/baseapp/Proxy.md#onLogOnAttempt), [onStreamComplete](/zh/api/kbengine/baseapp/Proxy.md#onStreamComplete) | [Proxy 会话与流式传输 API](/zh/architecture/source-analysis/proxy-client-session-api.md) |

## bots/Entity

API 页面：[docs/api/kbengine/bots/Entity.md](/zh/api/kbengine/bots/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [moveToPoint](/zh/api/kbengine/bots/Entity.md#moveToPoint), [cancelController](/zh/api/kbengine/bots/Entity.md#cancelController), [isPlayer](/zh/api/kbengine/bots/Entity.md#isPlayer), [onEnterWorld](/zh/api/kbengine/bots/Entity.md#onEnterWorld), [onLeaveWorld](/zh/api/kbengine/bots/Entity.md#onLeaveWorld), [onEnterSpace](/zh/api/kbengine/bots/Entity.md#onEnterSpace), [onLeaveSpace](/zh/api/kbengine/bots/Entity.md#onLeaveSpace) | [客户端实体生命周期](/zh/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) 与 [客户端本地移动控制器](/zh/architecture/source-analysis/networking.md#client-entity-move-ground-sync) |
| 已深入解析 | [fireEvent](/zh/api/kbengine/bots/Entity.md#fireEvent), [registerEvent](/zh/api/kbengine/bots/Entity.md#registerEvent), [deregisterEvent](/zh/api/kbengine/bots/Entity.md#deregisterEvent) | [事件系统](/zh/architecture/source-analysis/events.md) |
| 已深入解析 | [getComponent](/zh/api/kbengine/bots/Entity.md#getComponent) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) |

## bots/KBEngine

API 页面：[docs/api/kbengine/bots/KBEngine.md](/zh/api/kbengine/bots/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addBots](/zh/api/kbengine/bots/KBEngine.md#addBots), [scriptLogType](/zh/api/kbengine/bots/KBEngine.md#scriptLogType), [onInit](/zh/api/kbengine/bots/KBEngine.md#onInit), [onFinish](/zh/api/kbengine/bots/KBEngine.md#onFinish) | [组件型脚本 API](/zh/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [callback](/zh/api/kbengine/bots/KBEngine.md#callback), [cancelCallback](/zh/api/kbengine/bots/KBEngine.md#cancelCallback), [getWatcher](/zh/api/kbengine/bots/KBEngine.md#getWatcher), [getWatcherDir](/zh/api/kbengine/bots/KBEngine.md#getWatcherDir), [urlopen](/zh/api/kbengine/bots/KBEngine.md#urlopen), [genUUID64](/zh/api/kbengine/bots/KBEngine.md#genUUID64) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |

## bots/PyClientApp

API 页面：[docs/api/kbengine/bots/PyClientApp.md](/zh/api/kbengine/bots/PyClientApp.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/zh/api/kbengine/bots/PyClientApp.md#player) | [客户端句柄表](/zh/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 已深入解析 | [getSpaceData](/zh/api/kbengine/bots/PyClientApp.md#getSpaceData) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) |

## cellapp/Entity

API 页面：[docs/api/kbengine/cellapp/Entity.md](/zh/api/kbengine/cellapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addTimer](/zh/api/kbengine/cellapp/Entity.md#addTimer), [delTimer](/zh/api/kbengine/cellapp/Entity.md#delTimer), [onTimer](/zh/api/kbengine/cellapp/Entity.md#onTimer) | [Cell 实体定时器](/zh/architecture/source-analysis/scripting.md#cell-entity-script-timers) |
| 已深入解析 | [moveToPoint](/zh/api/kbengine/cellapp/Entity.md#moveToPoint), [moveToEntity](/zh/api/kbengine/cellapp/Entity.md#moveToEntity), [navigate](/zh/api/kbengine/cellapp/Entity.md#navigate), [navigatePathPoints](/zh/api/kbengine/cellapp/Entity.md#navigatePathPoints), [cancelController](/zh/api/kbengine/cellapp/Entity.md#cancelController) | [Cell 移动控制器链](/zh/architecture/source-analysis/space-aoi.md#cell-entity-move-controllers) |
| 已深入解析 | [fireEvent](/zh/api/kbengine/cellapp/Entity.md#fireEvent), [registerEvent](/zh/api/kbengine/cellapp/Entity.md#registerEvent), [deregisterEvent](/zh/api/kbengine/cellapp/Entity.md#deregisterEvent) | [事件系统](/zh/architecture/source-analysis/events.md) |
| 已深入解析 | [onEnterTrap](/zh/api/kbengine/cellapp/Entity.md#onEnterTrap), [onEnteredView](/zh/api/kbengine/cellapp/Entity.md#onEnteredView), [onLeaveTrap](/zh/api/kbengine/cellapp/Entity.md#onLeaveTrap), [onLoseControlledBy](/zh/api/kbengine/cellapp/Entity.md#onLoseControlledBy), [onMove](/zh/api/kbengine/cellapp/Entity.md#onMove), [onMoveOver](/zh/api/kbengine/cellapp/Entity.md#onMoveOver), [onMoveFailure](/zh/api/kbengine/cellapp/Entity.md#onMoveFailure), [onSpaceGone](/zh/api/kbengine/cellapp/Entity.md#onSpaceGone), [onTurn](/zh/api/kbengine/cellapp/Entity.md#onTurn), [onTeleport](/zh/api/kbengine/cellapp/Entity.md#onTeleport), [onTeleportFailure](/zh/api/kbengine/cellapp/Entity.md#onTeleportFailure), [onTeleportSuccess](/zh/api/kbengine/cellapp/Entity.md#onTeleportSuccess), [onUpdateBegin](/zh/api/kbengine/cellapp/Entity.md#onUpdateBegin), [onUpdateEnd](/zh/api/kbengine/cellapp/Entity.md#onUpdateEnd), [onWitnessed](/zh/api/kbengine/cellapp/Entity.md#onWitnessed) | [Cell Entity 回调触发点矩阵](/zh/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) |
| 已深入解析 | [setViewRadius](/zh/api/kbengine/cellapp/Entity.md#setViewRadius), [getViewRadius](/zh/api/kbengine/cellapp/Entity.md#getViewRadius), [getViewHystArea](/zh/api/kbengine/cellapp/Entity.md#getViewHystArea), [entitiesInView](/zh/api/kbengine/cellapp/Entity.md#entitiesInView), [getWitnesses](/zh/api/kbengine/cellapp/Entity.md#getWitnesses) | [Witness 运行态 API](/zh/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis) |
| 已深入解析 | [canNavigate](/zh/api/kbengine/cellapp/Entity.md#canNavigate), [destroySpace](/zh/api/kbengine/cellapp/Entity.md#destroySpace), [debugView](/zh/api/kbengine/cellapp/Entity.md#debugView), [entitiesInRange](/zh/api/kbengine/cellapp/Entity.md#entitiesInRange), [getRandomPoints](/zh/api/kbengine/cellapp/Entity.md#getRandomPoints) | [空间运行态 API](/zh/architecture/source-analysis/space-aoi.md#cell-entity-space-runtime-apis) |
| 已深入解析 | [accelerate](/zh/api/kbengine/cellapp/Entity.md#accelerate), [getComponent](/zh/api/kbengine/cellapp/Entity.md#getComponent) | [控制器调速与组件访问 API](/zh/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis) |
| 已深入解析 | [clientEntity](/zh/api/kbengine/cellapp/Entity.md#clientEntity), [isReal](/zh/api/kbengine/cellapp/Entity.md#isReal), [onGetWitness](/zh/api/kbengine/cellapp/Entity.md#onGetWitness), [onLoseWitness](/zh/api/kbengine/cellapp/Entity.md#onLoseWitness) | [Witness 运行态 API](/zh/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis) |
| 已深入解析 | [destroy](/zh/api/kbengine/cellapp/Entity.md#destroy), [onDestroy](/zh/api/kbengine/cellapp/Entity.md#onDestroy) | [Cell 销毁与摘链](/zh/architecture/source-analysis/space-aoi.md#cell-entity-destroy-lifecycle) |
| 已深入解析 | [onRestore](/zh/api/kbengine/cellapp/Entity.md#onRestore), [onWriteToDB](/zh/api/kbengine/cellapp/Entity.md#onWriteToDB), [writeToDB](/zh/api/kbengine/cellapp/Entity.md#writeToDB) | [持久化与数据库](/zh/architecture/source-analysis/persistence.md) |
| 已深入解析 | [addYawRotator](/zh/api/kbengine/cellapp/Entity.md#addYawRotator), [addProximity](/zh/api/kbengine/cellapp/Entity.md#addProximity), [teleport](/zh/api/kbengine/cellapp/Entity.md#teleport) | [控制器调速与组件访问 API](/zh/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis) 与 [Cell Entity 回调触发点矩阵](/zh/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) |

## cellapp/KBEngine

API 页面：[docs/api/kbengine/cellapp/KBEngine.md](/zh/api/kbengine/cellapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [cellAppData](/zh/api/kbengine/cellapp/KBEngine.md#cellAppData), [globalData](/zh/api/kbengine/cellapp/KBEngine.md#globalData), [onCellAppData](/zh/api/kbengine/cellapp/KBEngine.md#onCellAppData), [onGlobalData](/zh/api/kbengine/cellapp/KBEngine.md#onGlobalData) | [全局数据同步链](/zh/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [MemoryStream](/zh/api/kbengine/cellapp/KBEngine.md#MemoryStream), [createEntity](/zh/api/kbengine/cellapp/KBEngine.md#createEntity), [publish](/zh/api/kbengine/cellapp/KBEngine.md#publish), [reloadScript](/zh/api/kbengine/cellapp/KBEngine.md#reloadScript), [scriptLogType](/zh/api/kbengine/cellapp/KBEngine.md#scriptLogType), [time](/zh/api/kbengine/cellapp/KBEngine.md#time), [onInit](/zh/api/kbengine/cellapp/KBEngine.md#onInit), [onSpaceData](/zh/api/kbengine/cellapp/KBEngine.md#onSpaceData), [onSpaceGeometryLoaded](/zh/api/kbengine/cellapp/KBEngine.md#onSpaceGeometryLoaded), [onAllSpaceGeometryLoaded](/zh/api/kbengine/cellapp/KBEngine.md#onAllSpaceGeometryLoaded) | [CellApp 空间运行时 API](/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [getResFullPath](/zh/api/kbengine/cellapp/KBEngine.md#getResFullPath), [hasRes](/zh/api/kbengine/cellapp/KBEngine.md#hasRes), [listPathRes](/zh/api/kbengine/cellapp/KBEngine.md#listPathRes), [matchPath](/zh/api/kbengine/cellapp/KBEngine.md#matchPath), [open](/zh/api/kbengine/cellapp/KBEngine.md#open) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [addSpaceGeometryMapping](/zh/api/kbengine/cellapp/KBEngine.md#addSpaceGeometryMapping), [delSpaceData](/zh/api/kbengine/cellapp/KBEngine.md#delSpaceData), [getSpaceData](/zh/api/kbengine/cellapp/KBEngine.md#getSpaceData), [getSpaceGeometryMapping](/zh/api/kbengine/cellapp/KBEngine.md#getSpaceGeometryMapping), [setSpaceData](/zh/api/kbengine/cellapp/KBEngine.md#setSpaceData) | [CellApp 空间运行时 API](/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [addWatcher](/zh/api/kbengine/cellapp/KBEngine.md#addWatcher), [delWatcher](/zh/api/kbengine/cellapp/KBEngine.md#delWatcher), [getWatcher](/zh/api/kbengine/cellapp/KBEngine.md#getWatcher), [getWatcherDir](/zh/api/kbengine/cellapp/KBEngine.md#getWatcherDir) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [address](/zh/api/kbengine/cellapp/KBEngine.md#address), [debugTracing](/zh/api/kbengine/cellapp/KBEngine.md#debugTracing), [getAppFlags](/zh/api/kbengine/cellapp/KBEngine.md#getAppFlags), [setAppFlags](/zh/api/kbengine/cellapp/KBEngine.md#setAppFlags), [isShuttingDown](/zh/api/kbengine/cellapp/KBEngine.md#isShuttingDown), [genUUID64](/zh/api/kbengine/cellapp/KBEngine.md#genUUID64) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [executeRawDatabaseCommand](/zh/api/kbengine/cellapp/KBEngine.md#executeRawDatabaseCommand), [raycast](/zh/api/kbengine/cellapp/KBEngine.md#raycast) | [CellApp 空间运行时 API](/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [urlopen](/zh/api/kbengine/cellapp/KBEngine.md#urlopen) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [registerReadFileDescriptor](/zh/api/kbengine/cellapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/zh/api/kbengine/cellapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/zh/api/kbengine/cellapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/zh/api/kbengine/cellapp/KBEngine.md#deregisterWriteFileDescriptor) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [onCellAppDataDel](/zh/api/kbengine/cellapp/KBEngine.md#onCellAppDataDel), [onGlobalDataDel](/zh/api/kbengine/cellapp/KBEngine.md#onGlobalDataDel), [onReadyForLogin](/zh/api/kbengine/cellapp/KBEngine.md#onReadyForLogin) | [CellApp 空间运行时 API](/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |

## client/Entity

API 页面：[docs/api/kbengine/client/Entity.md](/zh/api/kbengine/client/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseCall](/zh/api/kbengine/client/Entity.md#baseCall), [cellCall](/zh/api/kbengine/client/Entity.md#cellCall) | [客户端远程调用链](/zh/architecture/source-analysis/networking.md#client-entity-remote-calls) |
| 已深入解析 | [isPlayer](/zh/api/kbengine/client/Entity.md#isPlayer), [onEnterWorld](/zh/api/kbengine/client/Entity.md#onEnterWorld), [onLeaveWorld](/zh/api/kbengine/client/Entity.md#onLeaveWorld), [onEnterSpace](/zh/api/kbengine/client/Entity.md#onEnterSpace), [onLeaveSpace](/zh/api/kbengine/client/Entity.md#onLeaveSpace) | [客户端实体生命周期](/zh/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) |
| 已深入解析 | [fireEvent](/zh/api/kbengine/client/Entity.md#fireEvent), [registerEvent](/zh/api/kbengine/client/Entity.md#registerEvent), [deregisterEvent](/zh/api/kbengine/client/Entity.md#deregisterEvent) | [事件系统](/zh/architecture/source-analysis/events.md) |
| 已深入解析 | [onDestroy](/zh/api/kbengine/client/Entity.md#onDestroy) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) |
| 已深入解析 | [getComponent](/zh/api/kbengine/client/Entity.md#getComponent) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) |

## client/KBEngine

API 页面：[docs/api/kbengine/client/KBEngine.md](/zh/api/kbengine/client/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/zh/api/kbengine/client/KBEngine.md#player), [findEntity](/zh/api/kbengine/client/KBEngine.md#findEntity) | [客户端句柄表](/zh/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 已深入解析 | [login](/zh/api/kbengine/client/KBEngine.md#login), [reloginBaseapp](/zh/api/kbengine/client/KBEngine.md#reloginBaseapp) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) |
| 已深入解析 | [createAccount](/zh/api/kbengine/client/KBEngine.md#createAccount), [getSpaceData](/zh/api/kbengine/client/KBEngine.md#getSpaceData) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) |
| 部分覆盖 | [resetPassword](/zh/api/kbengine/client/KBEngine.md#resetPassword), [bindAccountEmail](/zh/api/kbengine/client/KBEngine.md#bindAccountEmail), [newPassword](/zh/api/kbengine/client/KBEngine.md#newPassword) | [客户端登录、重登录与 SpaceData API](/zh/architecture/source-analysis/client-login-and-space-data-api.md) 已解释服务端链、客户端回包与 SDK 模板对照，但当前核心 `client_lib` 里仍未看到直接 Python 模块注册点。 |

## dbmgr/KBEngine

API 页面：[docs/api/kbengine/dbmgr/KBEngine.md](/zh/api/kbengine/dbmgr/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [executeRawDatabaseCommand](/zh/api/kbengine/dbmgr/KBEngine.md#executeRawDatabaseCommand), [onDBMgrReady](/zh/api/kbengine/dbmgr/KBEngine.md#onDBMgrReady), [onDBMgrShutDown](/zh/api/kbengine/dbmgr/KBEngine.md#onDBMgrShutDown), [onReadyForShutDown](/zh/api/kbengine/dbmgr/KBEngine.md#onReadyForShutDown), [onSelectAccountDBInterface](/zh/api/kbengine/dbmgr/KBEngine.md#onSelectAccountDBInterface) | [组件型脚本 API](/zh/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/zh/api/kbengine/dbmgr/KBEngine.md#addTimer), [delTimer](/zh/api/kbengine/dbmgr/KBEngine.md#delTimer), [urlopen](/zh/api/kbengine/dbmgr/KBEngine.md#urlopen) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |

## interfaces/KBEngine

API 页面：[docs/api/kbengine/interfaces/KBEngine.md](/zh/api/kbengine/interfaces/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [accountLoginResponse](/zh/api/kbengine/interfaces/KBEngine.md#accountLoginResponse), [createAccountResponse](/zh/api/kbengine/interfaces/KBEngine.md#createAccountResponse), [chargeResponse](/zh/api/kbengine/interfaces/KBEngine.md#chargeResponse), [executeRawDatabaseCommand](/zh/api/kbengine/interfaces/KBEngine.md#executeRawDatabaseCommand), [onInterfaceAppReady](/zh/api/kbengine/interfaces/KBEngine.md#onInterfaceAppReady), [onInterfaceAppShutDown](/zh/api/kbengine/interfaces/KBEngine.md#onInterfaceAppShutDown), [onRequestCreateAccount](/zh/api/kbengine/interfaces/KBEngine.md#onRequestCreateAccount), [onRequestAccountLogin](/zh/api/kbengine/interfaces/KBEngine.md#onRequestAccountLogin), [onRequestCharge](/zh/api/kbengine/interfaces/KBEngine.md#onRequestCharge) | [组件型脚本 API](/zh/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/zh/api/kbengine/interfaces/KBEngine.md#addTimer), [delTimer](/zh/api/kbengine/interfaces/KBEngine.md#delTimer), [urlopen](/zh/api/kbengine/interfaces/KBEngine.md#urlopen) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |

## logger/KBEngine

API 页面：[docs/api/kbengine/logger/KBEngine.md](/zh/api/kbengine/logger/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoggerAppReady](/zh/api/kbengine/logger/KBEngine.md#onLoggerAppReady), [onLoggerAppShutDown](/zh/api/kbengine/logger/KBEngine.md#onLoggerAppShutDown), [onLogWrote](/zh/api/kbengine/logger/KBEngine.md#onLogWrote), [onReadyForShutDown](/zh/api/kbengine/logger/KBEngine.md#onReadyForShutDown) | [组件型脚本 API](/zh/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/zh/api/kbengine/logger/KBEngine.md#addTimer), [delTimer](/zh/api/kbengine/logger/KBEngine.md#delTimer), [urlopen](/zh/api/kbengine/logger/KBEngine.md#urlopen) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |

## loginapp/KBEngine

API 页面：[docs/api/kbengine/loginapp/KBEngine.md](/zh/api/kbengine/loginapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoginAppReady](/zh/api/kbengine/loginapp/KBEngine.md#onLoginAppReady), [onLoginAppShutDown](/zh/api/kbengine/loginapp/KBEngine.md#onLoginAppShutDown), [onRequestLogin](/zh/api/kbengine/loginapp/KBEngine.md#onRequestLogin), [onLoginCallbackFromDB](/zh/api/kbengine/loginapp/KBEngine.md#onLoginCallbackFromDB), [onRequestCreateAccount](/zh/api/kbengine/loginapp/KBEngine.md#onRequestCreateAccount), [onCreateAccountCallbackFromDB](/zh/api/kbengine/loginapp/KBEngine.md#onCreateAccountCallbackFromDB) | [组件型脚本 API](/zh/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/zh/api/kbengine/loginapp/KBEngine.md#addTimer), [delTimer](/zh/api/kbengine/loginapp/KBEngine.md#delTimer), [urlopen](/zh/api/kbengine/loginapp/KBEngine.md#urlopen) | [通用运行时工具 API](/zh/architecture/source-analysis/runtime-utility-api.md) |

## 关键词与基础类型

API 页面：[docs/api/kbengine/keywords.md](/zh/api/kbengine/keywords.md)、[docs/api/kbengine/basetypes.md](/zh/api/kbengine/basetypes.md)

| 状态 | 项目 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | `EntityCall`, `entity`, `base`, `cell`, `client` | [实体系统](/zh/architecture/source-analysis/entity-system.md)、[网络与消息系统](/zh/architecture/source-analysis/networking.md) |
| 已深入解析 | `View`, `Witness`, `Space`, `real`, `ghost`, `cellapp`, `baseapp` | [空间与 AOI](/zh/architecture/source-analysis/space-aoi.md)、[进程模型](/zh/architecture/source-analysis/process-model.md) |
| 已深入解析 | `entities.xml`, 基础数据类型, `ENTITYCALL`, `.def` 类型系统 | [类型系统与实体定义文件](/zh/architecture/source-analysis/entitydef-type-system.md) |
| 已深入解析 | `KBE_ROOT`, `KBE_RES_PATH`, `kbengine_defaults.xml`, `kbengine.xml`, `vector3` | [运行时配置与基础类型](/zh/architecture/source-analysis/runtime-config-and-types.md) |
| 已深入解析 | `KBE_HYBRID_PATH` | [运行时配置与基础类型](/zh/architecture/source-analysis/runtime-config-and-types.md) 已说明：当前源码树未命中该名字，实际贯穿实现的是 `KBE_BIN_PATH`。 |

## 后续维护规则

- 新增源码解析专题时，应同步更新本页，把对应接口从“未覆盖”移动到“部分覆盖”或“已深入解析”。
- 若发现 KBEngine API 页与 CHM 不一致，只修 `docs/api/kbengine/**` 的错误；不要在 API 页中扩写源码解析。
- 如果某个 API 只适合保留 CHM 原文、不值得展开源码专题，也应在本页标注原因，避免重复排查。
