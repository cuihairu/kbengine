# KBEngine Typing TODO

最后更新: 2026-04-07

## 目标

- 为 `KBEngine` 服务端 Python 脚本提供可被 IDE 和静态检查器消费的类型信息。
- 不改动现有 C++/Python 运行时桥接逻辑。
- 优先覆盖 `baseapp`、`cellapp` 和模板脚本里最常用的 API。

## 非目标

- 不追求把所有动态能力 100% 静态化。
- 暂不覆盖 Unity、UE、客户端 SDK 的 typing。
- 暂不处理 `BigWorld-Engine-14.4.1` 目录。
- 不把类型桩实现成新的运行时层，避免引入双份行为逻辑。

## 当前已确认的事实

- `KBEngine` Python 模块由 C++ 侧安装并注入，不是纯 Python 模块。
- `baseapp`、`cellapp`、`python_app` 暴露到 Python 的 `KBEngine` API 并不完全相同。
- 实体类方法、属性、模块函数大量依赖运行时注册与注入。
- 仓库当前没有面向 `KBEngine` 服务端脚本 API 的官方 `.pyi` 类型桩。

## 需要交付的内容

- `KBEngine` 模块公共 API 类型桩。
- `baseapp` / `cellapp` 差异 API 的类型表达方案。
- `Entity` / `Proxy` / `Space` / `EntityComponent` 等基础脚本类型桩。
- 常见模块级回调钩子类型定义，例如 `kbemain.py` 中的 `onInit`、`onReadyForShutDown`、`onBaseAppReady`、`onCellAppData`。
- 常见实体级回调钩子类型定义，例如 `onTimer`、`onDestroy`、`onEnterWorld`、`onTeleportSuccess`、`onClientEnabled`。
- 基于 `entity_defs` 或等价元信息的实体桩生成方案。
- 至少一种静态检查配置接入方案，当前基线为 `pyright`。
- 一份说明文档，解释使用方式、边界和已知限制。

## 源信息入口

