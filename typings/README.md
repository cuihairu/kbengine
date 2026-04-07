# KBEngine Typings

这个目录用于存放 `KBEngine` 服务端 Python 脚本的静态类型桩。

## 当前策略

- `typings/KBEngine/__init__.pyi` 现在只承载“服务端公共层”。
- 各组件脚本目录通过同名包覆盖自己的 `KBEngine`：
  - `scripts/base/KBEngine/__init__.pyi`
  - `scripts/cell/KBEngine/__init__.pyi`
  - `scripts/interface/KBEngine/__init__.pyi`
  - `scripts/login/KBEngine/__init__.pyi`
  - `scripts/db/KBEngine/__init__.pyi`
  - `scripts/logger/KBEngine/__init__.pyi`
  - `scripts/bots/KBEngine/__init__.pyi`
- 公共定义与专属补丁分别沉到 helper：
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
- `SERVER_ERR_*` 常量全集由 `tools/generate_kbengine_server_errors_stub.py` 从 `kbe/src/lib/server/server_errors.h` 自动生成，并校验导出名字与 `SERVER_ERR_STR[]` 一致。
- 第一版优先解决 `import KBEngine`、`KBEngine.Proxy`、基础模块函数、基础实体类型的补全和检查。
- `base/cell` 维持服务端实体模型，`interfaces/login/db/logger` 复用更窄的 `PythonApp` 公共层，`bots` 单独按客户端脚本模型建模。

## `.pyi` 是如何被引用的

- Python 运行时不会导入 `.pyi`。
- IDE 和静态检查器会优先读取同名 `.pyi`，用它来推断 `.py` 的类型。
- 对这个仓库来说，类型来源分三层：
  - `typings/KBEngine/__init__.pyi`
    - 提供公共 `KBEngine` 包
  - `scripts/*/KBEngine/__init__.pyi`
    - 在各组件脚本目录内覆盖公共包，补充组件专属 API
  - 模板脚本同目录同名 `.pyi`
    - 例如 `scripts/base/Account.py` 对应 `scripts/base/Account.pyi`
    - 用来覆盖具体脚本的钩子签名和局部类型

## 如何配置

仓库根目录的 `pyrightconfig.json` 已经接好以下关键项：

- `executionEnvironments`
  - 按 `base` / `cell` / `interface` / `login` / `db` / `logger` / `bots` / `examples` 分开设置导入环境
  - 让 `base` 脚本优先解析 `scripts/base/KBEngine`
  - 让 `cell` 脚本优先解析 `scripts/cell/KBEngine`
  - 让 `interface` / `login` / `db` / `logger` / `bots` 也分别解析各自目录下的 `KBEngine`
- `extraPaths`
  - 每个环境显式加入自己的脚本目录、`scripts/common` 与 `typings`
- `stubPath = "typings/stubs"`
  - 仅保留给普通第三方 stub 搜索，避免覆盖 `KBEngine` 的目录级分流
- `include`
  - 当前会检查模板脚本目录和 `typings/examples`

如果其他工具要复用这套桩，最少要保证两件事：

- 能按目录给不同组件脚本建立不同导入环境
- 能让 `typings` 出现在搜索路径中，以解析 `_KBEngine_*` helper

## VS Code / Pylance 兼容性

- VS Code 默认常用的类型分析器是 `Pylance`。
- 这套 typing 方案以 `Pyright` 为基准设计，`Pylance` 可以直接消费同一套 `.pyi` 和 `pyrightconfig.json`。
- 对这个仓库来说，命令行 `npx -y pyright` 是基准检查结果，VS Code 中的 `Pylance` 应尽量和它保持一致。

推荐在工作区使用下面这份最小配置：

```json
{
  "python.analysis.diagnosticMode": "workspace",
  "python.analysis.diagnosticsSource": "Pyright"
}
```

建议：

- 用 VS Code 打开仓库根目录，而不是只单独打开某个 `scripts/*` 子目录
- 让 `Pylance` 读取仓库根目录的 `pyrightconfig.json`
- 把 `python.analysis.diagnosticMode` 设为 `workspace`
  - 否则默认只检查已打开文件，表现可能和命令行全量检查不一致

### Pylance 如何解析这个仓库

这个仓库的关键点不是只有 `.pyi`，而是：

- `pyrightconfig.json` 里的 `executionEnvironments`
- 各组件目录下的 `KBEngine/__init__.pyi`
- 同目录同名的旁路 `.pyi`

也就是说，`Pylance` 并不是简单地“看到 `import KBEngine` 就加载同一个桩”，而是会根据当前文件所在目录，切到对应的导入环境。

