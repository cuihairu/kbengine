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

API 页面：[docs/api/kbengine/baseapp/Entity.md](/en/zh/api/kbengine/baseapp/Entity.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [createCellEntity](/en/zh/api/kbengine/baseapp/Entity.md#createCellEntity, [onGetCell](/en/zh/api/kbengine/baseapp/Entity.md#onGetCell | [实体系统：Base -> Cell 交接](/en/zh/architecture/source-analysis/entity-system.md#base-entity-create-cell |
| 已深入解析 | [fireEvent](/en/zh/api/kbengine/baseapp/Entity.md#fireEvent, [registerEvent](/en/zh/api/kbengine/baseapp/Entity.md#registerEvent, [deregisterEvent](/en/zh/api/kbengine/baseapp/Entity.md#deregisterEvent | [事件系统](/en/zh/architecture/source-analysis/events.md |
| 已深入解析 | [addTimer](/en/zh/api/kbengine/baseapp/Entity.md#addTimer, [delTimer](/en/zh/api/kbengine/baseapp/Entity.md#delTimer, [onTimer](/en/zh/api/kbengine/baseapp/Entity.md#onTimer | [Base 实体生命周期](/en/zh/architecture/source-analysis/base-entity-lifecycle.md |
| 已深入解析 | [destroy](/en/zh/api/kbengine/baseapp/Entity.md#destroy, [onDestroy](/en/zh/api/kbengine/baseapp/Entity.md#onDestroy | [Base 实体生命周期](/en/zh/architecture/source-analysis/base-entity-lifecycle.md |
| 已深入解析 | [writeToDB](/en/zh/api/kbengine/baseapp/Entity.md#writeToDB, [onWriteToDB](/en/zh/api/kbengine/baseapp/Entity.md#onWriteToDB, [onRestore](/en/zh/api/kbengine/baseapp/Entity.md#onRestore, [onPreArchive](/en/zh/api/kbengine/baseapp/Entity.md#onPreArchive | [Base 实体生命周期](/en/zh/architecture/source-analysis/base-entity-lifecycle.md 与 [持久化与数据库](/en/zh/architecture/source-analysis/persistence.md |
| 已深入解析 | [createCellEntityInNewSpace](/en/zh/api/kbengine/baseapp/Entity.md#createCellEntityInNewSpace, [destroyCellEntity](/en/zh/api/kbengine/baseapp/Entity.md#destroyCellEntity, [onCreateCellFailure](/en/zh/api/kbengine/baseapp/Entity.md#onCreateCellFailure, [onLoseCell](/en/zh/api/kbengine/baseapp/Entity.md#onLoseCell | [Base 实体生命周期](/en/zh/architecture/source-analysis/base-entity-lifecycle.md |
| 已深入解析 | [getComponent](/en/zh/api/kbengine/baseapp/Entity.md#getComponent | [Base 实体生命周期](/en/zh/architecture/source-analysis/base-entity-lifecycle.md |

## baseapp/KBEngine

API 页面：[docs/api/kbengine/baseapp/KBEngine.md](/en/zh/api/kbengine/baseapp/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseAppData](/en/zh/api/kbengine/baseapp/KBEngine.md#baseAppData, [globalData](/en/zh/api/kbengine/baseapp/KBEngine.md#globalData, [onBaseAppData](/en/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppData, [onGlobalData](/en/zh/api/kbengine/baseapp/KBEngine.md#onGlobalData | [全局数据同步链](/en/zh/architecture/source-analysis/networking.md#global-data-dicts-sync |
| 已深入解析 | [createEntity](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntity, [createEntityLocally](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntityLocally, [createEntityAnywhere](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntityAnywhere, [createEntityRemotely](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntityRemotely, [createEntityFromDBID](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntityFromDBID, [createEntityAnywhereFromDBID](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntityAnywhereFromDBID, [createEntityRemotelyFromDBID](/en/zh/api/kbengine/baseapp/KBEngine.md#createEntityRemotelyFromDBID, [deleteEntityByDBID](/en/zh/api/kbengine/baseapp/KBEngine.md#deleteEntityByDBID, [lookUpEntityByDBID](/en/zh/api/kbengine/baseapp/KBEngine.md#lookUpEntityByDBID | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md |
| 已深入解析 | [MemoryStream](/en/zh/api/kbengine/baseapp/KBEngine.md#MemoryStream | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md |
| 已深入解析 | [getResFullPath](/en/zh/api/kbengine/baseapp/KBEngine.md#getResFullPath, [hasRes](/en/zh/api/kbengine/baseapp/KBEngine.md#hasRes, [listPathRes](/en/zh/api/kbengine/baseapp/KBEngine.md#listPathRes, [matchPath](/en/zh/api/kbengine/baseapp/KBEngine.md#matchPath, [open](/en/zh/api/kbengine/baseapp/KBEngine.md#open | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [publish](/en/zh/api/kbengine/baseapp/KBEngine.md#publish, [reloadScript](/en/zh/api/kbengine/baseapp/KBEngine.md#reloadScript, [scriptLogType](/en/zh/api/kbengine/baseapp/KBEngine.md#scriptLogType, [time](/en/zh/api/kbengine/baseapp/KBEngine.md#time, [onInit](/en/zh/api/kbengine/baseapp/KBEngine.md#onInit, [onBaseAppReady](/en/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppReady, [onReadyForLogin](/en/zh/api/kbengine/baseapp/KBEngine.md#onReadyForLogin, [onReadyForShutDown](/en/zh/api/kbengine/baseapp/KBEngine.md#onReadyForShutDown, [onBaseAppShutDown](/en/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppShutDown, [onCellAppDeath](/en/zh/api/kbengine/baseapp/KBEngine.md#onCellAppDeath, [onAutoLoadEntityCreate](/en/zh/api/kbengine/baseapp/KBEngine.md#onAutoLoadEntityCreate | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md |
| 已深入解析 | [addWatcher](/en/zh/api/kbengine/baseapp/KBEngine.md#addWatcher, [delWatcher](/en/zh/api/kbengine/baseapp/KBEngine.md#delWatcher, [getWatcher](/en/zh/api/kbengine/baseapp/KBEngine.md#getWatcher, [getWatcherDir](/en/zh/api/kbengine/baseapp/KBEngine.md#getWatcherDir | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [address](/en/zh/api/kbengine/baseapp/KBEngine.md#address, [debugTracing](/en/zh/api/kbengine/baseapp/KBEngine.md#debugTracing, [getAppFlags](/en/zh/api/kbengine/baseapp/KBEngine.md#getAppFlags, [setAppFlags](/en/zh/api/kbengine/baseapp/KBEngine.md#setAppFlags, [isShuttingDown](/en/zh/api/kbengine/baseapp/KBEngine.md#isShuttingDown, [genUUID64](/en/zh/api/kbengine/baseapp/KBEngine.md#genUUID64 | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [quantumPassedPercent](/en/zh/api/kbengine/baseapp/KBEngine.md#quantumPassedPercent | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md |
| 已深入解析 | [charge](/en/zh/api/kbengine/baseapp/KBEngine.md#charge, [onLoseChargeCB](/en/zh/api/kbengine/baseapp/KBEngine.md#onLoseChargeCB | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md |
| 已深入解析 | [executeRawDatabaseCommand](/en/zh/api/kbengine/baseapp/KBEngine.md#executeRawDatabaseCommand | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md |
| 已深入解析 | [registerReadFileDescriptor](/en/zh/api/kbengine/baseapp/KBEngine.md#registerReadFileDescriptor, [registerWriteFileDescriptor](/en/zh/api/kbengine/baseapp/KBEngine.md#registerWriteFileDescriptor, [deregisterReadFileDescriptor](/en/zh/api/kbengine/baseapp/KBEngine.md#deregisterReadFileDescriptor, [deregisterWriteFileDescriptor](/en/zh/api/kbengine/baseapp/KBEngine.md#deregisterWriteFileDescriptor | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [urlopen](/en/zh/api/kbengine/baseapp/KBEngine.md#urlopen | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [onBaseAppDataDel](/en/zh/api/kbengine/baseapp/KBEngine.md#onBaseAppDataDel, [onGlobalDataDel](/en/zh/api/kbengine/baseapp/KBEngine.md#onGlobalDataDel | [全局数据同步链](/en/zh/architecture/source-analysis/networking.md#global-data-dicts-sync |
| 已深入解析 | [onFini](/en/zh/api/kbengine/baseapp/KBEngine.md#onFini | [BaseApp 运行时 API](/en/zh/architecture/source-analysis/baseapp-kbengine-runtime-api.md 已说明：当前源码树未查到明确触发点，实际可追关闭链是 `onReadyForShutDown` 与 `onBaseAppShutDown(state)`。 |

## baseapp/Proxy

API 页面：[docs/api/kbengine/baseapp/Proxy.md](/en/zh/api/kbengine/baseapp/Proxy.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [giveClientTo](/en/zh/api/kbengine/baseapp/Proxy.md#giveClientTo, [onClientEnabled](/en/zh/api/kbengine/baseapp/Proxy.md#onClientEnabled | [Proxy.giveClientTo 客户端迁移链](/en/zh/architecture/source-analysis/space-aoi.md#proxy-give-client-to |
| 已深入解析 | [getClientType](/en/zh/api/kbengine/baseapp/Proxy.md#getClientType, [disconnect](/en/zh/api/kbengine/baseapp/Proxy.md#disconnect, [getClientDatas](/en/zh/api/kbengine/baseapp/Proxy.md#getClientDatas, [streamFileToClient](/en/zh/api/kbengine/baseapp/Proxy.md#streamFileToClient, [streamStringToClient](/en/zh/api/kbengine/baseapp/Proxy.md#streamStringToClient, [onClientDeath](/en/zh/api/kbengine/baseapp/Proxy.md#onClientDeath, [onClientGetCell](/en/zh/api/kbengine/baseapp/Proxy.md#onClientGetCell, [onGiveClientToFailure](/en/zh/api/kbengine/baseapp/Proxy.md#onGiveClientToFailure, [onLogOnAttempt](/en/zh/api/kbengine/baseapp/Proxy.md#onLogOnAttempt, [onStreamComplete](/en/zh/api/kbengine/baseapp/Proxy.md#onStreamComplete | [Proxy 会话与流式传输 API](/en/zh/architecture/source-analysis/proxy-client-session-api.md |

## bots/Entity

API 页面：[docs/api/kbengine/bots/Entity.md](/en/zh/api/kbengine/bots/Entity.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [moveToPoint](/en/zh/api/kbengine/bots/Entity.md#moveToPoint, [cancelController](/en/zh/api/kbengine/bots/Entity.md#cancelController, [isPlayer](/en/zh/api/kbengine/bots/Entity.md#isPlayer, [onEnterWorld](/en/zh/api/kbengine/bots/Entity.md#onEnterWorld, [onLeaveWorld](/en/zh/api/kbengine/bots/Entity.md#onLeaveWorld, [onEnterSpace](/en/zh/api/kbengine/bots/Entity.md#onEnterSpace, [onLeaveSpace](/en/zh/api/kbengine/bots/Entity.md#onLeaveSpace | [客户端实体生命周期](/en/zh/architecture/source-analysis/networking.md#client-entity-world-space-callbacks 与 [客户端本地移动控制器](/en/zh/architecture/source-analysis/networking.md#client-entity-move-ground-sync |
| 已深入解析 | [fireEvent](/en/zh/api/kbengine/bots/Entity.md#fireEvent, [registerEvent](/en/zh/api/kbengine/bots/Entity.md#registerEvent, [deregisterEvent](/en/zh/api/kbengine/bots/Entity.md#deregisterEvent | [事件系统](/en/zh/architecture/source-analysis/events.md |
| 已深入解析 | [getComponent](/en/zh/api/kbengine/bots/Entity.md#getComponent | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md |

## bots/KBEngine

API 页面：[docs/api/kbengine/bots/KBEngine.md](/en/zh/api/kbengine/bots/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addBots](/en/zh/api/kbengine/bots/KBEngine.md#addBots, [scriptLogType](/en/zh/api/kbengine/bots/KBEngine.md#scriptLogType, [onInit](/en/zh/api/kbengine/bots/KBEngine.md#onInit, [onFinish](/en/zh/api/kbengine/bots/KBEngine.md#onFinish | [组件型脚本 API](/en/zh/architecture/source-analysis/component-script-api.md |
| 已深入解析 | [callback](/en/zh/api/kbengine/bots/KBEngine.md#callback, [cancelCallback](/en/zh/api/kbengine/bots/KBEngine.md#cancelCallback, [getWatcher](/en/zh/api/kbengine/bots/KBEngine.md#getWatcher, [getWatcherDir](/en/zh/api/kbengine/bots/KBEngine.md#getWatcherDir, [urlopen](/en/zh/api/kbengine/bots/KBEngine.md#urlopen, [genUUID64](/en/zh/api/kbengine/bots/KBEngine.md#genUUID64 | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |

## bots/PyClientApp

API 页面：[docs/api/kbengine/bots/PyClientApp.md](/en/zh/api/kbengine/bots/PyClientApp.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/en/zh/api/kbengine/bots/PyClientApp.md#player | [客户端句柄表](/en/zh/architecture/source-analysis/networking.md#client-entity-handles-table |
| 已深入解析 | [getSpaceData](/en/zh/api/kbengine/bots/PyClientApp.md#getSpaceData | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md |

## cellapp/Entity

API 页面：[docs/api/kbengine/cellapp/Entity.md](/en/zh/api/kbengine/cellapp/Entity.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [addTimer](/en/zh/api/kbengine/cellapp/Entity.md#addTimer, [delTimer](/en/zh/api/kbengine/cellapp/Entity.md#delTimer, [onTimer](/en/zh/api/kbengine/cellapp/Entity.md#onTimer | [Cell 实体定时器](/en/zh/architecture/source-analysis/scripting.md#cell-entity-script-timers |
| 已深入解析 | [moveToPoint](/en/zh/api/kbengine/cellapp/Entity.md#moveToPoint, [moveToEntity](/en/zh/api/kbengine/cellapp/Entity.md#moveToEntity, [navigate](/en/zh/api/kbengine/cellapp/Entity.md#navigate, [navigatePathPoints](/en/zh/api/kbengine/cellapp/Entity.md#navigatePathPoints, [cancelController](/en/zh/api/kbengine/cellapp/Entity.md#cancelController | [Cell 移动控制器链](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-move-controllers |
| 已深入解析 | [fireEvent](/en/zh/api/kbengine/cellapp/Entity.md#fireEvent, [registerEvent](/en/zh/api/kbengine/cellapp/Entity.md#registerEvent, [deregisterEvent](/en/zh/api/kbengine/cellapp/Entity.md#deregisterEvent | [事件系统](/en/zh/architecture/source-analysis/events.md |
| 已深入解析 | [onEnterTrap](/en/zh/api/kbengine/cellapp/Entity.md#onEnterTrap, [onEnteredView](/en/zh/api/kbengine/cellapp/Entity.md#onEnteredView, [onLeaveTrap](/en/zh/api/kbengine/cellapp/Entity.md#onLeaveTrap, [onLoseControlledBy](/en/zh/api/kbengine/cellapp/Entity.md#onLoseControlledBy, [onMove](/en/zh/api/kbengine/cellapp/Entity.md#onMove, [onMoveOver](/en/zh/api/kbengine/cellapp/Entity.md#onMoveOver, [onMoveFailure](/en/zh/api/kbengine/cellapp/Entity.md#onMoveFailure, [onSpaceGone](/en/zh/api/kbengine/cellapp/Entity.md#onSpaceGone, [onTurn](/en/zh/api/kbengine/cellapp/Entity.md#onTurn, [onTeleport](/en/zh/api/kbengine/cellapp/Entity.md#onTeleport, [onTeleportFailure](/en/zh/api/kbengine/cellapp/Entity.md#onTeleportFailure, [onTeleportSuccess](/en/zh/api/kbengine/cellapp/Entity.md#onTeleportSuccess, [onUpdateBegin](/en/zh/api/kbengine/cellapp/Entity.md#onUpdateBegin, [onUpdateEnd](/en/zh/api/kbengine/cellapp/Entity.md#onUpdateEnd, [onWitnessed](/en/zh/api/kbengine/cellapp/Entity.md#onWitnessed | [Cell Entity 回调触发点矩阵](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix |
| 已深入解析 | [setViewRadius](/en/zh/api/kbengine/cellapp/Entity.md#setViewRadius, [getViewRadius](/en/zh/api/kbengine/cellapp/Entity.md#getViewRadius, [getViewHystArea](/en/zh/api/kbengine/cellapp/Entity.md#getViewHystArea, [entitiesInView](/en/zh/api/kbengine/cellapp/Entity.md#entitiesInView, [getWitnesses](/en/zh/api/kbengine/cellapp/Entity.md#getWitnesses | [Witness 运行态 API](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis |
| 已深入解析 | [canNavigate](/en/zh/api/kbengine/cellapp/Entity.md#canNavigate, [destroySpace](/en/zh/api/kbengine/cellapp/Entity.md#destroySpace, [debugView](/en/zh/api/kbengine/cellapp/Entity.md#debugView, [entitiesInRange](/en/zh/api/kbengine/cellapp/Entity.md#entitiesInRange, [getRandomPoints](/en/zh/api/kbengine/cellapp/Entity.md#getRandomPoints | [空间运行态 API](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-space-runtime-apis |
| 已深入解析 | [accelerate](/en/zh/api/kbengine/cellapp/Entity.md#accelerate, [getComponent](/en/zh/api/kbengine/cellapp/Entity.md#getComponent | [控制器调速与组件访问 API](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis |
| 已深入解析 | [clientEntity](/en/zh/api/kbengine/cellapp/Entity.md#clientEntity, [isReal](/en/zh/api/kbengine/cellapp/Entity.md#isReal, [onGetWitness](/en/zh/api/kbengine/cellapp/Entity.md#onGetWitness, [onLoseWitness](/en/zh/api/kbengine/cellapp/Entity.md#onLoseWitness | [Witness 运行态 API](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-witness-runtime-apis |
| 已深入解析 | [destroy](/en/zh/api/kbengine/cellapp/Entity.md#destroy, [onDestroy](/en/zh/api/kbengine/cellapp/Entity.md#onDestroy | [Cell 销毁与摘链](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-destroy-lifecycle |
| 已深入解析 | [onRestore](/en/zh/api/kbengine/cellapp/Entity.md#onRestore, [onWriteToDB](/en/zh/api/kbengine/cellapp/Entity.md#onWriteToDB, [writeToDB](/en/zh/api/kbengine/cellapp/Entity.md#writeToDB | [持久化与数据库](/en/zh/architecture/source-analysis/persistence.md |
| 已深入解析 | [addYawRotator](/en/zh/api/kbengine/cellapp/Entity.md#addYawRotator, [addProximity](/en/zh/api/kbengine/cellapp/Entity.md#addProximity, [teleport](/en/zh/api/kbengine/cellapp/Entity.md#teleport | [控制器调速与组件访问 API](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-controller-component-apis 与 [Cell Entity 回调触发点矩阵](/en/zh/architecture/source-analysis/space-aoi.md#cell-entity-callback-trigger-matrix |

## cellapp/KBEngine

API 页面：[docs/api/kbengine/cellapp/KBEngine.md](/en/zh/api/kbengine/cellapp/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [cellAppData](/en/zh/api/kbengine/cellapp/KBEngine.md#cellAppData, [globalData](/en/zh/api/kbengine/cellapp/KBEngine.md#globalData, [onCellAppData](/en/zh/api/kbengine/cellapp/KBEngine.md#onCellAppData, [onGlobalData](/en/zh/api/kbengine/cellapp/KBEngine.md#onGlobalData | [全局数据同步链](/en/zh/architecture/source-analysis/networking.md#global-data-dicts-sync |
| 已深入解析 | [MemoryStream](/en/zh/api/kbengine/cellapp/KBEngine.md#MemoryStream, [createEntity](/en/zh/api/kbengine/cellapp/KBEngine.md#createEntity, [publish](/en/zh/api/kbengine/cellapp/KBEngine.md#publish, [reloadScript](/en/zh/api/kbengine/cellapp/KBEngine.md#reloadScript, [scriptLogType](/en/zh/api/kbengine/cellapp/KBEngine.md#scriptLogType, [time](/en/zh/api/kbengine/cellapp/KBEngine.md#time, [onInit](/en/zh/api/kbengine/cellapp/KBEngine.md#onInit, [onSpaceData](/en/zh/api/kbengine/cellapp/KBEngine.md#onSpaceData, [onSpaceGeometryLoaded](/en/zh/api/kbengine/cellapp/KBEngine.md#onSpaceGeometryLoaded, [onAllSpaceGeometryLoaded](/en/zh/api/kbengine/cellapp/KBEngine.md#onAllSpaceGeometryLoaded | [CellApp 空间运行时 API](/en/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md |
| 已深入解析 | [getResFullPath](/en/zh/api/kbengine/cellapp/KBEngine.md#getResFullPath, [hasRes](/en/zh/api/kbengine/cellapp/KBEngine.md#hasRes, [listPathRes](/en/zh/api/kbengine/cellapp/KBEngine.md#listPathRes, [matchPath](/en/zh/api/kbengine/cellapp/KBEngine.md#matchPath, [open](/en/zh/api/kbengine/cellapp/KBEngine.md#open | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [addSpaceGeometryMapping](/en/zh/api/kbengine/cellapp/KBEngine.md#addSpaceGeometryMapping, [delSpaceData](/en/zh/api/kbengine/cellapp/KBEngine.md#delSpaceData, [getSpaceData](/en/zh/api/kbengine/cellapp/KBEngine.md#getSpaceData, [getSpaceGeometryMapping](/en/zh/api/kbengine/cellapp/KBEngine.md#getSpaceGeometryMapping, [setSpaceData](/en/zh/api/kbengine/cellapp/KBEngine.md#setSpaceData | [CellApp 空间运行时 API](/en/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md |
| 已深入解析 | [addWatcher](/en/zh/api/kbengine/cellapp/KBEngine.md#addWatcher, [delWatcher](/en/zh/api/kbengine/cellapp/KBEngine.md#delWatcher, [getWatcher](/en/zh/api/kbengine/cellapp/KBEngine.md#getWatcher, [getWatcherDir](/en/zh/api/kbengine/cellapp/KBEngine.md#getWatcherDir | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [address](/en/zh/api/kbengine/cellapp/KBEngine.md#address, [debugTracing](/en/zh/api/kbengine/cellapp/KBEngine.md#debugTracing, [getAppFlags](/en/zh/api/kbengine/cellapp/KBEngine.md#getAppFlags, [setAppFlags](/en/zh/api/kbengine/cellapp/KBEngine.md#setAppFlags, [isShuttingDown](/en/zh/api/kbengine/cellapp/KBEngine.md#isShuttingDown, [genUUID64](/en/zh/api/kbengine/cellapp/KBEngine.md#genUUID64 | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [executeRawDatabaseCommand](/en/zh/api/kbengine/cellapp/KBEngine.md#executeRawDatabaseCommand, [raycast](/en/zh/api/kbengine/cellapp/KBEngine.md#raycast | [CellApp 空间运行时 API](/en/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md |
| 已深入解析 | [urlopen](/en/zh/api/kbengine/cellapp/KBEngine.md#urlopen | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [registerReadFileDescriptor](/en/zh/api/kbengine/cellapp/KBEngine.md#registerReadFileDescriptor, [registerWriteFileDescriptor](/en/zh/api/kbengine/cellapp/KBEngine.md#registerWriteFileDescriptor, [deregisterReadFileDescriptor](/en/zh/api/kbengine/cellapp/KBEngine.md#deregisterReadFileDescriptor, [deregisterWriteFileDescriptor](/en/zh/api/kbengine/cellapp/KBEngine.md#deregisterWriteFileDescriptor | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |
| 已深入解析 | [onCellAppDataDel](/en/zh/api/kbengine/cellapp/KBEngine.md#onCellAppDataDel, [onGlobalDataDel](/en/zh/api/kbengine/cellapp/KBEngine.md#onGlobalDataDel, [onReadyForLogin](/en/zh/api/kbengine/cellapp/KBEngine.md#onReadyForLogin | [CellApp 空间运行时 API](/en/zh/architecture/source-analysis/cellapp-kbengine-space-runtime-api.md |

## client/Entity

API 页面：[docs/api/kbengine/client/Entity.md](/en/zh/api/kbengine/client/Entity.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [baseCall](/en/zh/api/kbengine/client/Entity.md#baseCall, [cellCall](/en/zh/api/kbengine/client/Entity.md#cellCall | [客户端远程调用链](/en/zh/architecture/source-analysis/networking.md#client-entity-remote-calls |
| 已深入解析 | [isPlayer](/en/zh/api/kbengine/client/Entity.md#isPlayer, [onEnterWorld](/en/zh/api/kbengine/client/Entity.md#onEnterWorld, [onLeaveWorld](/en/zh/api/kbengine/client/Entity.md#onLeaveWorld, [onEnterSpace](/en/zh/api/kbengine/client/Entity.md#onEnterSpace, [onLeaveSpace](/en/zh/api/kbengine/client/Entity.md#onLeaveSpace | [客户端实体生命周期](/en/zh/architecture/source-analysis/networking.md#client-entity-world-space-callbacks |
| 已深入解析 | [fireEvent](/en/zh/api/kbengine/client/Entity.md#fireEvent, [registerEvent](/en/zh/api/kbengine/client/Entity.md#registerEvent, [deregisterEvent](/en/zh/api/kbengine/client/Entity.md#deregisterEvent | [事件系统](/en/zh/architecture/source-analysis/events.md |
| 已深入解析 | [onDestroy](/en/zh/api/kbengine/client/Entity.md#onDestroy | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md |
| 已深入解析 | [getComponent](/en/zh/api/kbengine/client/Entity.md#getComponent | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md |

## client/KBEngine

API 页面：[docs/api/kbengine/client/KBEngine.md](/en/zh/api/kbengine/client/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [player](/en/zh/api/kbengine/client/KBEngine.md#player, [findEntity](/en/zh/api/kbengine/client/KBEngine.md#findEntity | [客户端句柄表](/en/zh/architecture/source-analysis/networking.md#client-entity-handles-table |
| 已深入解析 | [login](/en/zh/api/kbengine/client/KBEngine.md#login, [reloginBaseapp](/en/zh/api/kbengine/client/KBEngine.md#reloginBaseapp | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md |
| 已深入解析 | [createAccount](/en/zh/api/kbengine/client/KBEngine.md#createAccount, [getSpaceData](/en/zh/api/kbengine/client/KBEngine.md#getSpaceData | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md |
| 部分覆盖 | [resetPassword](/en/zh/api/kbengine/client/KBEngine.md#resetPassword, [bindAccountEmail](/en/zh/api/kbengine/client/KBEngine.md#bindAccountEmail, [newPassword](/en/zh/api/kbengine/client/KBEngine.md#newPassword | [客户端登录、重登录与 SpaceData API](/en/zh/architecture/source-analysis/client-login-and-space-data-api.md 已解释服务端链、客户端回包与 SDK 模板对照，但当前核心 `client_lib` 里仍未看到直接 Python 模块注册点。 |

## dbmgr/KBEngine

API 页面：[docs/api/kbengine/dbmgr/KBEngine.md](/en/zh/api/kbengine/dbmgr/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [executeRawDatabaseCommand](/en/zh/api/kbengine/dbmgr/KBEngine.md#executeRawDatabaseCommand, [onDBMgrReady](/en/zh/api/kbengine/dbmgr/KBEngine.md#onDBMgrReady, [onDBMgrShutDown](/en/zh/api/kbengine/dbmgr/KBEngine.md#onDBMgrShutDown, [onReadyForShutDown](/en/zh/api/kbengine/dbmgr/KBEngine.md#onReadyForShutDown, [onSelectAccountDBInterface](/en/zh/api/kbengine/dbmgr/KBEngine.md#onSelectAccountDBInterface | [组件型脚本 API](/en/zh/architecture/source-analysis/component-script-api.md |
| 已深入解析 | [addTimer](/en/zh/api/kbengine/dbmgr/KBEngine.md#addTimer, [delTimer](/en/zh/api/kbengine/dbmgr/KBEngine.md#delTimer, [urlopen](/en/zh/api/kbengine/dbmgr/KBEngine.md#urlopen | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |

## interfaces/KBEngine

API 页面：[docs/api/kbengine/interfaces/KBEngine.md](/en/zh/api/kbengine/interfaces/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [accountLoginResponse](/en/zh/api/kbengine/interfaces/KBEngine.md#accountLoginResponse, [createAccountResponse](/en/zh/api/kbengine/interfaces/KBEngine.md#createAccountResponse, [chargeResponse](/en/zh/api/kbengine/interfaces/KBEngine.md#chargeResponse, [executeRawDatabaseCommand](/en/zh/api/kbengine/interfaces/KBEngine.md#executeRawDatabaseCommand, [onInterfaceAppReady](/en/zh/api/kbengine/interfaces/KBEngine.md#onInterfaceAppReady, [onInterfaceAppShutDown](/en/zh/api/kbengine/interfaces/KBEngine.md#onInterfaceAppShutDown, [onRequestCreateAccount](/en/zh/api/kbengine/interfaces/KBEngine.md#onRequestCreateAccount, [onRequestAccountLogin](/en/zh/api/kbengine/interfaces/KBEngine.md#onRequestAccountLogin, [onRequestCharge](/en/zh/api/kbengine/interfaces/KBEngine.md#onRequestCharge | [组件型脚本 API](/en/zh/architecture/source-analysis/component-script-api.md |
| 已深入解析 | [addTimer](/en/zh/api/kbengine/interfaces/KBEngine.md#addTimer, [delTimer](/en/zh/api/kbengine/interfaces/KBEngine.md#delTimer, [urlopen](/en/zh/api/kbengine/interfaces/KBEngine.md#urlopen | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |

## logger/KBEngine

API 页面：[docs/api/kbengine/logger/KBEngine.md](/en/zh/api/kbengine/logger/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoggerAppReady](/en/zh/api/kbengine/logger/KBEngine.md#onLoggerAppReady, [onLoggerAppShutDown](/en/zh/api/kbengine/logger/KBEngine.md#onLoggerAppShutDown, [onLogWrote](/en/zh/api/kbengine/logger/KBEngine.md#onLogWrote, [onReadyForShutDown](/en/zh/api/kbengine/logger/KBEngine.md#onReadyForShutDown | [组件型脚本 API](/en/zh/architecture/source-analysis/component-script-api.md |
| 已深入解析 | [addTimer](/en/zh/api/kbengine/logger/KBEngine.md#addTimer, [delTimer](/en/zh/api/kbengine/logger/KBEngine.md#delTimer, [urlopen](/en/zh/api/kbengine/logger/KBEngine.md#urlopen | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |

## loginapp/KBEngine

API 页面：[docs/api/kbengine/loginapp/KBEngine.md](/en/zh/api/kbengine/loginapp/KBEngine.md

| 状态 | 接口 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | [onLoginAppReady](/en/zh/api/kbengine/loginapp/KBEngine.md#onLoginAppReady, [onLoginAppShutDown](/en/zh/api/kbengine/loginapp/KBEngine.md#onLoginAppShutDown, [onRequestLogin](/en/zh/api/kbengine/loginapp/KBEngine.md#onRequestLogin, [onLoginCallbackFromDB](/en/zh/api/kbengine/loginapp/KBEngine.md#onLoginCallbackFromDB, [onRequestCreateAccount](/en/zh/api/kbengine/loginapp/KBEngine.md#onRequestCreateAccount, [onCreateAccountCallbackFromDB](/en/zh/api/kbengine/loginapp/KBEngine.md#onCreateAccountCallbackFromDB | [组件型脚本 API](/en/zh/architecture/source-analysis/component-script-api.md |
| 已深入解析 | [addTimer](/en/zh/api/kbengine/loginapp/KBEngine.md#addTimer, [delTimer](/en/zh/api/kbengine/loginapp/KBEngine.md#delTimer, [urlopen](/en/zh/api/kbengine/loginapp/KBEngine.md#urlopen | [通用运行时工具 API](/en/zh/architecture/source-analysis/runtime-utility-api.md |

## 关键词与基础类型

API 页面：[docs/api/kbengine/keywords.md](/en/zh/api/kbengine/keywords.md、[docs/api/kbengine/basetypes.md](/en/zh/api/kbengine/basetypes.md

| 状态 | 项目 | 源码解析落点 / 缺口 |
| --- | --- | --- |
| 已深入解析 | `EntityCall`, `entity`, `base`, `cell`, `client` | [实体系统](/en/zh/architecture/source-analysis/entity-system.md、[网络与消息系统](/en/zh/architecture/source-analysis/networking.md |
| 已深入解析 | `View`, `Witness`, `Space`, `real`, `ghost`, `cellapp`, `baseapp` | [空间与 AOI](/en/zh/architecture/source-analysis/space-aoi.md、[进程模型](/en/zh/architecture/source-analysis/process-model.md |
| 已深入解析 | `entities.xml`, 基础数据类型, `ENTITYCALL`, `.def` 类型系统 | [类型系统与实体定义文件](/en/zh/architecture/source-analysis/entitydef-type-system.md |
| 已深入解析 | `KBE_ROOT`, `KBE_RES_PATH`, `kbengine_defaults.xml`, `kbengine.xml`, `vector3` | [运行时配置与基础类型](/en/zh/architecture/source-analysis/runtime-config-and-types.md |
| 已深入解析 | `KBE_HYBRID_PATH` | [运行时配置与基础类型](/en/zh/architecture/source-analysis/runtime-config-and-types.md 已说明：当前源码树未命中该名字，实际贯穿实现的是 `KBE_BIN_PATH`。 |

## 后续维护规则

- 新增源码解析专题时，应同步更新本页，把对应接口从“未覆盖”移动到“部分覆盖”或“已深入解析”。
- 若发现 KBEngine API 页与 CHM 不一致，只修 `docs/api/kbengine/**` 的错误；不要在 API 页中扩写源码解析。
- 如果某个 API 只适合保留 CHM 原文、不值得展开源码专题，也应在本页标注原因，避免重复排查。
