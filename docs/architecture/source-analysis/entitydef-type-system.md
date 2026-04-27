# 类型系统与实体定义文件：`types.xml`、`entities.xml`、`.def` 与 `ENTITYCALL`

> 这一页我想把一组经常混在一起的问题拆开：
>
> - `entities.xml` 到底只是“实体清单”，还是还承担别的运行时语义
> - `.def` 里的 `<Type>` 为什么不是随便写一个字符串
> - `types.xml`、`ARRAY`、`FIXED_DICT` 是怎么真正进入实体系统的
> - `ENTITYCALL` 到底是“实体对象引用”还是“远程 mailbox”
>
> 前面的 [实体系统](/architecture/source-analysis/entity-system.md) 和 [脚本运行时与热重载](/architecture/source-analysis/scripting.md) 已经把主线搭起来了；这一页继续把“类型系统”和“实体定义文件”这半边补成可落源码的专题。

## 先给结论

我现在更愿意把这一组文件拆成四层来看：

- `types.xml`
  负责把“类型名字”注册成真正的 `DataType`
- `entities.xml`
  负责把“有哪些实体”注册成真正的 `ScriptDefModule`
- `<Entity>.def`
  负责把“这个实体有哪些属性、方法、组件”挂到对应 `ScriptDefModule`
- `ENTITYCALL`
  负责把“跨实体调用句柄”接到远程方法和组件调用链

```mermaid
flowchart TD
    A["entity_defs/types.xml"] --> B["DataTypes::initialize"]
    B --> C["内建类型 + 别名类型 + ARRAY + FIXED_DICT"]

    D["entities.xml"] --> E["EntityDef::initialize"]
    E --> F["registerNewScriptDefModule"]
    F --> G["ScriptDefModule / utype"]

    H["entity_defs/*.def"] --> E
    E --> I["loadDefInfo / loadDetailLevelInfo"]
    C --> I
    G --> I
    I --> J["属性 / 方法 / 组件 / flags / alias"]

    J --> K["createEntity / RPC / 属性同步 / 持久化 / SDK 生成"]
```

先压成几句结论：

- `types.xml` 不是注释性配置，而是实体类型系统真正的注册入口。
- `entities.xml` 不只是列名字，它还影响实体 `utype` 分配，以及 `hasBase / hasCell / hasClient` 的断言边界。
- `.def` 解析时，属性和方法用到的类型都必须已经能在 `DataTypes` 里找到。
- `ENTITYCALL` 不是普通持久化字段；它更接近“可序列化的远程调用句柄”。

## 第一层：真正的加载顺序是 `types.xml -> entities.xml -> .def`

`EntityDef::initialize(...)` 在当前源码树里的顺序很明确：

1. 先初始化 `DataTypes`
2. 再打开 `entities.xml`
3. 按 `entities.xml` 顺序为每个实体注册 `ScriptDefModule`
4. 再读取对应的 `entity_defs/<Entity>.def`
5. 最后才进入 `script::entitydef::initialize()` 和实体脚本模块加载

关键入口在：

- `kbe/src/lib/entitydef/entitydef.cpp`
- `kbe/src/lib/entitydef/datatypes.cpp`

这意味着我现在不再把这些文件理解成“并列配置”：

- `types.xml` 先回答“类型系统里有哪些合法名字”
- `entities.xml` 再回答“有哪些实体模块”
- `.def` 最后才能安全地引用这些类型和实体定义

```mermaid
sequenceDiagram
    participant D as DataTypes
    participant E as EntityDef
    participant M as ScriptDefModule
    participant F as *.def

    E->>D: initialize(types.xml)
    D-->>E: 注册基础类型 / ARRAY / FIXED_DICT / 别名
    E->>E: 读取 entities.xml
    E->>M: registerNewScriptDefModule(name)
    E->>F: 打开 entity_defs/<Entity>.def
    F-->>M: 属性 / 方法 / 组件 / detailLevel
```

这一层最容易忽略的点是：

- `.def` 里 `<Type>` 能不能解析，不是看脚本里有没有类，而是看 `DataTypes` 里有没有这个名字。
- `entities.xml` 里的顺序也不是无关紧要的展示顺序，它会影响后面的 `utype` 分配。

## 第二层：`entities.xml` 不只是清单，它还决定实体注册顺序和部分断言

### `entities.xml` 里的顺序会进入 `ScriptDefModule` 的 `utype`

`EntityDef::initialize(...)` 遍历 `entities.xml` 时，会对每个实体调用：

