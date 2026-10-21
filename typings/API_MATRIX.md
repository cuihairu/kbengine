# KBEngine Typing API Matrix

这个文档记录第一版 typing 使用的 API 来源矩阵，用于区分哪些接口属于公共超集，哪些接口属于组件特有能力。

## 设计原则

- `typings/KBEngine/__init__.pyi` 维护公共 `KBEngine` 包。
- 各组件脚本目录通过 `scripts/*/KBEngine/__init__.pyi` 覆盖公共包。
- 具体声明沉到 helper：
  - `typings/_KBEngine_common.pyi`
  - `typings/_KBEngine_server_errors.pyi`
  - `typings/_KBEngine_pythonapp.pyi`
  - `typings/_KBEngine_baseapp.pyi`
  - `typings/_KBEngine_cellapp.pyi`
  - `typings/_KBEngine_interfaces.pyi`
  - `typings/_KBEngine_loginapp.pyi`
  - `typings/_KBEngine_dbmgr.pyi`
  - `typings/_KBEngine_logger.pyi`
  - `typings/_KBEngine_bots.pyi`
  - `typings/_KBEngine_hooks.pyi`
- 模板脚本目录下的同名 `.pyi` 负责表达具体 `kbemain.py`、`Account.py` 的钩子签名。
- `pyrightconfig.json` 通过 `executionEnvironments` 按目录切换 `KBEngine` 的解析结果。

## 来源层级

### 公共服务端模块能力

主要来自以下入口：

- `kbe/src/lib/server/python_app.cpp`
- `kbe/src/lib/pyscript/script.cpp`
- `kbe/src/lib/pyscript/pyurl.cpp`

当前沉到 `typings/_KBEngine_pythonapp.pyi` 的典型接口：

- `MemoryStream`
- `publish`
- `scriptLogType`
- `getResFullPath`
- `hasRes`
- `listPathRes`
- `matchPath`
- `debugTracing`
- `genUUID64`
- `registerReadFileDescriptor`
- `registerWriteFileDescriptor`
- `deregisterReadFileDescriptor`
- `deregisterWriteFileDescriptor`
- `open`
- `addTimer`
- `delTimer`
- `urlopen`

### 公共错误码常量

主要来自：

- `kbe/src/lib/server/server_errors.h`
- `kbe/src/lib/server/entity_app.h`

说明：

- `EntityApp::installPyModules()` 会向公共 `KBEngine` 模块注入整套 `SERVER_SUCCESS` / `SERVER_ERR_*` 常量。
- `interfaces` / `loginapp` / `dbmgr` 也在各自的 `onInstallPyModules()` 里重复注入同一套错误码。
- typing 侧把这组常量集中放到 `typings/_KBEngine_server_errors.pyi`，并由生成器校验它与 `SERVER_ERR_STR[]` 顺序一致。

典型常量：

- `SERVER_SUCCESS`
- `SERVER_ERR_SRV_NO_READY`
- `SERVER_ERR_LOCAL_PROCESSING`
- `SERVER_ERR_DB`
- `SERVER_ERR_ACCOUNT_LOGIN_ANOTHER_SERVER`
- `SERVER_ERR_MAX`

### Interfaces 专属模块能力

主要来自：

- `kbe/src/server/tools/interfaces/interfaces.cpp`
- `docs-vuepress/api/interfaces/KBEngine.md`

典型接口：

- `accountLoginResponse`
- `createAccountResponse`
- `chargeResponse`
- `executeRawDatabaseCommand`
- 共享整套 `SERVER_SUCCESS` / `SERVER_ERR_*`

### LoginApp 专属模块能力

主要来自：

- `kbe/src/server/loginapp/loginapp.cpp`
- `docs-vuepress/api/loginapp/KBEngine.md`

典型接口：

- 共享整套 `SERVER_SUCCESS` / `SERVER_ERR_*`

说明：

- 源码里通过 `SERVER_ERR_STR` 注入一整组 `SERVER_ERR_*` 常量。
- typing 里这组常量已经从 `server_errors.h` 全量生成，不再只靠高频项手写补桩。

