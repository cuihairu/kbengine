# EntityDef 到 `.pyi` 生成设计

这个文档定义 `entity_defs -> .pyi` 生成器的第一版边界，目标是把用户实体脚本的类型信息从手写维护，收敛到“由定义文件稳定生成”。

## 目标

- 从 `scripts/entities.xml`、`scripts/entity_defs/*.def`、`scripts/entity_defs/types.xml` 推导实体和组件的静态类型。
- 为 `base` / `cell` / `base/components` / `cell/components` 生成旁路 `.pyi`。
- 不引入新的运行时层，不修改 KBEngine 现有加载逻辑。

## 非目标

- 第一版不追求 100% 精确覆盖所有动态行为。
- 第一版不尝试重建网络序列化细节，例如 alias、utype、bundle 编码。
- 第一版不覆盖客户端脚本生成。
- 第一版不替代运行时校验，静态桩只服务 IDE 和静态检查器。

## 已确认的源码事实

### 定义加载顺序

`kbe/src/lib/entitydef/entitydef.cpp` 的主顺序是：

```text
loadAllDefDescriptions
  -> Properties
  -> CellMethods
  -> BaseMethods
  -> ClientMethods
loadInterfaces
loadComponents
loadParentClass
```

这意味着生成器也必须遵循同样的合并顺序：

- 当前实体自己的属性/方法先进入
- `Interfaces` 递归摊平合并
- `Components` 生成组件描述，并在宿主上增加组件属性
- `Parent` 最后继续向上加载

### `Interfaces` 与 `Components` 的语义差异

- `Interfaces`
  - 来自 `entity_defs/interfaces/*.def`
  - 直接合并进宿主实体定义
  - 没有独立组件实例
- `Components`
  - 来自 `entity_defs/components/*.def`
  - 会形成独立组件 `ScriptDefModule`
  - 宿主实体新增一个组件属性

因此生成器的输出规则必须是：

- `Interfaces`：摊平到宿主类
- `Components`：生成独立组件类桩，并在宿主类上挂组件属性

### Base / Cell / Client 归属不是只看 `.def`

`kbe/src/lib/entitydef/scriptdef_module.cpp` 的 `autoMatchCompOwn()` 表明：

- 实体或组件是否真正有 `base` / `cell` / `client` 部分，不只取决于 `.def`
- 还要结合：
  - `entities.xml` 上的 `hasBase` / `hasCell` / `hasClient`
  - 实际脚本文件是否存在
    - `scripts/base/<Entity>.py`
    - `scripts/cell/<Entity>.py`
    - `scripts/base/components/<Component>.py`
    - `scripts/cell/components/<Component>.py`

所以生成器不能只根据 `.def` 猜测要产出哪些桩，必须把脚本存在性一起作为输入。

## 输入面

第一版生成器读取以下输入：

- `scripts/entities.xml`
- `scripts/entity_defs/types.xml`
- `scripts/entity_defs/*.def`
- `scripts/entity_defs/interfaces/*.def`
- `scripts/entity_defs/components/*.def`
- `scripts/base/*.py`
- `scripts/cell/*.py`
- `scripts/base/components/*.py`
- `scripts/cell/components/*.py`

## 输出面

第一版默认采用“旁路同名 `.pyi`”策略，直接生成到脚本旁边：

- `scripts/base/<Entity>.pyi`
- `scripts/cell/<Entity>.pyi`
- `scripts/base/components/<Component>.pyi`
- `scripts/cell/components/<Component>.pyi`

这样做的原因很直接：

- `pyright` / Pylance / PyCharm 都天然支持同名 `.pyi`
- 不需要再引入复杂的模块搜索顺序
- 与当前模板桩策略一致，认知成本最低

第一版不额外设计独立 `outDir`。如果后续确实需要“源码目录零生成物”，再加可选输出目录。

## 与现有手写桩的关系

当前仓库已经存在两类手写桩：