### 例子 1：同样是 `import KBEngine`，base 和 cell 看到的 API 不同

在 `base` 环境里：

```python
import KBEngine

entity = KBEngine.createEntityLocally("Account", {"nickname": "demo"})
KBEngine.baseAppData["mode"] = "pve"

# 这里应该报错，因为这是 cellapp 专属 API
KBEngine.setSpaceData(1, "weather", "sunny")
```

在 `cell` 环境里：

```python
import KBEngine

KBEngine.setSpaceData(1, "weather", "sunny")
mapping = KBEngine.getSpaceGeometryMapping(1)

# 这里应该报错，因为这是 baseapp 专属 API
KBEngine.createEntityLocally("Account", {"nickname": "demo"})
```

如果 `Pylance` 没有表现出这种差异，通常说明：

- 没有从仓库根目录打开工作区
- `pyrightconfig.json` 没有被正确拾取
- 当前文件不在预期的 `executionEnvironments` 范围内

### 例子 2：Pylance 会消费同目录 `.pyi`

例如：

- `scripts/base/Account.py`
- `scripts/base/Account.pyi`

当你在 VS Code 中打开 `scripts/base/Account.py` 时，`Pylance` 会优先参考旁边的 `Account.pyi` 来补全和检查钩子签名，而不是要求你把这些注解全部写回运行时脚本。

这种方式特别适合 `KBEngine` 这种由 C++ 注入运行时模块的场景，因为：

- 运行时脚本保持原样
- 类型信息放在 `.pyi`
- IDE 和静态检查器读取 `.pyi`
- 引擎实际运行逻辑不受影响

## 目录说明

- `typings/KBEngine/__init__.pyi`
  - `KBEngine` 公共包入口
- `typings/KBEngine/hooks.pyi`
  - `KBEngine.hooks` 公共包入口
- `typings/_KBEngine_common.pyi`
  - 公共类型与公共模块 API
- `typings/_KBEngine_server_errors.pyi`
  - 从 `server_errors.h` 生成的 `SERVER_SUCCESS` / `SERVER_ERR_*` / `SERVER_ERR_MAX`
- `typings/_KBEngine_pythonapp.pyi`
  - `PythonApp` 公共模块 API
- `typings/_KBEngine_baseapp.pyi`
  - `baseapp` 专属模块 API
- `typings/_KBEngine_cellapp.pyi`
  - `cellapp` 专属模块 API
- `typings/_KBEngine_interfaces.pyi`
  - `interfaces` 专属模块 API
- `typings/_KBEngine_loginapp.pyi`
  - `loginapp` 专属模块 API
- `typings/_KBEngine_dbmgr.pyi`
  - `dbmgr` 专属模块 API
- `typings/_KBEngine_logger.pyi`
  - `logger` 专属模块 API
- `typings/_KBEngine_bots.pyi`
  - `bots` 专属模块 API 与客户端侧近似类型
- `typings/_KBEngine_hooks.pyi`
  - 钩子协议的真实定义
- `typings/API_MATRIX.md`
  - typing 设计使用的 API 来源矩阵
  - 用于说明公共超集与组件专属接口的边界
- `typings/ENTITY_DEFS_GENERATION.md`
  - `entity_defs -> .pyi` 生成器的第一版设计
  - 用于约束后续实体桩自动生成的输入、输出和边界

## 模板脚本如何获得类型

- `import KBEngine`
  - 默认由 `typings/KBEngine/__init__.pyi` 提供公共包
  - 在各组件脚本目录下会被同名包覆盖成对应专属版本
- `kbe/res/sdk_templates/server/python_assets/scripts/*/*.py`
  - 优先使用同目录同名 `.pyi`
  - 例如 `scripts/base/kbemain.py` 对应 `scripts/base/kbemain.pyi`
- `if TYPE_CHECKING: from KBEngine.hooks import ...`
  - 用于额外引用钩子协议
  - 只能放在 `TYPE_CHECKING` 分支里，避免运行时导入不存在的 `KBEngine.hooks`

## 如何验证

在仓库根目录运行：

```powershell
python .\tools\validate_kbengine_typing.py
```

这条命令会顺序执行：

- `tools/generate_kbengine_server_errors_stub.py --check`
- `tools/generate_kbengine_entity_stubs.py --check`
- `python -m py_compile` 校验 typing 工具脚本语法
- `npx -y pyright -p .`

如果你要先把生成结果同步到最新，再跑完整校验：

```powershell
python .\tools\validate_kbengine_typing.py --mode sync
```

如果你只想看静态检查结果，也可以单独运行：