### DBMgr 专属模块能力

主要来自：

- `kbe/src/server/dbmgr/dbmgr.cpp`
- `docs-vuepress/api/dbmgr/KBEngine.md`

典型接口：

- `executeRawDatabaseCommand`
- 共享整套 `SERVER_SUCCESS` / `SERVER_ERR_*`

### Logger 专属模块能力

主要来自：

- `kbe/src/server/tools/logger/logger.cpp`
- `docs-vuepress/api/logger/KBEngine.md`

典型接口：

- 目前模块层主要复用 `PythonApp` 公共 API
- `kbemain.py` 钩子通过旁路 `.pyi` 精确表达：
  - `onLoggerAppReady`
  - `onLoggerAppShutDown`
  - `onReadyForShutDown`
  - `onLogWrote`

### Bots 专属模块能力

主要来自：

- `kbe/src/server/tools/bots/bots.cpp`
- `kbe/src/server/tools/bots/pybots.cpp`
- `kbe/src/lib/client_lib/clientobjectbase.cpp`
- `docs-vuepress/api/bots/KBEngine.md`

当前按源码建模后，模块层优先保证的接口：

- `addBots`
- `scriptLogType`
- `genUUID64`
- `urlopen`
- `bots`
- `component`
- `LOG_TYPE_*`

当前按客户端脚本模型近似提供的类型：

- `Entity`
- `EntityComponent`
- `Entities`
- `ClientObject`
- `PyBots`

### BaseApp 专属模块能力

主要来自：

- `kbe/src/server/baseapp/baseapp.cpp`
- `docs-vuepress/api/baseapp/KBEngine.md`

典型接口：

- `entities`
- `globalData`
- `baseAppData`
- `time`
- `isShuttingDown`
- `getWatcher`
- `getWatcherDir`
- `addWatcher`
- `delWatcher`
- `createEntity`
- `createEntityLocally`
- `createEntityAnywhere`
- `createEntityRemotely`
- `createEntityFromDBID`
- `createEntityAnywhereFromDBID`
- `createEntityRemotelyFromDBID`
- `charge`
- `deleteEntityByDBID`
- `lookUpEntityByDBID`
- `setAppFlags`
- `getAppFlags`
- `reloadScript`
- `address`
- `quantumPassedPercent`

### CellApp 专属模块能力

主要来自：

- `kbe/src/server/cellapp/cellapp.cpp`
- `docs-vuepress/api/cellapp/KBEngine.md`

典型接口：

- `entities`
- `globalData`
- `cellAppData`
- `time`
- `isShuttingDown`
- `getWatcher`
- `getWatcherDir`
- `addWatcher`
- `delWatcher`
- `createEntity(entityType, spaceID, position, direction, params)`
- `addSpaceGeometryMapping`
- `getSpaceGeometryMapping`
- `setSpaceData`
- `getSpaceData`
- `delSpaceData`
- `raycast`

## 基础类型来源

### Base 实体类型

来源：

- `kbe/src/server/baseapp/entity.cpp`
- `docs-vuepress/api/baseapp/Entity.md`

### Proxy 类型

来源：

- `kbe/src/server/baseapp/proxy.cpp`
- `docs-vuepress/api/baseapp/Proxy.md`

### Cell 实体类型

来源：

- `kbe/src/server/cellapp/entity.cpp`
- `docs-vuepress/api/cellapp/Entity.md`

### Space / EntityComponent / EntityCall

来源：

- `kbe/src/server/baseapp/space.cpp`
- `kbe/src/server/cellapp/space.cpp`
- `kbe/src/lib/entitydef/`
- `docs-vuepress/api/`

## 模板脚本钩子覆盖策略

### 模块级钩子

通过模板脚本旁路 `.pyi` 覆盖：

- `scripts/base/kbemain.pyi`
- `scripts/cell/kbemain.pyi`
- `scripts/login/kbemain.pyi`
- `scripts/interface/kbemain.pyi`
- `scripts/db/kbemain.pyi`
- `scripts/logger/kbemain.pyi`
- `scripts/bots/kbemain.pyi`

