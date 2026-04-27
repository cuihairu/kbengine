# 文档重构 TODO

> 这页不是灵感清单，而是当前文档重构的执行看板。后续推进以这页为准，避免“做着做着漂移到别的专题”。  
> 判定原则也很简单：站点迁移已经基本完成，当前剩下的是**源码解析覆盖补全**与**少量结构收尾**。

## 完成定义

只有同时满足下面几条，才算“文档重构完成”：

- VitePress 站点可稳定构建，导航、Mermaid、页面结构正常。
- `docs-vuepress/**` 旧站内容已经完成迁移，不再依赖旧目录继续维护。
- `api-coverage.md` 中高频、高价值 API 不再大面积处于 `未覆盖`。
- `source-analysis/**` 已形成稳定专题目录，不再临时加页和改定位。
- API 页面继续保持和 CHM 一致，源码解析集中落在专题页，不再双写。

## 当前结论

| 维度 | 状态 | 说明 |
| --- | --- | --- |
| 站点迁移 | 已基本完成 | VitePress 可构建，导航和页面结构已切换完成。 |
| 源码解析骨架 | 已完成 | `entry/process/entity/space/network/persistence/scripting/events` 主专题已建立。 |
| API 覆盖补全 | 进行中 | 高频缺口已基本收口，当前主要剩 `client/KBEngine` 三个账号辅助接口的直注册入口边界。 |
| 收尾管理 | 进行中 | 需要这张 TODO 表持续收敛范围。 |

## 执行规则

- 每次只推进一个批次，不跨批次漂移。
- 每完成一个批次：
  - 更新对应专题页
  - 更新 `api-coverage.md`
  - 跑一次 `npm run docs:build`
  - 回写本页状态
- 如果临时发现新洞，先记到“新增缺口”，不打断当前批次。

## 批次总表

| 批次 | 优先级 | 范围 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| Batch 0 | P0 | 站点迁移可用性 | 已完成 | 构建、Mermaid、导航、目录切换已经稳定。 |
| Batch 1 | P0 | `cellapp/Entity` 高频 API | 已完成 | 高频接口已收尾，覆盖矩阵中的这批剩余项已转为已深入解析。 |
| Batch 2 | P0 | `baseapp/Entity` + `baseapp/KBEngine` 高频 API | 已完成 | `baseapp/Entity` 与 `baseapp/KBEngine` 高频 API 专题已补齐。 |
| Batch 3 | P1 | `baseapp/Proxy` 会话与流式传输 | 已完成 | `giveClientTo` 之外的连接管理、登录尝试与流式下载专题已补齐。 |
| Batch 4 | P1 | `cellapp/KBEngine` 组件侧 API | 已完成 | 空间几何、SpaceData、watcher、FD、工具 API 与初始化回调专题已补齐。 |
| Batch 5 | P1 | `client/KBEngine` + `bots/PyClientApp` | 已完成 | 已补登录 / 重登录 / SpaceData / 组件访问 / 销毁边界；账号辅助接口已补服务端链、客户端回包与 SDK 模板对照，并明确记录核心 `client_lib` 绑定入口差异。 |
| Batch 6 | P2 | `loginapp/interfaces/dbmgr/logger/bots` | 已完成 | 组件型脚本 API 已收束组件业务链，跨组件宿主能力已转移到 Batch 7 统一整理。 |
| Batch 7 | P2 | 通用工具专题 | 已完成 | `runtime-utility-api.md` 与 `runtime-config-and-types.md` 已收束宿主工具、配置加载链、环境变量路径链与 `vector3` 类型链。 |
| Batch 8 | P2 | 类型系统专题 | 已完成 | `entitydef-type-system.md` 已补 `types.xml / entities.xml / .def / ENTITYCALL / FIXED_DICT / ARRAY` 的协议链与源码边界。 |

## Batch 1：`cellapp/Entity` 收尾

状态：`已完成`

已完成：

- `moveToPoint / moveToEntity / navigate / cancelController`
- `fireEvent / registerEvent / deregisterEvent`
- `onEnterTrap / onEnteredView / onLeaveTrap / onLoseControlledBy`
- `onMove / onMoveOver / onMoveFailure / onTurn`
- `onSpaceGone / onTeleport* / onUpdateBegin / onUpdateEnd / onWitnessed`
- `setViewRadius / getViewRadius / getViewHystArea / entitiesInView / getWitnesses`
- `canNavigate / destroySpace / debugView / entitiesInRange / getRandomPoints`
- `accelerate / getComponent`
- `clientEntity / isReal / onGetWitness / onLoseWitness`
- `destroy / onDestroy`
- `onRestore / onWriteToDB / writeToDB`
- `addYawRotator / addProximity / teleport`