- `registerNewScriptDefModule(moduleName)`

而 `EntityDef::registerNewScriptDefModule(...)` 的实现会继续做：

- `__scriptTypeMappingUType[moduleName] = g_scriptUtype`
- `new ScriptDefModule(moduleName, g_scriptUtype++)`

所以这里更准确的理解是：

- `entities.xml` 决定了哪些实体会被注册
- 同时也决定了这些实体拿到的脚本 `utype` 顺序

这件事为什么重要：

- `EntityCallType::addToStream()` 会把实体的 `utype` 一起写进流
- 远端收到后需要靠 `utype` 找回对应 `ScriptDefModule`

也就是说，`entities.xml` 的顺序不是纯展示信息，它已经进入协议层。

### `hasBase / hasCell / hasClient` 不是只看 `.def`

`ScriptDefModule` 在判断实体某一部分是否存在时，源码里明确有两条依据：

1. `.def` 和脚本实现里是否真的有这一部分
2. `entities.xml` 里是否显式写了 `hasBase / hasCell / hasClient`

`kbe/src/lib/entitydef/scriptdef_module.cpp` 里还有一段注释把这件事说得很直白：

- 一种是实体定义和脚本里本来就存在这一部分
- 另一种是用户在 `entities.xml` 里显式声明存在，为了 Unity、HTML5 这类前端场景保留

所以我现在更愿意把 `entities.xml` 理解成：

- 不只是“注册表”
- 还是“实体分部断言表”

```mermaid
flowchart TD
    A["entities.xml"] --> B["实体名注册"]
    A --> C["hasBase / hasCell / hasClient 断言"]
    B --> D["ScriptDefModule(name, utype)"]
    C --> D
    E[".def / 脚本文件是否存在"] --> D
    D --> F["最终 Base / Cell / Client 部分是否成立"]
```

### 使用场景

- 新增一个实体时，先确认它已经进了 `entities.xml`，再谈 `.def` 和脚本是否会生效。
- 调试某个实体“为什么客户端部分明明有定义却没被当成 hasClient”时，先回头看 `entities.xml` 的显式属性和脚本文件是否存在。
- 如果要调整实体顺序，最好把它当成协议层变更看待，而不是普通排版修改。

## 第三层：`types.xml` 先注册内建类型，再扩展别名、`ARRAY`、`FIXED_DICT`

### 内建类型不是从 XML 读出来的，而是 C++ 先注册

`DataTypes::initialize(...)` 的前半段会先把基础类型直接注册进来：

- `UINT8 / UINT16 / UINT32 / UINT64`
- `INT8 / INT16 / INT32 / INT64`
- `STRING / UNICODE / FLOAT / DOUBLE`
- `PYTHON / PY_DICT / PY_TUPLE / PY_LIST`
- `ENTITYCALL / BLOB`
- `VECTOR2 / VECTOR3 / VECTOR4`

然后才调用：

- `loadTypes(file)`

也就是说，`types.xml` 不是把整套类型系统从零建起来，而是在“内建类型已存在”的前提下继续扩展。

### `types.xml` 的三种主要扩展方式

`DataTypes::loadTypes(...)` 里能看到三种分支：

1. `FIXED_DICT`
2. `ARRAY`
3. 普通别名

对应地，我现在会这样理解：

- `SomeAlias = UINT32`
  是普通类型别名
- `SomeList = ARRAY`
  是固定元素类型数组
- `SomeStruct = FIXED_DICT`
  是固定键集合的结构体类型

`DataTypes::addDataType(name, dataType)` 还会把名字同时写进：

- 原始名字映射
- 小写名字映射
- `uid -> DataType` 映射

所以类型查找不是单一表，而是一组并行索引。

## 第四层：`ARRAY` 和 `FIXED_DICT` 的关键不是“像不像 Python 容器”，而是“有没有固定协议”

### `ARRAY` 真正固定的是元素类型

`FixedArrayType::initialize(...)` 会先找：

- `<of>`

然后把数组元素类型继续解析成：

- 已存在的基础类型
- 或者嵌套的 `ARRAY`

写流时：

- 先写长度
- 再逐项递归调用元素类型的 `addToStream`

读流时：

- 先读长度
- 再逐项递归调用元素类型的 `createFromStream`

所以这里的重点不是“它像 list”，而是：

- 元素协议是固定的
- 网络序列化和持久化时可递归展开