```powershell
npx -y pyright
```

当前主校验范围包括：

- `kbe/res/sdk_templates/server/python_assets/scripts`
- `typings/examples`

验证样例文件：

- `typings/examples/baseapp_usage.py`
  - 演示 base 环境下的 `KBEngine` 专属 API
- `typings/examples/cellapp/cellapp_usage.py`
  - 演示 cell 环境下的 `KBEngine` 专属 API
- `typings/examples/interfaceapp/interface_usage.py`
  - 演示 interface 环境下的 `KBEngine` 专属 API
- `typings/examples/loginapp/login_usage.py`
  - 演示 login 环境下的 `KBEngine` 专属 API
- `typings/examples/dbmgr/dbmgr_usage.py`
  - 演示 dbmgr 环境下的 `KBEngine` 专属 API
- `typings/examples/logger/logger_usage.py`
  - 演示 logger 环境下的 `KBEngine` 专属 API
- `typings/examples/bots/bots_usage.py`
  - 演示 bots 环境下的 `KBEngine` 专属 API

## 如何导出到旧项目

如果你不打算把旧项目放进这个仓库，而是希望把 typing 工具包单独导出，再导入本地旧项目，可以直接用这两个脚本：

- `tools/build_kbengine_typing_bundle.py`
  - 负责把当前仓库里的 typing 文件、生成器和安装器打成一个可直接丢到旧项目根目录的 drop-in 包
- `tools/install_kbengine_typing_bundle.py`
  - 负责把 payload 安装到目标项目，并在目标项目本地生成实体 `.pyi`

注意：

- 不建议只手工复制 `typings/` 到旧项目 `scripts` 的同层目录。
- 原因是这套方案除了 `typings/` 之外，还依赖落在旧项目 `scripts` 树里的旁路 stub：
  - `scripts/*/KBEngine/*.pyi`
  - `scripts/*/kbemain.pyi`
  - `scripts/interface/Poller.pyi`
  - `scripts/common/KBEDebug.pyi`
  - `scripts/common/Functor.pyi`
- 所以更稳妥的做法是复制 bundle，然后执行安装脚本，让它一次性把这些文件放到正确位置。

### 第一步：导出 bundle

在当前仓库根目录运行：

```powershell
python .\tools\build_kbengine_typing_bundle.py
```

默认输出：

- `.tmp/kbengine-typing-bundle/`
- `.tmp/kbengine-typing-bundle.zip`

导出后的目录结构大致如下：

```text
kbengine-typing-bundle/
  install_kbengine_typing_bundle.py
  install_kbengine_typing_bundle.cmd
  .kbengine_typing_payload/
    typings/
    scripts/
    tools/
```

### 第二步：导入到旧项目

推荐做法：

1. 把整个 `kbengine-typing-bundle/` 目录内容复制到旧项目根目录
2. 在旧项目根目录直接运行：

```powershell
python .\install_kbengine_typing_bundle.py
```

或者在 Windows 下直接运行：

```bat
install_kbengine_typing_bundle.cmd
```

这种模式下，安装器会自动推断：

- `project_root = 安装脚本所在目录`
- `bundle_root = ./.kbengine_typing_payload`
- `scripts_dir = ./scripts`

兼容路径：

如果你不想把 drop-in 包复制到项目根目录，也可以像以前一样在外部目录运行安装器，显式指定目标项目：

```powershell
python .\install_kbengine_typing_bundle.py `
  --project-root "D:/your-old-project" `
  --scripts-dir "D:/your-old-project/scripts"
```

安装器会做这些事：

- 复制 `typings/` 到旧项目根目录
- 复制各组件的 `scripts/*/KBEngine/*.pyi`
- 复制 `kbemain.pyi`、`Poller.pyi`、`KBEDebug.pyi`、`Functor.pyi`
- 复制 `tools/generate_kbengine_entity_stubs.py`
- 复制 `tools/generate_kbengine_server_errors_stub.py`
- 复制 `tools/validate_kbengine_typing.py`
- 默认在旧项目本地执行一次实体 `.pyi` 生成

### pyright 配置写入规则

安装器支持：

- `--pyright-config auto`
  - 如果目标项目没有 `pyrightconfig.json`，直接写入
  - 如果已经有，额外生成 `pyrightconfig.kbengine_typing.json`
- `--pyright-config always`
  - 总是直接写入 `pyrightconfig.json`
- `--pyright-config never`
  - 只生成 `pyrightconfig.kbengine_typing.json`

例子：

```powershell
python .\install_kbengine_typing_bundle.py `
  --project-root "D:/your-old-project" `
  --scripts-dir "D:/your-old-project/scripts" `
  --pyright-config never