- 模块级桩
  - 例如 `kbemain.pyi`
- 实体级桩
  - 例如 `base/Account.pyi`
  - 主要承载生命周期钩子和常见回调签名

后续生成器的文件所有权需要明确区分：

- `kbemain.pyi` 继续手写维护
- 实体/组件 `.pyi` 由生成器接管

原因很简单：

- `entity_defs` 能稳定提供属性、定义方法、组件关系
- 但实体生命周期钩子并不来自 `.def`

所以第一版实体桩生成时，需要把两部分内容合并到一个输出文件里：

- 公共基类和通用钩子模板
- `entity_defs` 推导出来的属性和定义方法

不要让“手写钩子桩”和“生成属性桩”各写同一个文件，否则后续只会不断互相覆盖。

## 生成内容

### 1. 实体基类选择

Base 侧：

- 如果实体有 `client` 部分，基类生成为 `KBEngine.Proxy`
- 否则生成为 `KBEngine.Entity`

Cell 侧：

- 一律生成为 `KBEngine.Entity`

组件侧：

- 生成为 `KBEngine.EntityComponent`

### 2. 属性生成

生成器需要把 `.def` 中的属性按可见侧筛到对应脚本类里：

- Base 可见属性 -> 出现在 `base/<Entity>.pyi`
- Cell 可见属性 -> 出现在 `cell/<Entity>.pyi`
- 组件属性 -> 出现在组件 `.pyi`
- 宿主实体额外获得组件属性本身，例如 `combat: Combat`

第一版不试图把所有同步语义编码进类型系统，只表达“这个属性在该侧能访问，且类型是什么”。

### 3. 方法生成

实体局部方法：

- `<BaseMethods>` -> 生成到 `base/<Entity>.pyi`
- `<CellMethods>` -> 生成到 `cell/<Entity>.pyi`

客户端方法：

- `<ClientMethods>` 不直接写成本地实体方法
- 生成器会把它们写入实体专属 `ClientCall` 远端引用类
- 同理：
  - `<BaseMethods>` 进入实体专属 `BaseCall`
  - `<CellMethods>` 进入实体专属 `CellCall`

当前实现会同时生成：

- 本地实体类自己的本地方法签名
- `self.base` / `self.cell` / `self.client` 对应的实体专属远端调用类型
- `clientEntity()` 的实体专属返回类型
- `getComponent(componentTypeName, all=...)` 按组件类型名生成 `Literal + overload`
- 组件专属 `BaseCall` / `CellCall` / `ClientCall`
- 实体远端调用对象上的组件属性，例如 `self.cell.combat`

### 3.1 公共钩子模板

由于 `.def` 不描述生命周期钩子，生成器需要按角色补一份固定模板：

- Base 且有 Client 的实体
  - 使用 `Proxy` 常见钩子模板
- Base 且无 Client 的实体
  - 使用普通 `Entity` 常见钩子模板
- Cell 实体
  - 使用 Cell 实体常见钩子模板
- 组件
  - 使用 `EntityComponent` 常见钩子模板

这些模板内容不从项目 `.def` 推导，而是来自当前仓库已经整理好的手写知识。

### 4. `Interfaces` / `Parent` 合并规则

生成器在内部构建一个“最终视图”：

- 先收集当前 `.def`
- 递归展开 `Interfaces`
- 递归展开 `Parent`

最终写到实体桩里的，是展开后的合并结果，而不是只看当前文件文本。

### 5. 类型映射

第一版采用保守映射：

| KBEngine 类型 | Python 类型 |
|---|---|
| `UINT8/16/32/64` | `int` |
| `INT8/16/32/64` | `int` |
| `FLOAT/DOUBLE` | `float` |
| `UNICODE/STRING` | `str` |
| `BLOB` | `bytes` |
| `PYTHON` | `Any` |
| `VECTOR2` | `tuple[float, float]` |
| `VECTOR3` | `tuple[float, float, float]` |
| `VECTOR4` | `tuple[float, float, float, float]` |
| `ARRAY<T>` | `list[T]` |
| `FIXED_DICT` | 见下文 |