### 实体级钩子

通过模板实体旁路 `.pyi` 覆盖：

- `scripts/base/Account.pyi`
- `scripts/bots/Account.pyi`

公共参考协议仍放在：

- `typings/KBEngine/hooks.pyi`

## 当前边界

- `baseapp` / `cellapp` / `interfaces` / `loginapp` / `dbmgr` / `logger` / `bots` 已做目录级模块拆分。
- `EntityCall` 的远端方法和用户自定义实体属性仍是动态的，暂时保留 `Any`。
- `SERVER_ERR_*` 常量全集已经由 `server_errors.h` 自动生成；如果上游头文件调整，需要重新运行生成器同步。
- `bots` 文档中的模块级 API 与源码直接注入结果并不完全一致；第一版 typing 优先以源码注入为准，再对客户端对象做近似建模。

## 修正对账（2026-10-21）

依据 `typings/VALIDATION_REPORT.md`（对照 `kbe/src/` C++ 源码逐条核验）对本树 pyi 做的修正，逐条与报告清单一一对应：

### A. 无出处移除（13 条）

| 条目 | 处理 | 报告证据 |
|---|---|---|
| `_KBEngine_logger.pyi` 的 `accountLoginResponse` / `createAccountResponse` / `chargeResponse` / `executeRawDatabaseCommand` | 移除（logger.cpp 无任何脚本方法注册，logger 仅继承 PythonApp 公共面，`server/tools/logger/logger.h:49`；仅 `DatabaseCommandResult`/`DatabaseCommandCallback` 别名按报告"OK"保留并加注） | interfaces.cpp:186-189 为唯一注册点 |
| `hooks.BaseAppModuleHooks.onFini` | 移除（服务端无 "onFini" 调用点，仅 clientapp.cpp:304 "onFinish"） | 报告 §11 |
| `hooks.LoginAppModuleHooks.onTick` / `InterfacesModuleHooks.onTick` / `DBMgrModuleHooks.onTick` | 移除（全库无 "onTick" 调用点） | 报告 §11 |
| `hooks.BotsModuleHooks.onStart` | 移除（bots.cpp 仅 onInit(:103)/onFinish(:125)） | 报告 §11 |
| `bots.Entity.baseCall` / `Entity.cellCall` | 移除（全库无声明；远端调用走 `entity.base/cell` 的 EntityCall 路径，client_lib/entity.cpp:41-42） | 报告 §10 |
| `bots.Entity.isOnGround` / `Entity.inWorld` | 移除（仅 C++ 访问器 client_lib/entity.h:172/:167，未暴露给脚本） | 报告 §10 |

### B. 偏差修正（13 条）