目标：

- 把 `cellapp/Entity` 从“主体讲完”推进到“高频接口基本齐”。
- 当前结果：已完成，下一步固定进入 Batch 2。

## Batch 2：`baseapp/Entity` + `baseapp/KBEngine`

状态：`已完成`

为什么优先：

- 当前覆盖矩阵里，这一块的高频缺口最多，而且直接影响实体主线闭环。
- `Base -> Cell -> DB -> Client` 的很多链路，缺的正是 Base 侧解释。

待办：

- `baseapp/Entity`
  - 已完成：`destroy / onDestroy`
  - 已完成：`writeToDB / onWriteToDB / onRestore`
  - 已完成：`createCellEntityInNewSpace`
  - 已完成：`destroyCellEntity / onCreateCellFailure / onLoseCell / onPreArchive`
  - 已完成：`getComponent`
- `baseapp/KBEngine`
  - 已完成：`createEntity* / createEntity*FromDBID / deleteEntityByDBID / lookUpEntityByDBID`
  - 已完成：`executeRawDatabaseCommand`
  - 已完成：`address / getAppFlags / setAppFlags / isShuttingDown / genUUID64 / quantumPassedPercent`
  - 已完成：`MemoryStream / publish / reloadScript / scriptLogType / time`
  - 已完成：`addWatcher / delWatcher / getWatcher / getWatcherDir`
  - 已完成：`registerReadFileDescriptor / registerWriteFileDescriptor / deregister*`
  - 已完成：`urlopen`
  - 已完成：`onBaseAppReady / onBaseAppShutDown / onCellAppDeath / onReadyForLogin / onReadyForShutDown / onAutoLoadEntityCreate`
  - 已完成：`getResFullPath / hasRes / listPathRes / matchPath / open`

已完成专题：

- `base-entity-lifecycle.md`
- `baseapp-kbengine-runtime-api.md`

下一步专题：

- `proxy-client-session-api.md`

## Batch 3：`baseapp/Proxy`

状态：`已完成`

已完成：

- `giveClientTo`
- `onClientEnabled`
- `disconnect`
- `getClientType`
- `getClientDatas`
- `streamFileToClient`
- `streamStringToClient`
- `onClientDeath`
- `onClientGetCell`
- `onGiveClientToFailure`
- `onLogOnAttempt`
- `onStreamComplete`

待办：
- 暂无

目标：

- 把“客户端归属迁移”扩展成完整的 Proxy 会话与流式传输专题。

已完成专题：

- `proxy-client-session-api.md`

## Batch 4：`cellapp/KBEngine`

状态：`已完成`

待办：
- 暂无

已完成：

- `addSpaceGeometryMapping / getSpaceGeometryMapping / onSpaceGeometryLoaded / onAllSpaceGeometryLoaded`
- `setSpaceData / getSpaceData / delSpaceData / onSpaceData`
- `addWatcher / delWatcher / getWatcher / getWatcherDir`
- `address / debugTracing / getAppFlags / setAppFlags / isShuttingDown / genUUID64`
- `MemoryStream / publish / reloadScript / scriptLogType / time`
- `executeRawDatabaseCommand / urlopen / raycast`
- `registerReadFileDescriptor / registerWriteFileDescriptor / deregister*`
- `onCellAppDataDel / onGlobalDataDel / onReadyForLogin / onInit`

已完成专题：

- `cellapp-kbengine-space-runtime-api.md`

## Batch 5：`client/KBEngine` + `bots/PyClientApp`

状态：`已完成`

待办：

- `client/KBEngine`
  - 已完成：`login / reloginBaseapp`
  - 已完成：`createAccount / resetPassword / bindAccountEmail / newPassword`
  - 已完成：`getSpaceData`
- `client/Entity` / `bots/Entity`
  - 已完成：`getComponent`
  - 已完成：`onDestroy`
- `bots/PyClientApp`
  - 已完成：`getSpaceData`

已完成专题：

- `client-login-and-space-data-api.md`

已确认边界：

