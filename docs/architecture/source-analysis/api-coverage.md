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
| P2 | `client/KBEngine` 的 `resetPassword / bindAccountEmail / newPassword` 在核心 `client_lib` 的直注册入口边界 | `client-login-and-space-data-api.md` 补充段 |

## baseapp/Entity

API 页面：[docs/api/baseapp/Entity.md](/api/baseapp/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [createCellEntity](/api/baseapp/Entity.md#createCellEntity), [onGetCell](/api/baseapp/Entity.md#onGetCell) | [实体系统：Base -> Cell 交接](/architecture/source-analysis/entity-system.md#base-entity-create-cell) |
| 已深入解析 | [fireEvent](/api/baseapp/Entity.md#fireEvent), [registerEvent](/api/baseapp/Entity.md#registerEvent), [deregisterEvent](/api/baseapp/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [addTimer](/api/baseapp/Entity.md#addTimer), [delTimer](/api/baseapp/Entity.md#delTimer), [onTimer](/api/baseapp/Entity.md#onTimer) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [destroy](/api/baseapp/Entity.md#destroy), [onDestroy](/api/baseapp/Entity.md#onDestroy) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [writeToDB](/api/baseapp/Entity.md#writeToDB), [onWriteToDB](/api/baseapp/Entity.md#onWriteToDB), [onRestore](/api/baseapp/Entity.md#onRestore), [onPreArchive](/api/baseapp/Entity.md#onPreArchive) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) 与 [持久化与数据库](/architecture/source-analysis/persistence.md) |
| 已深入解析 | [createCellEntityInNewSpace](/api/baseapp/Entity.md#createCellEntityInNewSpace), [destroyCellEntity](/api/baseapp/Entity.md#destroyCellEntity), [onCreateCellFailure](/api/baseapp/Entity.md#onCreateCellFailure), [onLoseCell](/api/baseapp/Entity.md#onLoseCell) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |
| 已深入解析 | [getComponent](/api/baseapp/Entity.md#getComponent) | [Base 实体生命周期](/architecture/source-analysis/base-entity-lifecycle.md) |

## baseapp/KBEngine

API 页面：[docs/api/baseapp/KBEngine.md](/api/baseapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseAppData](/api/baseapp/KBEngine.md#baseAppData), [globalData](/api/baseapp/KBEngine.md#globalData), [onBaseAppData](/api/baseapp/KBEngine.md#onBaseAppData), [onGlobalData](/api/baseapp/KBEngine.md#onGlobalData) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [createEntity](/api/baseapp/KBEngine.md#createEntity), [createEntityLocally](/api/baseapp/KBEngine.md#createEntityLocally), [createEntityAnywhere](/api/baseapp/KBEngine.md#createEntityAnywhere), [createEntityRemotely](/api/baseapp/KBEngine.md#createEntityRemotely), [createEntityFromDBID](/api/baseapp/KBEngine.md#createEntityFromDBID), [createEntityAnywhereFromDBID](/api/baseapp/KBEngine.md#createEntityAnywhereFromDBID), [createEntityRemotelyFromDBID](/api/baseapp/KBEngine.md#createEntityRemotelyFromDBID), [deleteEntityByDBID](/api/baseapp/KBEngine.md#deleteEntityByDBID), [lookUpEntityByDBID](/api/baseapp/KBEngine.md#lookUpEntityByDBID) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [MemoryStream](/api/baseapp/KBEngine.md#MemoryStream) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [getResFullPath](/api/baseapp/KBEngine.md#getResFullPath), [hasRes](/api/baseapp/KBEngine.md#hasRes), [listPathRes](/api/baseapp/KBEngine.md#listPathRes), [matchPath](/api/baseapp/KBEngine.md#matchPath), [open](/api/baseapp/KBEngine.md#open) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [publish](/api/baseapp/KBEngine.md#publish), [reloadScript](/api/baseapp/KBEngine.md#reloadScript), [scriptLogType](/api/baseapp/KBEngine.md#scriptLogType), [time](/api/baseapp/KBEngine.md#time), [onInit](/api/baseapp/KBEngine.md#onInit), [onBaseAppReady](/api/baseapp/KBEngine.md#onBaseAppReady), [onReadyForLogin](/api/baseapp/KBEngine.md#onReadyForLogin), [onReadyForShutDown](/api/baseapp/KBEngine.md#onReadyForShutDown), [onBaseAppShutDown](/api/baseapp/KBEngine.md#onBaseAppShutDown), [onCellAppDeath](/api/baseapp/KBEngine.md#onCellAppDeath), [onAutoLoadEntityCreate](/api/baseapp/KBEngine.md#onAutoLoadEntityCreate) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [addWatcher](/api/baseapp/KBEngine.md#addWatcher), [delWatcher](/api/baseapp/KBEngine.md#delWatcher), [getWatcher](/api/baseapp/KBEngine.md#getWatcher), [getWatcherDir](/api/baseapp/KBEngine.md#getWatcherDir) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [address](/api/baseapp/KBEngine.md#address), [debugTracing](/api/baseapp/KBEngine.md#debugTracing), [getAppFlags](/api/baseapp/KBEngine.md#getAppFlags), [setAppFlags](/api/baseapp/KBEngine.md#setAppFlags), [isShuttingDown](/api/baseapp/KBEngine.md#isShuttingDown), [genUUID64](/api/baseapp/KBEngine.md#genUUID64) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [quantumPassedPercent](/api/baseapp/KBEngine.md#quantumPassedPercent) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [charge](/api/baseapp/KBEngine.md#charge), [onLoseChargeCB](/api/baseapp/KBEngine.md#onLoseChargeCB) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [executeRawDatabaseCommand](/api/baseapp/KBEngine.md#executeRawDatabaseCommand) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) |
| 已深入解析 | [registerReadFileDescriptor](/api/baseapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/api/baseapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/api/baseapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/api/baseapp/KBEngine.md#deregisterWriteFileDescriptor) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [urlopen](/api/baseapp/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [onBaseAppDataDel](/api/baseapp/KBEngine.md#onBaseAppDataDel), [onGlobalDataDel](/api/baseapp/KBEngine.md#onGlobalDataDel) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [onFini](/api/baseapp/KBEngine.md#onFini) | [BaseApp 运行时 API](/architecture/source-analysis/baseapp-kbengine-runtime-api.md) 已说明：当前源码树未查到明确触发点，实际可追关闭链是 `onReadyForShutDown` 与 `onBaseAppShutDown(state)`。 |

## baseapp/Proxy

API 页面：[docs/api/baseapp/Proxy.md](/api/baseapp/Proxy.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [giveClientTo](/api/baseapp/Proxy.md#giveClientTo), [onClientEnabled](/api/baseapp/Proxy.md#onClientEnabled) | [Proxy.giveClientTo 客户端迁移链](/architecture/source-analysis/space-aoi.md#proxy-give-client-to) |
| 已深入解析 | [getClientType](/api/baseapp/Proxy.md#getClientType), [disconnect](/api/baseapp/Proxy.md#disconnect), [getClientDatas](/api/baseapp/Proxy.md#getClientDatas), [streamFileToClient](/api/baseapp/Proxy.md#streamFileToClient), [streamStringToClient](/api/baseapp/Proxy.md#streamStringToClient), [onClientDeath](/api/baseapp/Proxy.md#onClientDeath), [onClientGetCell](/api/baseapp/Proxy.md#onClientGetCell), [onGiveClientToFailure](/api/baseapp/Proxy.md#onGiveClientToFailure), [onLogOnAttempt](/api/baseapp/Proxy.md#onLogOnAttempt), [onStreamComplete](/api/baseapp/Proxy.md#onStreamComplete) | [Proxy 会话与流式传输 API](/architecture/source-analysis/proxy-client-session-api.md) |

## bots/Entity

API 页面：[docs/api/bots/Entity.md](/api/bots/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [moveToPoint](/api/bots/Entity.md#moveToPoint), [cancelController](/api/bots/Entity.md#cancelController), [isPlayer](/api/bots/Entity.md#isPlayer), [onEnterWorld](/api/bots/Entity.md#onEnterWorld), [onLeaveWorld](/api/bots/Entity.md#onLeaveWorld), [onEnterSpace](/api/bots/Entity.md#onEnterSpace), [onLeaveSpace](/api/bots/Entity.md#onLeaveSpace) | [客户端实体生命周期](/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) 与 [客户端本地移动控制器](/architecture/source-analysis/networking.md#client-entity-move-ground-sync) |
| 已深入解析 | [fireEvent](/api/bots/Entity.md#fireEvent), [registerEvent](/api/bots/Entity.md#registerEvent), [deregisterEvent](/api/bots/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [getComponent](/api/bots/Entity.md#getComponent) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |

## bots/KBEngine

API 页面：[docs/api/bots/KBEngine.md](/api/bots/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addBots](/api/bots/KBEngine.md#addBots), [scriptLogType](/api/bots/KBEngine.md#scriptLogType), [onInit](/api/bots/KBEngine.md#onInit), [onFinish](/api/bots/KBEngine.md#onFinish) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [callback](/api/bots/KBEngine.md#callback), [cancelCallback](/api/bots/KBEngine.md#cancelCallback), [getWatcher](/api/bots/KBEngine.md#getWatcher), [getWatcherDir](/api/bots/KBEngine.md#getWatcherDir), [urlopen](/api/bots/KBEngine.md#urlopen), [genUUID64](/api/bots/KBEngine.md#genUUID64) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## bots/PyClientApp

API 页面：[docs/api/bots/PyClientApp.md](/api/bots/PyClientApp.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/api/bots/PyClientApp.md#player) | [客户端句柄表](/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 已深入解析 | [getSpaceData](/api/bots/PyClientApp.md#getSpaceData) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |

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
| 已深入解析 | [clientEntity](/api/cellapp/Entity.md#clientEntity), [isReal](/api/cellapp/Entity.md#isReal), [onGetWitness](/api/cellapp/Entity.md#onGetWitness), [onLoseWitness](/api/cellapp/Entity.md#onLoseWitness) | [Witness 运行态 API](/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis) |
| 已深入解析 | [destroy](/api/cellapp/Entity.md#destroy), [onDestroy](/api/cellapp/Entity.md#onDestroy) | [Cell 销毁与摘链](/architecture/source-analysis/space-aoi.md#cell-entity-destroy-lifecycle) |
| 已深入解析 | [onRestore](/api/cellapp/Entity.md#onRestore), [onWriteToDB](/api/cellapp/Entity.md#onWriteToDB), [writeToDB](/api/cellapp/Entity.md#writeToDB) | [持久化与数据库](/architecture/source-analysis/persistence.md) |
| 已深入解析 | [addYawRotator](/api/cellapp/Entity.md#addYawRotator), [addProximity](/api/cellapp/Entity.md#addProximity), [teleport](/api/cellapp/Entity.md#teleport) | [控制器调速与组件访问 API](/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis) 与 [Cell Entity 回调触发点矩阵](/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix) |

## cellapp/KBEngine

API 页面：[docs/api/cellapp/KBEngine.md](/api/cellapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [cellAppData](/api/cellapp/KBEngine.md#cellAppData), [globalData](/api/cellapp/KBEngine.md#globalData), [onCellAppData](/api/cellapp/KBEngine.md#onCellAppData), [onGlobalData](/api/cellapp/KBEngine.md#onGlobalData) | [全局数据同步链](/architecture/source-analysis/networking.md#global-data-dicts-sync) |
| 已深入解析 | [MemoryStream](/api/cellapp/KBEngine.md#MemoryStream), [createEntity](/api/cellapp/KBEngine.md#createEntity), [publish](/api/cellapp/KBEngine.md#publish), [reloadScript](/api/cellapp/KBEngine.md#reloadScript), [scriptLogType](/api/cellapp/KBEngine.md#scriptLogType), [time](/api/cellapp/KBEngine.md#time), [onInit](/api/cellapp/KBEngine.md#onInit), [onSpaceData](/api/cellapp/KBEngine.md#onSpaceData), [onSpaceGeometryLoaded](/api/cellapp/KBEngine.md#onSpaceGeometryLoaded), [onAllSpaceGeometryLoaded](/api/cellapp/KBEngine.md#onAllSpaceGeometryLoaded) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [getResFullPath](/api/cellapp/KBEngine.md#getResFullPath), [hasRes](/api/cellapp/KBEngine.md#hasRes), [listPathRes](/api/cellapp/KBEngine.md#listPathRes), [matchPath](/api/cellapp/KBEngine.md#matchPath), [open](/api/cellapp/KBEngine.md#open) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [addSpaceGeometryMapping](/api/cellapp/KBEngine.md#addSpaceGeometryMapping), [delSpaceData](/api/cellapp/KBEngine.md#delSpaceData), [getSpaceData](/api/cellapp/KBEngine.md#getSpaceData), [getSpaceGeometryMapping](/api/cellapp/KBEngine.md#getSpaceGeometryMapping), [setSpaceData](/api/cellapp/KBEngine.md#setSpaceData) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [addWatcher](/api/cellapp/KBEngine.md#addWatcher), [delWatcher](/api/cellapp/KBEngine.md#delWatcher), [getWatcher](/api/cellapp/KBEngine.md#getWatcher), [getWatcherDir](/api/cellapp/KBEngine.md#getWatcherDir) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [address](/api/cellapp/KBEngine.md#address), [debugTracing](/api/cellapp/KBEngine.md#debugTracing), [getAppFlags](/api/cellapp/KBEngine.md#getAppFlags), [setAppFlags](/api/cellapp/KBEngine.md#setAppFlags), [isShuttingDown](/api/cellapp/KBEngine.md#isShuttingDown), [genUUID64](/api/cellapp/KBEngine.md#genUUID64) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [executeRawDatabaseCommand](/api/cellapp/KBEngine.md#executeRawDatabaseCommand), [raycast](/api/cellapp/KBEngine.md#raycast) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |
| 已深入解析 | [urlopen](/api/cellapp/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [registerReadFileDescriptor](/api/cellapp/KBEngine.md#registerReadFileDescriptor), [registerWriteFileDescriptor](/api/cellapp/KBEngine.md#registerWriteFileDescriptor), [deregisterReadFileDescriptor](/api/cellapp/KBEngine.md#deregisterReadFileDescriptor), [deregisterWriteFileDescriptor](/api/cellapp/KBEngine.md#deregisterWriteFileDescriptor) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |
| 已深入解析 | [onCellAppDataDel](/api/cellapp/KBEngine.md#onCellAppDataDel), [onGlobalDataDel](/api/cellapp/KBEngine.md#onGlobalDataDel), [onReadyForLogin](/api/cellapp/KBEngine.md#onReadyForLogin) | [CellApp 空间运行时 API](/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md) |

## client/Entity

API 页面：[docs/api/client/Entity.md](/api/client/Entity.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseCall](/api/client/Entity.md#baseCall), [cellCall](/api/client/Entity.md#cellCall) | [客户端远程调用链](/architecture/source-analysis/networking.md#client-entity-remote-calls) |
| 已深入解析 | [isPlayer](/api/client/Entity.md#isPlayer), [onEnterWorld](/api/client/Entity.md#onEnterWorld), [onLeaveWorld](/api/client/Entity.md#onLeaveWorld), [onEnterSpace](/api/client/Entity.md#onEnterSpace), [onLeaveSpace](/api/client/Entity.md#onLeaveSpace) | [客户端实体生命周期](/architecture/source-analysis/networking.md#client-entity-world-space-callbacks) |
| 已深入解析 | [fireEvent](/api/client/Entity.md#fireEvent), [registerEvent](/api/client/Entity.md#registerEvent), [deregisterEvent](/api/client/Entity.md#deregisterEvent) | [事件系统](/architecture/source-analysis/events.md) |
| 已深入解析 | [onDestroy](/api/client/Entity.md#onDestroy) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |
| 已深入解析 | [getComponent](/api/client/Entity.md#getComponent) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |

## client/KBEngine

API 页面：[docs/api/client/KBEngine.md](/api/client/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/api/client/KBEngine.md#player), [findEntity](/api/client/KBEngine.md#findEntity) | [客户端句柄表](/architecture/source-analysis/networking.md#client-entity-handles-table) |
| 已深入解析 | [login](/api/client/KBEngine.md#login), [reloginBaseapp](/api/client/KBEngine.md#reloginBaseapp) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |
| 已深入解析 | [createAccount](/api/client/KBEngine.md#createAccount), [getSpaceData](/api/client/KBEngine.md#getSpaceData) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) |
| 部分覆盖 | [resetPassword](/api/client/KBEngine.md#resetPassword), [bindAccountEmail](/api/client/KBEngine.md#bindAccountEmail), [newPassword](/api/client/KBEngine.md#newPassword) | [客户端登录、重登录与 SpaceData API](/architecture/source-analysis/client-login-and-space-data-api.md) 已解释服务端链、客户端回包与 SDK 模板对照，但当前核心 `client_lib` 里仍未看到直接 Python 模块注册点。 |

## dbmgr/KBEngine

API 页面：[docs/api/dbmgr/KBEngine.md](/api/dbmgr/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [executeRawDatabaseCommand](/api/dbmgr/KBEngine.md#executeRawDatabaseCommand), [onDBMgrReady](/api/dbmgr/KBEngine.md#onDBMgrReady), [onDBMgrShutDown](/api/dbmgr/KBEngine.md#onDBMgrShutDown), [onReadyForShutDown](/api/dbmgr/KBEngine.md#onReadyForShutDown), [onSelectAccountDBInterface](/api/dbmgr/KBEngine.md#onSelectAccountDBInterface) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/dbmgr/KBEngine.md#addTimer), [delTimer](/api/dbmgr/KBEngine.md#delTimer), [urlopen](/api/dbmgr/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## interfaces/KBEngine

API 页面：[docs/api/interfaces/KBEngine.md](/api/interfaces/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [accountLoginResponse](/api/interfaces/KBEngine.md#accountLoginResponse), [createAccountResponse](/api/interfaces/KBEngine.md#createAccountResponse), [chargeResponse](/api/interfaces/KBEngine.md#chargeResponse), [executeRawDatabaseCommand](/api/interfaces/KBEngine.md#executeRawDatabaseCommand), [onInterfaceAppReady](/api/interfaces/KBEngine.md#onInterfaceAppReady), [onInterfaceAppShutDown](/api/interfaces/KBEngine.md#onInterfaceAppShutDown), [onRequestCreateAccount](/api/interfaces/KBEngine.md#onRequestCreateAccount), [onRequestAccountLogin](/api/interfaces/KBEngine.md#onRequestAccountLogin), [onRequestCharge](/api/interfaces/KBEngine.md#onRequestCharge) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/interfaces/KBEngine.md#addTimer), [delTimer](/api/interfaces/KBEngine.md#delTimer), [urlopen](/api/interfaces/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## logger/KBEngine

API 页面：[docs/api/logger/KBEngine.md](/api/logger/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoggerAppReady](/api/logger/KBEngine.md#onLoggerAppReady), [onLoggerAppShutDown](/api/logger/KBEngine.md#onLoggerAppShutDown), [onLogWrote](/api/logger/KBEngine.md#onLogWrote), [onReadyForShutDown](/api/logger/KBEngine.md#onReadyForShutDown) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/logger/KBEngine.md#addTimer), [delTimer](/api/logger/KBEngine.md#delTimer), [urlopen](/api/logger/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## loginapp/KBEngine

API 页面：[docs/api/loginapp/KBEngine.md](/api/loginapp/KBEngine.md)

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoginAppReady](/api/loginapp/KBEngine.md#onLoginAppReady), [onLoginAppShutDown](/api/loginapp/KBEngine.md#onLoginAppShutDown), [onRequestLogin](/api/loginapp/KBEngine.md#onRequestLogin), [onLoginCallbackFromDB](/api/loginapp/KBEngine.md#onLoginCallbackFromDB), [onRequestCreateAccount](/api/loginapp/KBEngine.md#onRequestCreateAccount), [onCreateAccountCallbackFromDB](/api/loginapp/KBEngine.md#onCreateAccountCallbackFromDB) | [组件型脚本 API](/architecture/source-analysis/component-script-api.md) |
| 已深入解析 | [addTimer](/api/loginapp/KBEngine.md#addTimer), [delTimer](/api/loginapp/KBEngine.md#delTimer), [urlopen](/api/loginapp/KBEngine.md#urlopen) | [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md) |

## 关键词与基础类型

API 页面：[docs/api/keywords.md](/api/keywords.md)、[docs/api/basetypes.md](/api/basetypes.md)

| 状态 | 项目 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | `EntityCall`, `entity`, `base`, `cell`, `client` | [实体系统](/architecture/source-analysis/entity-system.md)、[网络与消息系统](/architecture/source-analysis/networking.md) |
| 已深入解析 | `View`, `Witness`, `Space`, `real`, `ghost`, `cellapp`, `baseapp` | [空间与 AOI](/architecture/source-analysis/space-aoi.md)、[进程模型](/architecture/source-analysis/process-model.md) |
| 已深入解析 | `entities.xml`, 基础数据类型, `ENTITYCALL`, `.def` 类型系统 | [类型系统与实体定义文件](/architecture/source-analysis/entitydef-type-system.md) |
| 已深入解析 | `KBE_ROOT`, `KBE_RES_PATH`, `kbengine_defaults.xml`, `kbengine.xml`, `vector3` | [运行时配置与基础类型](/architecture/source-analysis/runtime-config-and-types.md) |
| 已深入解析 | `KBE_HYBRID_PATH` | [运行时配置与基础类型](/architecture/source-analysis/runtime-config-and-types.md) 已说明：当前源码树未命中该名字，实际贯穿实现的是 `KBE_BIN_PATH`。 |

## 后续维护规则

- 新增源码解析专题时，应同步更新本页，把对应接口从“未覆盖”移动到“部分覆盖”或“已深入解析”。
- 若发现 API 页与 CHM 不一致，只修 `docs/api/**` 的错误；不要在 API 页中扩写源码解析。
- 如果某个 API 只适合保留 CHM 原文、不值得展开源码专题，也应在本页标注原因，避免重复排查。