- `kbe/src/lib/pyscript/script.cpp`
- `kbe/src/lib/pyscript/py_macros.h`
- `kbe/src/lib/server/entity_app.h`
- `kbe/src/lib/server/python_app.cpp`
- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/cellapp/cellapp.cpp`
- `kbe/src/server/baseapp/entity.cpp`
- `kbe/src/server/baseapp/proxy.cpp`
- `kbe/src/server/cellapp/entity.cpp`
- `kbe/src/lib/entitydef/py_entitydef.cpp`
- `kbe/res/sdk_templates/server/python_assets/scripts/`
- `docs-vuepress/api/`

## TODO

- [x] 整理 API 来源矩阵，区分公共 API、`baseapp` 专属 API、`cellapp` 专属 API、`python_app` 通用 API。
- [x] 确定类型桩目录结构，明确哪些文件手写、哪些文件生成。
- [x] 设计 `KBEngine` 模块的导入策略，解决不同进程同名模块 API 不同的问题。
- [x] 产出第一版 `KBEngine` 公共模块类型桩。
- [x] 产出第一版 `baseapp` 专属类型桩。
- [x] 产出第一版 `cellapp` 专属类型桩。
- [x] 为 `interfaces` / `loginapp` / `dbmgr` / `logger` / `bots` 拆出目录级 `KBEngine` 覆盖包。
- [x] 产出第一版基础脚本类型桩：`Entity`、`Proxy`、`Space`、`EntityComponent`。
- [x] 补齐常见模块级钩子签名：`baseapp`、`cellapp`、`loginapp`、`interfaces`、`dbmgr`、`logger`、`bots`。
- [x] 补齐常见实体级钩子签名：`baseapp.Entity`、`baseapp.Proxy`、`cellapp.Entity` 及模板中高频使用的回调。
- [x] 为常量和标志位补齐类型定义，例如 `LOG_TYPE_*`、`LOG_ON_*`、`NEXT_ONLY`。
- [x] 设计 `entity_defs` 到实体 `.pyi` 的生成流程。
- [x] 实现最小可用的实体桩生成器，并先覆盖模板工程示例。
- [x] 明确 `EntityCall`、`base`、`cell`、`client` 等动态对象的类型边界，必要处使用 `Protocol`、`Any` 或泛型近似。
- [x] 选定静态检查器并接入配置，优先考虑 `pyright`。
- [x] 为模板脚本增加验证样例，确认 IDE 补全和静态检查可用。
- [x] 写一份使用说明，说明如何生成、如何更新、哪些能力仍是动态的。
- [x] 做一轮完整验证，记录剩余误报、漏报和无法静态表达的部分。
- [x] 从 `server_errors.h` 自动生成 `SERVER_SUCCESS` / `SERVER_ERR_*` / `SERVER_ERR_MAX`，并校验与 `SERVER_ERR_STR[]` 一致。
- [x] 固化统一校验入口：`tools/validate_kbengine_typing.py`。

## 本轮验证记录

- 命令：`python .\tools\validate_kbengine_typing.py --mode sync`
- 命令：`python .\tools\validate_kbengine_typing.py`
- 日期：2026-04-07
- 结果：`0 errors, 0 warnings, 0 informations`
- base/cell 专属模块桩校验：
  - `typings/examples/baseapp_usage.py`
  - `typings/examples/cellapp/cellapp_usage.py`
  - 结果：随主 `pyright` 一起通过
- 其余组件专属模块桩校验：
  - `typings/examples/interfaceapp/interface_usage.py`
  - `typings/examples/loginapp/login_usage.py`
  - `typings/examples/dbmgr/dbmgr_usage.py`
  - `typings/examples/logger/logger_usage.py`
  - `typings/examples/bots/bots_usage.py`
  - 结果：随主 `pyright` 一起通过
- 组件夹具校验：
  - `npx -y pyright -p .tmp/kbengine_typing_components_fixture/pyrightconfig.base.json`
  - `npx -y pyright -p .tmp/kbengine_typing_components_fixture/pyrightconfig.cell.json`
  - 结果：两侧均为 `0 errors, 0 warnings, 0 informations`
- 已知剩余边界：
  - `entity_defs` 驱动的本地实体属性、本地方法、实体远端调用、组件远端调用、`getComponent()` 已能生成，但组件 `hasClient` 仍是近似推导
  - `implementedBy` 仍未映射为具体 Python 类，复杂自定义 user_type 仍需要人工补桩
  - `bots` 的客户端侧 `Entity` / `ClientObject` 目前是近似模型，后续还可以继续对齐 `client` 文档与 C++ 真实脚本暴露面
  - `SERVER_ERR_*` 常量全集已经自动生成；如果上游 `server_errors.h` 变更，需要重新运行生成器同步

## 本轮新增工具

- `tools/build_kbengine_typing_bundle.py`
  - 生成 drop-in 导入包，输出根目录安装器 + `.kbengine_typing_payload/`
- `tools/generate_kbengine_server_errors_stub.py`
  - 根据 `kbe/src/lib/server/server_errors.h` 生成 `typings/_KBEngine_server_errors.pyi`
  - 同时校验宏定义顺序、`SERVER_ERR_STR[]` 顺序和 `SERVER_ERR_MAX`
- `tools/install_kbengine_typing_bundle.py`
  - 支持 drop-in 无参安装：旧项目根目录直接运行
- `tools/validate_kbengine_typing.py`
  - 统一串起错误码生成校验、实体桩校验、`py_compile` 与 `pyright`

## 本轮生成器进展

- 工具：`tools/generate_kbengine_entity_stubs.py`
- 当前已实现：
  - 解析 `entities.xml`
  - 解析 `entity_defs/*.def`
  - 递归合并 `Interfaces` 与 `Parent`
  - 解析 `types.xml` 中的基础别名与 `ARRAY`
  - 解析 `types.xml` 中的 `FIXED_DICT` 并生成 `TypedDict`
  - 为实体生成专属 `BaseCall` / `CellCall` / `ClientCall` 远端调用桩
  - 将 `self.base` / `self.cell` / `self.client` / `clientEntity()` 收窄到实体专属远端类型
  - 为组件生成专属 `BaseCall` / `CellCall` / `ClientCall` 远端调用桩
  - 按组件类型名为本地实体与远端 `EntityCall.getComponent()` 生成 overload
  - 将实体远端调用对象上的组件属性收窄到组件专属远端类型
  - 按本地真实侧过滤实体/组件上的 `base` / `cell` / `client` 引用
  - 生成 `base` / `cell` / `components` 的旁路 `.pyi`
- 当前已覆盖模板工程示例：
  - `kbe/res/sdk_templates/server/python_assets/scripts/base/Account.pyi`
- 当前未完成：
  - 复杂 `Components / Interfaces / Parent` 真实项目样例覆盖

## 建议实施顺序

1. 先把 `KBEngine` 模块和基础脚本类型桩做出来，让模板脚本立刻受益。
2. 再做实体桩生成器，因为这部分依赖 `entity_defs` 和项目脚本结构。
3. 最后接入静态检查与文档，形成可维护闭环。

## 主要风险

- `KBEngine` 是运行时注入模型，源码里没有单一的“完整 Python API 真相文件”。
- 不同组件对同名模块 `KBEngine` 暴露的方法不同，静态建模必须显式分层。
- 业务实体来自用户脚本与 `entity_defs`，不适合纯手写维护。
- 某些动态行为只能近似建模，无法像纯静态框架那样完全精确。

## 完成定义

- 模板脚本中的 `import KBEngine`、`KBEngine.Proxy`、常用模块函数可以获得基础补全。
- 至少一套模板实体脚本可以通过静态检查，不依赖大面积 `Any`。
- 新增/修改 `entity_defs` 后，实体类型桩可以稳定再生成。
- 文档中明确说明能力范围和限制，避免误以为 typing 能替代运行时校验。