- `resetPassword / bindAccountEmail / newPassword` 在当前源码树里已补服务端处理链、客户端回包语义与 SDK 模板对照。
- 当前仍没有在核心 `client_lib` 里看到明确的 Python 模块直注册入口，因此覆盖矩阵中这三项保留为“部分覆盖”。

## Batch 6：组件型 API

状态：`已完成`

范围：

- `loginapp/KBEngine`
- `interfaces/KBEngine`
- `dbmgr/KBEngine`
- `logger/KBEngine`
- `bots/KBEngine`

目标：

- 不再把这些组件 API 混在主线章节里零散提及，而是按组件职责成组说明。

已完成：

- 已完成专题：`component-script-api.md`
- 已完成：`loginapp` 的登录/建号入口回调与 DB 回流回调
- 已完成：`interfaces` 的登录/建号/充值请求与响应提交链
- 已完成：`dbmgr` 的账号库选择与原始 DB 命令执行链
- 已完成：`logger` 的日志写入回调与关闭闸门
- 已完成：`bots` 的批量接入控制面与客户端运行时继承边界

收尾说明：

- `addTimer / delTimer / urlopen / genUUID64 / watcher / FD` 这类跨组件通用宿主能力，已统一收束到 [通用运行时工具 API](/architecture/source-analysis/runtime-utility-api.md)。

## Batch 7：通用工具与配置专题

状态：`已完成`

范围：

- 通用运行时工具 API
- `kbengine_defaults.xml` / `kbengine.xml`
- `KBE_ROOT / KBE_RES_PATH / KBE_HYBRID_PATH`
- `vector3` 等基础运行时类型

已完成：

- 已完成专题：`runtime-utility-api.md`
- 已完成专题：`runtime-config-and-types.md`
- 已完成：`genUUID64 / urlopen` 的 `Script` 全局宿主边界
- 已完成：服务端 `addTimer / delTimer / register*deregister*FileDescriptor`
- 已完成：服务端 `addWatcher / delWatcher / getWatcher / getWatcherDir`
- 已完成：资源路径 API 与 `debugTracing`
- 已完成：客户端/Bots `callback / cancelCallback / getWatcher / getWatcherDir`
- 已完成：`address / isShuttingDown / getAppFlags / setAppFlags` 的组件实例态边界
- 已完成：`kbengine_defaults.xml / kbengine.xml` 的加载与覆盖链
- 已完成：`KBE_ROOT / KBE_RES_PATH / KBE_HYBRID_PATH` 关键词与当前源码主链 `KBE_BIN_PATH` 的差异说明
- 已完成：`vector3` 的 `math -> entitydef -> ScriptVector3 -> Entity.position/direction` 绑定链

## Batch 8：类型系统专题

状态：`已完成`

范围：

- `entities.xml`
- `entity_defs/*.def`
- `entity_defs/types.xml`
- `ENTITYCALL`
- `FIXED_DICT / ARRAY`

已完成：

- 已完成专题：`entitydef-type-system.md`
- 已完成：`EntityDef::initialize` 的 `types.xml -> entities.xml -> .def` 加载顺序说明
- 已完成：`entities.xml` 对 `ScriptDefModule` 注册顺序、`utype` 与 `hasBase / hasCell / hasClient` 断言的边界说明
- 已完成：`DataTypes::initialize / loadTypes` 的基础类型、别名类型、`ARRAY`、`FIXED_DICT` 注册链
- 已完成：`EntityCallType` 的判型、序列化、反序列化与持久化边界
- 已完成：`EntityCall::onScriptGetAttribute` 的远程方法、组件调用与 `base / cell / client` 链式分发说明

收尾说明：

- `vector3` 继续放在 [运行时配置与基础类型](/architecture/source-analysis/runtime-config-and-types.md) 里，不重复展开。
- 关键词与基础类型这一块在覆盖矩阵里已从“部分覆盖”收口为“已深入解析”。

## 新增缺口记录

后续如果发现新问题，先记这里，不打断当前批次：

- 暂无

## 下一步

下一步固定为：

1. 继续观察 `client/KBEngine` 的 `resetPassword / bindAccountEmail / newPassword` 是否还能在核心 `client_lib` 里找到更明确的 Python 直注册入口
2. 回刷 `api-coverage.md` 的总体快照与长尾 `未覆盖` 项，避免统计口径滞后
3. 继续清理覆盖矩阵中的低频遗留项