### `FIXED_DICT` 真正固定的是键集合和每个键的类型

`FixedDictType::initialize(...)` 会读取：

- `<Properties>`
- 每个 key 的 `<Type>`
- 可选的 `<Persistent>`
- 可选的 `<DatabaseLength>`
- 可选的 `<implementedBy>`

当前源码里有两个特别值得记的边界：

1. 如果某个 key 的数据类型是 `ENTITYCALL`
   - 持久化会被强制降成 `false`
2. `implementedBy`
   - 会把这个固定结构额外挂到一个 Python 实现模块

所以 `FIXED_DICT` 更接近：

- 一个固定 schema 的结构化类型
- 而不是“任意 dict 都能塞进去”

```mermaid
flowchart TD
    A["types.xml"] --> B["ARRAY"]
    A --> C["FIXED_DICT"]

    B --> B1["解析 of"]
    B1 --> B2["元素类型 DataType"]
    B2 --> B3["长度 + 逐项递归序列化"]

    C --> C1["解析 Properties"]
    C1 --> C2["每个 key 绑定 DataType"]
    C2 --> C3["Persistent / DatabaseLength / implementedBy"]
    C3 --> C4["固定键集合 + 递归序列化"]
```

### 使用场景

- `ARRAY`
  适合“同构重复项”，例如背包格子列表、路径点列表。
- `FIXED_DICT`
  适合“字段固定的结构”，例如一条邮件头、一个战斗结算块。
- 如果只是想临时塞任意对象，`PYTHON / PY_DICT / PY_LIST` 更自由，但协议边界会更弱。

## 第五层：`ENTITYCALL` 不是实体本身，而是远程调用句柄

### 类型判定时，它接受两类东西

`EntityCallType::isSameType(...)` 当前接受：

- 真正的 `EntityCall`
- 或者一个实体脚本对象
- 另外也允许 `None`

这意味着脚本层写：

```python
self.target = otherEntity.base
```

和某些场景下直接把实体对象交给 `ENTITYCALL` 类型，底层都能接住。

### 序列化时，真正写进去的是四元组

`EntityCallType::addToStream(...)` 写流时会写入：

- `ENTITY_ID`
- `COMPONENT_ID`
- `ENTITYCALL_TYPE`
- `ENTITY_SCRIPT_UID`

如果传入的是实体对象，底层会先把它折算成当前组件上的：

- `base`
- `cell`
- 或 `client`

对应的调用句柄信息。

所以 `ENTITYCALL` 本质上不是“把整个实体对象序列化”，而是：

- 把能够重新定位远程实体的 mailbox 信息写进流

### 反序列化时，会优先尝试本地实体，再退回远程 mailbox

`EntityCallType::createFromStream(...)` 的顺序是：

1. 先读 `id / cid / type / utype`
2. 如果目标组件类型和当前组件相同
   - 先尝试 `EntityDef::tryGetEntity(cid, id)`
3. 本地拿不到，再 new 一个 `EntityCall`

也就是说，收到 `ENTITYCALL` 后，脚本层最后拿到的可能是：

- 本地已存在实体对象
- 或一个远程 `EntityCall`

这也是为什么我更愿意把它理解成：

- “可反解的实体调用句柄”
- 不是“保证一定是远端对象”

### `ENTITYCALL` 不能持久化

这里源码边界非常明确：

- `property.cpp` 在构造 `PropertyDescription` 时，如果数据类型是 `ENTITYCALL` 且标记了 persistent，就会强制关掉持久化。
- `FixedDictType::initialize(...)` 里，如果某个 key 的类型是 `ENTITYCALL`，同样会把这个 key 的 persistent 降成 `false`。

所以更准确的理解是：

- `ENTITYCALL` 适合运行时引用
- 不适合数据库持久化

## 第六层：脚本里看到的 `base / cell / client` 链，其实都挂在 `EntityCall` 的属性分发上

`EntityCall::onScriptGetAttribute(...)` 这一段很关键，它把三件事接到一起了：

1. 远程方法调用
2. 组件调用
3. `base / cell / client` 继续跳转

具体来说：

- 如果属性名命中方法描述
  - 返回 `RemoteEntityMethod`
- 如果属性名命中组件属性
  - 返回 `EntityComponentCall`
- 如果属性名是 `base / cell / client`
  - 再按当前 `ENTITYCALL_TYPE` 派生出下一跳 mailbox

所以脚本里这些写法：

```python
avatar.base.doSomething()
avatar.cell.moveToPoint(...)
avatar.client.onMatchSuccess()
```

