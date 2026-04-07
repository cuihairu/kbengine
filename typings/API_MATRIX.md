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