| 条目 | 修正 | 源码出处 |
|---|---|---|
| `common.EntityCall` 三子类 `BaseEntityCall`/`CellEntityCall`/`ClientEntityCall` | 合回单一 `EntityCall`；Entity/EntityComponent 的 `client`/`cell`/`base`、`createCellEntity`、`clientEntity`、`teleport`、baseapp `createEntityRemotely(FromDBID)` 的类型同步收敛。`ENTITYCALL_TYPE` 为内部枚举（CELL=0/BASE=1/CLIENT=2/CELL_VIA_BASE=3/BASE_VIA_CELL=4/CLIENT_VIA_CELL=5/CLIENT_VIA_BASE=6），未注册为脚本可见常量，故以注释形式记录而不设 pyi 常量，避免再造无出处条目 | entity_call.h:15；lib/common/common.h:283-291 |
| `common.open` | 去掉 `encoding`，恰 2 参 `(res, mode="r")` | python_app.cpp:444-452 |
| `common.addTimer` | 去掉默认值，恰 3 参；注明 baseapp/cellapp 模块未注册该函数 | python_app.cpp:721-726 |
| `common.delTimer` | 返回 `int`（timerID，失败 -1） | python_app.cpp:766-771 |
| `common.listPathRes` | `extension: str`（不再接受 Tuple） | python_app.cpp:509-540 |
| `common.debugTracing` | 去掉 `self`，零参 | py_gc.cpp:224-228 |
| `common`/`pythonapp` 的 `MemoryStream.__bytes__` | 改为普通方法 `bytes()` | py_memorystream.cpp:30 |
| `baseapp.charge` | `pycallback` 必填（恰 4 参） | baseapp.cpp:3398-3403 |
| `cellapp.addSpaceGeometryMapping` | 前 3 参（spaceID/mapper/path）必填，3-5 参 | spacememory.cpp:144-147、:152-230 |
| `cellapp.raycast` | 3/4 参重载（3 参形式无 layer，默认 0），返回命中点元组 | cellapp.cpp:2139-2173 |
| `common.LOG_ON_*`（信息级） | 保留于公共超集，加注"仅 baseapp 注入" | baseapp.cpp:463-473 |
| `common.accountLoginResponse` 等 3 个（信息级） | 保留，加注"仅 interfaces 注册" | interfaces.cpp:186-188 |
| `common.Entity.cell`/`isReal` 等合并面（信息级） | 保留全量合并模型，报告中"适用范围注记"以注释落实 | baseapp/entity.cpp:35-41 vs cellapp/entity.cpp:47-90 |

### C. 缺漏补齐（39 条实体条目 + 4 条可选 dunder/内部项）

| 范围 | 条目 | 出处 |
|---|---|---|
| `common.Entity` cellapp 侧方法 ×8 | `addProximity(range_xz, range_y, userArg)`、`addYawRotator(yaw, velocity, userData)`、`navigate(dest, velocity, distance, maxMoveDistance, maxDistance, faceMovement, layer, userData)`、`navigatePathPoints(dest, maxSearchDistance, layer)`、`getRandomPoints(centerPos, maxRadius, maxPoints, layer)`、`moveToPoint(dest, velocity, distance, userData, faceMovement, moveVertically)`、`moveToEntity(...)`（6/7 参重载）、`accelerate(type, acceleration)`；签名照 `DECLARE_PY_MOTHOD_ARGn` 恒定实参解析面 | server/cellapp/entity.cpp:52-64、entity.h:203-491 |
| `common.Entity` cellapp 侧属性 ×13 | `allClients`、`otherClients`、`isWitnessed`、`hasWitness`、`isOnGround`、`spaceID`、`layer`、`position`、`direction`、`topSpeed`、`topSpeedY`、`controlledBy`、`volatileInfo` | server/cellapp/entity.cpp:76-90 |
| bots 模块面 ×7 | `publish`、`hasRes`、`open`、`listPathRes`、`matchPath`、`getWatcher`、`getWatcherDir` | lib/client_lib/clientapp.cpp:191-215 |
| bots Entity ×6 | `addTimer`、`delTimer`、`moveToPoint`、`cancelController`、`velocity`、`clientapp` | lib/client_lib/entity.cpp:29-46 |
| baseapp/cellapp 模块 ×2 | `kbassert`（common.pyi 亦补同源条目） | lib/server/entity_app.h:423 |
| `MemoryStream` ×3 | `rpos`（0/1 参重载）、`wpos`（0/1 参重载）、`fill(val)` | py_memorystream.cpp:540-646 |
| 可选 dunder/内部项 | `Entity.__reduce_ex__`/`__getDEP__`（entity_macro.h:15-16）、`MemoryStream.__reduce_ex__`（py_memorystream.cpp:34）已补；`EntityCall.__unpickle__` 因序列化协议内部签名未入 pyi；"volatile 语义"非 API 条目，仅在属性 `volatileInfo` 上体现 | entity_macro.h:15-16、entity_call.cpp:255-263 |

### D. 说明

- 修正仅覆盖报告判定范围，未引入任何报告之外的 API 条目；所有新增/修改处均内联 `file:line` 注释以便复核。
- `examples/*.py` 与 README 未在本次修正范围（任务边界：typings/** 的 .pyi 与本文件）。