背后不是语法糖，而是一条明确的属性分发链。

这里还有一个客户端边界：

- 在 `CLIENT_TYPE / BOTS_TYPE` 上，只允许访问 `isExposed()` 的方法。

所以客户端拿到 `ENTITYCALL` 后，也不是“什么方法都能调”，而是还要过 exposed 检查。

### 使用例子

```python
self.friendRef = other.base
if self.friendRef:
    self.friendRef.invite(self.id)
```

```python
mailbox = self.target.cell
mailbox.teleport(spaceID, position)
```

更适合 `ENTITYCALL` 的场景：

- 在线运行时跨实体调方法
- 暂存一个会话期引用

不适合它的场景：

- 想把目标实体引用安全写进数据库长期保存

## 第七层：`.def` 真正做的是“把已注册类型挂到属性和方法描述上”

前面几层都准备好之后，`EntityDef::initialize(...)` 才会继续：

- 打开每个 `<Entity>.def`
- `loadDefInfo(...)`
- `loadDetailLevelInfo(...)`

这时候 `.def` 里的 `<Type>` 才能被安全解析成真正的 `DataType*`。

所以 `.def` 在这一页里最重要的角色不是“再介绍实体系统一遍”，而是：

- 消费前面已经注册好的类型系统
- 把这些类型绑定到属性描述、方法参数、组件描述

这一层也解释了为什么：

- 改 `types.xml`
- 改 `entities.xml`
- 改 `.def`

这三件事我现在都把它们看成“协议层变更”，而不是普通脚本改动。

## 第八层：当前源码树还支持一条 Python 版 entitydef 路径，但底层仍复用同一套 `DataTypes`

这一点在 `py_entitydef.cpp` 里能直接看到。

如果走 Python 版定义路径，`DefContext` 最终仍会把：

- `FIXED_ARRAY`
- `FIXED_DICT`
- rename alias

继续注册回 `DataTypes`。

也就是说，当前源码里其实有两条入口：

- XML 入口
- Python entitydef 入口

但它们最后仍然汇合到同一套：

- `DataTypes`
- `ScriptDefModule`
- `EntityDef` 元数据

所以我现在更愿意把它记成：

- 入口可以不同
- 协议层核心容器还是同一套

## 这一页适合怎样使用

1. 想判断“为什么 `.def` 里这个 `<Type>` 报找不到”
   先看 `types.xml` 和 `DataTypes::loadTypes(...)`。
2. 想判断“为什么改了 `entities.xml` 后一些远程调用或 SDK 行为异常”
   先回头看实体注册顺序和 `utype`。
3. 想判断“`ENTITYCALL` 到底能不能写库”
   直接看 `property.cpp` 和 `FixedDictType::initialize(...)` 的强制降级逻辑。
4. 想判断“脚本里 `avatar.base.cell.client` 这种链为什么能工作”
   直接看 `EntityCall::onScriptGetAttribute(...)`。
5. 想给项目设计结构化数据类型
   优先考虑 `FIXED_DICT / ARRAY`，而不是先把一切都塞进 `PYTHON`。

## 使用场景

- `entities.xml`
  适合做实体注册和实体分部断言，不适合随意改顺序做排版整理。
- `types.xml`
  适合沉淀可复用的结构类型和别名。
- `FIXED_DICT`
  适合“字段稳定、协议明确”的结构。
- `ARRAY`
  适合“元素同构”的重复序列。
- `ENTITYCALL`
  适合运行期跨实体通信，不适合长期持久化。

## 与其他专题的关系

- 想看实体怎样被真正实例化，接着看 [实体系统](/architecture/source-analysis/entity-system.md)
- 想看脚本宿主、热重载和实体脚本类型怎样接起来，接着看 [脚本运行时与热重载](/architecture/source-analysis/scripting.md)
- `vector3`、配置路径、环境变量这条线，继续看 [运行时配置与基础类型](/architecture/source-analysis/runtime-config-and-types.md)
- 远程消息、RPC、`EntityCall` 最后怎么走到网络层，继续看 [网络与消息系统](/architecture/source-analysis/networking.md)

这一页最后我只想把边界收成一句话：

- `types.xml` 负责把“类型”注册好，
- `entities.xml` 负责把“实体模块”注册好，
- `.def` 负责把“这些类型挂到实体协议上”，
- `ENTITYCALL` 则把“跨实体调用句柄”接进运行时。