### 6. `types.xml` 与 `FIXED_DICT`

`types.xml` 先于实体定义加载，因此生成器也必须先解析它。

第一版规则：

- 纯别名类型：递归展开到底层 Python 类型
- `ARRAY`：生成 `list[T]`
- `FIXED_DICT`：
  - 如果键集合可静态确定，生成 `TypedDict`
  - 当前实现优先按结构生成 `TypedDict`
  - `implementedBy` 暂时不映射为具体 Python 类，只保留为后续精化入口

这样做的原因很直接：`implementedBy` 往往依赖项目自己的 `user_type/` 布局，而固定键结构本身已经足够支撑 IDE 补全与静态检查。

## 生成器内部数据模型

第一版只需要下面几类中间结构：

- `EntityModel`
  - `name`
  - `has_base`
  - `has_cell`
  - `has_client`
  - `base_methods`
  - `cell_methods`
  - `client_methods`
  - `properties`
  - `components`
- `ComponentModel`
  - `name`
  - `type_name`
  - `has_base`
  - `has_cell`
  - `properties`
  - `base_methods`
  - `cell_methods`
- `TypeModel`
  - `name`
  - `python_type_repr`
  - `kind`

不需要一开始就完全镜像 C++ 里的 `ScriptDefModule` / `PropertyDescription` / `MethodDescription`。

## 建议实现步骤

### 第一步：只做解析与建模

- 解析 `entities.xml`
- 解析 `types.xml`
- 递归解析 `.def`
- 产出内存里的 `EntityModel` / `ComponentModel`

这一阶段先不写文件，先打印或测试模型结果。

### 第二步：生成旁路 `.pyi`

- 为存在的 `base/*.py` / `cell/*.py` 生成同名 `.pyi`
- 为 `base/components` / `cell/components` 生成组件 `.pyi`
- 组件桩允许以 `pyi-only` 形式存在，用来承载组件远端调用类型
  - 例如某组件只在 Base 本地存在，但 Cell 侧实体桩仍需要引用它的 `BaseCall`
- 本地组件类本身仍只在对应脚本侧生成，避免伪造不存在的本地对象

### 第三步：把生成器接到校验流程

- 执行生成
- 跑 `npx -y pyright`
- 记录剩余误报与无法表达的动态边界

## CLI 约定建议

第一版建议脚本路径：

- `tools/generate_kbengine_entity_stubs.py`

建议参数：

```text
python tools/generate_kbengine_entity_stubs.py ^
  --scripts-dir ./kbe/res/sdk_templates/server/python_assets/scripts ^
  --mode adjacent
```

参数只保留必要最小集：

- `--scripts-dir`
- `--mode adjacent`
- `--check`

不要一开始就加模板、缓存、watch、并行等能力。

当前仓库已经按这一路径落地了第一版工具，实现范围与本设计一致。

## 已知边界

- `implementedBy` 可能依赖项目自定义 Python 模块，无法保证全部静态可解析
- 组件 `hasClient` 当前按脚本存在性、客户端可见属性、`ClientMethods`、以及服务端 `Exposed` 方法做近似推导
- `baseapp` / `cellapp` 专属 `KBEngine` API 仍未拆成更严格的独立模块桩
- 如果用户脚本类本身与 `.def` 不一致，`.pyi` 只能反映定义侧，不负责修复实现侧问题

## 第一版完成标准

- 改动 `entity_defs/*.def` 或 `types.xml` 后，可以稳定再生成 `.pyi`
- Base / Cell 实体类的属性和本地方法能获得补全
- 组件类与宿主组件属性能获得补全
- `getComponent()` 和实体/组件远端调用上的组件属性能获得基础收窄
- `npx -y pyright` 可跑通模板工程或目标工程