```

### 只安装通用桩，不生成实体 `.pyi`

如果你只是想先把通用模块桩装进去，不立刻跑实体生成器：

```powershell
python .\install_kbengine_typing_bundle.py `
  --project-root "D:/your-old-project" `
  --scripts-dir "D:/your-old-project/scripts" `
  --skip-generate
```

### 这个 bundle 不会直接带走哪些文件

- 不会带模板工程自己的 `Account.pyi` 之类示例实体桩
- 真实项目实体 / 组件的 `.pyi` 仍然要依赖目标项目自己的：
  - `scripts/entities.xml`
  - `scripts/entity_defs/*.def`
  - `scripts/entity_defs/types.xml`

这样做的好处是：

- 旧项目不用上传到当前代码库
- 通用类型桩可以复用
- 实体类型桩仍然在旧项目本地按真实定义生成

## 如何生成错误码常量 `.pyi`

错误码常量桩由下面这个工具生成：

- `tools/generate_kbengine_server_errors_stub.py`

在仓库根目录运行：

```powershell
python .\tools\generate_kbengine_server_errors_stub.py
```

只检查是否过期：

```powershell
python .\tools\generate_kbengine_server_errors_stub.py --check
```

当前生成目标：

- `typings/_KBEngine_server_errors.pyi`

生成规则：

- 常量名以 `kbe/src/lib/server/server_errors.h` 为准
- 会额外校验 `SERVER_ERR_STR[]` 与宏定义顺序一致
- 因此 Python 侧真实导出的名字使用 `SERVER_ERR_IN_SHUTTINGDOWN`，而不是 XML 里的 `SERVER_ERR_SERVER_IN_SHUTTINGDOWN`

## 如何生成实体 `.pyi`

第一版生成器已经落在：

- `tools/generate_kbengine_entity_stubs.py`

在仓库根目录运行：

```powershell
python .\tools\generate_kbengine_entity_stubs.py
```

只检查是否需要更新：

```powershell
python .\tools\generate_kbengine_entity_stubs.py --check
```

当前生成范围：

- `scripts/base/<Entity>.pyi`
- `scripts/cell/<Entity>.pyi`
- `scripts/base/components/<Component>.pyi`
- `scripts/cell/components/<Component>.pyi`

当前职责边界：

- `kbemain.pyi` 继续手写维护
- 实体和组件 `.pyi` 由生成器接管
- 生成器目前已覆盖模板工程里的 `base/Account.pyi`
- `types.xml` 里的 `FIXED_DICT` 当前会按固定键结构生成 `TypedDict`
- 生成器会为实体生成专属 `BaseCall` / `CellCall` / `ClientCall`，用于收窄 `self.base` / `self.cell` / `self.client`
- 生成器会为组件生成专属 `BaseCall` / `CellCall` / `ClientCall`
- 生成器会把 `getComponent(componentTypeName, all=...)` 收窄成按组件类型名区分的 overload
- 实体远端调用对象上的组件属性，例如 `self.cell.combat`，也会收窄到组件专属远端类型

## 推荐用法

```python
import KBEngine

def onInit(isReload: bool) -> None:
    pass

class Account(KBEngine.Proxy):
    def onClientEnabled(self) -> None:
        pass
```

如果需要引用钩子协议：

```python
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from KBEngine.hooks import BaseAppModuleHooks
```

## 已知边界

- `KBEngine` 运行时模块由 C++ 注入，不存在真实的 `KBEngine.py`。
- 公共 `typings/KBEngine/__init__.pyi` 仍然是回退用的“公共超集”；精确结果依赖目录级覆盖包与 `pyright` 环境分流。
- 自定义实体属性、EntityCall 动态方法、运行时注册接口无法完全静态化，第一版会保留一定的 `Any`。
- 旁路 `.pyi` 只影响静态分析，不会改变脚本真实运行行为。
- 实体桩生成器当前已细化本地实体属性、本地方法、基础 hook，以及实体专属远端 `client/base/cell` 调用方法集。
- `FIXED_DICT` 当前按结构生成 `TypedDict`，`implementedBy` 只保留为运行时语义线索，暂未映射为具体 Python 类。
- 组件 `hasClient` 当前按脚本存在性、客户端可见属性、`ClientMethods`、以及服务端 `Exposed` 方法做近似推导。
- `bots` 当前按源码中的模块注入结果优先建模；其客户端侧 `Entity` / `ClientObject` 仍是近似模型，且文档列出的部分 `ClientApp` 风格模块函数并未在 `Bots::installPyModules()` 里直接看到。
