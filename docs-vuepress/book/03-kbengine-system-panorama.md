# 3. KBEngine 系统全景

> 上一章回答了"为什么会形成 BigWorld 这套模型"。这一章回答"KBEngine 在这个模型上具体采用了什么结构，又做了哪些取舍"。

## 3.1 本章核心问题

- KBEngine 和 BigWorld 的关系应该怎么理解？
- 五类进程的职责边界是什么？
- 进入源码阅读前，必须先记住哪几条系统边界？
- KBEngine 对 BigWorld 做了哪些关键取舍？

## 3.2 KBEngine 不是照搬 BigWorld，而是在同一模型上做开源化取舍

最容易出现的两种误解：

- **误解一**：KBEngine 就是 BigWorld 的"简化版复刻"
- **误解二**：KBEngine 是完全独立的一套 MMO 服务端模型

更准确的说法：

- KBEngine 明显**继承了 BigWorld 的核心运行模型**
- 但在**工程组织、技术栈、实现细节和开源可维护性**上做了自己的选择

理解 KBEngine 的最好方式：先承认它们共享一套深层模型，再看 KBEngine 在哪些地方做了变形与简化。

## 3.3 核心组件的继承骨架

KBEngine 的核心运行组件共享一套相对清晰的继承骨架：

```
ServerApp
  ├── EntityApp<Entity>        ← 加了 Entity 管理、EntityDef、Python 脚本
  │     ├── Baseapp            ← Base Entity + Proxy + Backup/Archive
  │     └── Cellapp            ← Cell Entity + Space + AOI + Witness + Ghost
  ├── PythonApp                ← 加了 Python 解释器初始化
  │     ├── Loginapp           ← 接入认证
  │     └── Dbmgr              ← 持久化仲裁
  ├── Baseappmgr               ← BaseApp 调度
  ├── Cellappmgr               ← CellApp 调度
  └── Machine                  ← 服务注册中心
```

源码层面的证据：

```cpp
// 文件：kbe/src/lib/server/serverapp.h:44
class ServerApp :
    public SignalHandler,
    public TimerHandler,
    public ShutdownHandler,
    public Network::ChannelTimeOutHandler,
    public Network::ChannelDeregisterHandler,
    public Components::ComponentsNotificationHandler
{
    // ...
};

// 文件：kbe/src/lib/server/entity_app.h:43
template<class E>
class EntityApp : public ServerApp
{
    // 加了 EntityDef、Entity 管理、ScriptTimers
    // ...
};
```

对比 BigWorld 的继承体系：

```
ServerApp
  ├── EntityApp          ← 加了 Entity 管理
  │     ├── ScriptApp    ← 再加 Python 运行时（BigWorld 多了一层）
  │     │     ├── BaseApp
  │     │     ├── CellApp
  │     │     └── DBApp
  │     └── (直接特化)
  ├── LoginApp
  └── ...
```

BigWorld 多了一层 `ScriptApp`，因为 BigWorld 把 Python 运行时初始化抽成了独立层。KBEngine 把这个功能合并进了 `EntityApp` 和 `PythonApp`。

## 3.4 五类进程的职责边界

### 接入层：LoginApp

```cpp
// 文件：kbe/src/server/loginapp/loginapp.h:22
class Loginapp : public PythonApp, public Singleton<Loginapp>
{
    // 关键方法：
    // login() — 处理登录请求
    // reqCreateAccount() — 创建账号
    // onLoginAccountQueryResultFromDbmgr() — DBMgr 返回账号查询结果
    // ...
};
```

**职责**：第一跳接入、认证、分发，把客户端导向正确的 BaseApp。它尽量不承载玩家长期业务状态，但会持有登录阶段的连接态和临时流程状态。

### 逻辑层：BaseApp

```cpp
// 文件：kbe/src/server/baseapp/baseapp.h:37
class Baseapp : public EntityApp<Entity>, public Singleton<Baseapp>
{
    // 关键方法：
    // createEntity() / createEntityAnywhere() / createEntityRemotely()
    // loginBaseapp() / logoutBaseapp() / reloginBaseapp()
    // handleBackup() / handleArchive()
    // onCellAppDeath() — CellApp 死亡处理
    // ...
};
```

**职责**：玩家长期逻辑主体、Proxy、客户端会话绑定、Base Entity 生命周期、与 DB 和 Cell 的协作。

### 空间层：CellApp

```cpp
// 文件：kbe/src/server/cellapp/cellapp.h:21
class Cellapp : public EntityApp<Entity>, public Singleton<Cellapp>
{
    // 关键方法：
    // handleGameTick() — 游戏主 tick
    // onCreateEntity() — 创建 Cell Entity
    // onRemoteCallMethodFromClient() — 客户端远程调用
    // onUpdateDataFromClient() — 客户端位置/属性更新
    // ...
};
```

**职责**：实时空间运行、Space / AOI / Witness、real / ghost 管理、空间内方法调用与同步。

### 持久化层：DBMgr

```cpp
// 文件：kbe/src/server/dbmgr/dbmgr.h:32
class Dbmgr : public PythonApp, public Singleton<Dbmgr>
{
    // 关键方法：
    // queryAccount() — 账号查询
    // writeEntity() / removeEntity() — 实体持久化
    // entityAutoLoad() — 启动时自动加载实体
    // onReqAllocEntityID() — 分配实体 ID 区间
    // initDB() — 初始化数据库连接
    // ...
};
```

**职责**：账号/实体状态查询、持久化/恢复/删除、在线/离线边界管理、实体 ID 分配。

BigWorld 把这个拆成了 `DBApp`（实际操作）+ `DBAppMgr`（集群协调），KBEngine 合为单个 `DBMgr`。

### 管理与外围层

| 组件 | 基类 | 职责 |
|------|------|------|
| BaseappMgr | ServerApp | BaseApp 调度：`findFreeBaseapp()` 分配最空闲的 BaseApp |
| CellappMgr | ServerApp | CellApp 调度：`findFreeCellapp()` 分配最空闲的 CellApp |
| Machine | ServerApp | 服务注册中心：UDP 广播发现、组件注册与查询 |
| Logger | — | 集中日志收集 |
| Interfaces | — | 外部接口网关（对接 billing 等） |
| Bots | — | 压测机器人 |

## 3.5 玩家主线：一条路径贯穿五类进程

最实用的系统图：记住这一条玩家主线。

```
Client ──TCP/KCP──▶ LoginApp
                       │
                       │ 认证请求
                       ▼
                     DBMgr（查账号/密码/在线状态）
                       │
                       │ 分配 BaseApp（通过 BaseappMgr）
                       ▼
                     BaseApp
                       │
                       ├── Proxy（客户端会话锚点）
                       ├── Base Entity（长期逻辑）
                       │
                       │ 组件消息（创建 Cell Entity）
                       ▼
                     CellApp
                       │
                       ├── Cell Entity（实时空间）
                       ├── Witness → Client（AOI 同步）
                       │
                       │ 必要时收束 Cell 侧空间态
                       ▼
                     Base / DBMgr（持久化）
```

**这条主线非常重要**，后面多数源码走读其实都是它的局部展开。

## 3.6 KBEngine 对 BigWorld 的几个关键取舍

### 取舍一：通信协议 — TCP 为主 vs UDP 为主

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 内部通信 | TCP | UDP + 自建可靠性 |
| 外部通信 | TCP / UDP / KCP | 以客户端接入 TCP 为主，内部路径更复杂 |
| 设计倾向 | 简单可靠 | 高性能定制 |

KBEngine 选择 TCP 做内部通信，牺牲了 UDP 的极致性能，但换来了实现简单和调试方便。

### 取舍二：RPC — 纯单向 vs TwoWay

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 默认模式 | Fire-and-forget | Fire-and-forget + TwoWay |
| 返回值 | 无（用 CallbackMgr 补偿） | 有（PyDeferred + Twisted） |
| 复杂度 | 低 | 高 |

KBEngine 砍掉了 TwoWay RPC，用更简单的 CallbackMgr 替代。代价是不能链式 then，但实现更直接。

### 取舍三：空间拓扑 — 简化 vs BSP 树

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 空间分割 | 无 BSP 树，SpaceMemory | BSP 树动态分割 |
| 负载均衡 | 简单分配 | CellAppGroup + MetaBalance + Offload |
| 动态扩容 | 无 | grow/shrink 边界移动 |

这是 KBEngine 最明显的简化。BigWorld 的 BSP 树空间拓扑是核心功能，KBEngine 完全砍掉了。

### 取舍四：进程恢复 — 无 vs Reviver

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 进程恢复 | 无自动拉起 | Reviver 自动检测和恢复 |
| 容错 | Backup/Archive 机制 | Backup + Reviver + Ghost 恢复 |

### 取舍五：注册中心 — 简单广播 vs 洪流发现

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| Machine | UDP 广播 + 查询 | bwmachined 洪流发现 + birth/death 监听器 |
| 持久化 | 无 | 注册表 save/load |
| 集群管理 | 轻量 | Cluster + Flood + Listeners |

### 取舍六：DB — 单 DBMgr vs DBApp + DBAppMgr

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 架构 | 单 DBMgr | DBApp（操作）+ DBAppMgr（协调） |
| 水平扩展 | 不支持 | 多 DBApp 可扩展 |
| 高速数据层 | Redis / SharedData 组合使用 | SharedData（内存共享） |

KBEngine 合并为单 DBMgr，不支持 DB 操作的水平扩展。同时引入 Redis 作为高速数据存储/后端选项，而不是只把它当成传统意义上的缓存。

### 取舍七：新增能力

KBEngine 也有一些 BigWorld 没有的东西：

- **KCP 协议支持**：外部通信可选 KCP（可靠的 UDP 变体）
- **Redis 后端**：`lib/db_redis/` 完整实现
- **guiconsole**：集成式 GUI 管理工具
- **更强调 Python 脚本驱动**：很多核心行为直接暴露到 Python 层

## 3.7 四条系统边界

不先建立边界意识，后面读代码会一直把职责看混。

### 接入边界

```
LoginApp → BaseApp
```

问题本质：第一跳接入 → 第二跳会话真正落地。

涉及的关键方法：`Loginapp::login()` → `Baseapp::loginBaseapp()`。

### 逻辑边界

```
BaseApp → CellApp
```

问题本质：长期逻辑主体 ↔ 实时空间主体。

涉及的关键方法：`Baseapp::createCellEntityInNewSpace()` → `Cellapp::onCreateCellEntityFromBaseapp()`。

### 持久化边界

```
Base / Cell → DBMgr
```

问题本质：运行态 ↔ 持久态。

涉及的关键方法：`Entity::writeToDB()` → `Dbmgr::writeEntity()` → `DBTaskWriteEntity`。

### 管理边界

```
*Mgr / Machine → 业务组件
```

问题本质：调度/注册/发现 ↔ 真正逻辑执行。

这一边界没有一条单线调用链，更准确地说，它由几个关键节点共同构成：`Machine::onBroadcastInterface()` 负责广播注册，`Baseappmgr::onAddComponent()` 负责把组件收进调度视图，`Baseapp::onGetEntityAppFromDbmgr()` / `Cellapp::onGetEntityAppFromDbmgr()` 负责建立业务组件之间的互连认知。

## 3.8 为什么四条边界对源码阅读特别重要

后面你会遇到很多"看起来都像处理玩家"的代码，但它们并不是一回事：

| 代码行为 | 属于哪条边界 |
|---------|------------|
| 登录成功回客户端地址 | 接入边界 |
| Proxy 绑定新客户端 | 逻辑边界 |
| 创建 Cell Entity | 逻辑 → 空间边界 |
| `writeToDB` | 持久化边界 |
| `onRegisterNewApp` | 管理边界 |

一旦边界感清楚，同一个"玩家行为"在不同组件里就不会再显得杂乱。

## 3.9 一张足够实用的脑内系统图

建议先在脑中固定这张图：

```
            ┌─────────────────────────────────────────────┐
            │              Client                          │
            └────────┬──────────────┬──────────────────────┘
                     │ TCP/KCP      │ TCP/KCP
                     ▼              │
               ┌──────────┐        │
               │ LoginApp │────────┤
               └─────┬────┘        │
                     │             │
            ┌────────▼────────┐    │
            │     DBMgr       │    │
            └────────┬────────┘    │
                     │             │
    ┌────────────────▼─────────────▼───────────────┐
    │              BaseApp                          │
    │    ┌─────────┐  ┌──────────────────┐         │
    │    │  Proxy   │  │   Base Entity    │         │
    │    └─────────┘  └──────────────────┘         │
    └──────────┬───────────────────────────────────┘
               │ EntityCall
    ┌──────────▼───────────────────────────────────┐
    │              CellApp                          │
    │    ┌──────────┐  ┌────────┐  ┌───────────┐  │
    │    │  Witness  │  │  Space │  │   Ghost    │  │
    │    └──────────┘  └────────┘  └───────────┘  │
    └──────────────────────────────────────────────┘

    外围：BaseappMgr / CellappMgr / Machine / Logger / Interfaces
```

迷路时问自己：**这段代码属于这张图里的哪一层？**

## 3.10 关键源码入口

| 组件 | 类声明文件 | 关键方法 |
|------|-----------|---------|
| ServerApp | `kbe/src/lib/server/serverapp.h` | `handleTimeout()` / `initializeBegin()` |
| EntityApp | `kbe/src/lib/server/entity_app.h` | 模板类，加了 Entity 管理 |
| Baseapp | `kbe/src/server/baseapp/baseapp.h` | `loginBaseapp()` / `handleBackup()` / `handleArchive()` |
| Cellapp | `kbe/src/server/cellapp/cellapp.h` | `handleGameTick()` / `onCreateEntity()` |
| Loginapp | `kbe/src/server/loginapp/loginapp.h` | `login()` / `onLoginAccountQueryResultFromDbmgr()` |
| Dbmgr | `kbe/src/server/dbmgr/dbmgr.h` | `queryAccount()` / `writeEntity()` / `entityAutoLoad()` |
| BaseappMgr | `kbe/src/server/baseappmgr/baseappmgr.h` | `findFreeBaseapp()` |
| CellappMgr | `kbe/src/server/cellappmgr/cellappmgr.h` | `findFreeCellapp()` |
| Machine | `kbe/src/server/machine/machine.h` | `onBroadcastInterface()` |

## 3.11 源码走读路径

### 路径一：理解继承层次

1. `kbe/src/lib/server/serverapp.h` — 基础骨架（Timer、Signal、Shutdown）
2. `kbe/src/lib/server/entity_app.h` — 在 ServerApp 上加了 Entity 管理
3. `kbe/src/server/baseapp/baseapp.h` — EntityApp → Baseapp 的特化
4. `kbe/src/server/cellapp/cellapp.h` — EntityApp → Cellapp 的特化

### 路径二：理解进程注册与发现

1. `kbe/src/server/machine/machine.h` — `onBroadcastInterface()` 注册
2. `kbe/src/server/baseappmgr/baseappmgr.h` — `onAddComponent()` 收录 BaseApp
3. `kbe/src/server/baseapp/baseapp.h` — `onGetEntityAppFromDbmgr()` 互相建立连接

### 路径三：理解调度决策

1. `kbe/src/server/baseappmgr/baseappmgr.h` — `findFreeBaseapp()` 最空闲策略
2. `kbe/src/server/cellappmgr/cellappmgr.h` — `findFreeCellapp()` 最空闲策略

## 3.12 小结

- KBEngine 和 BigWorld **共享核心 MMO 运行模型**（Login/Base/Cell/DB 四类分工）
- KBEngine 的**继承层次清晰**：ServerApp → EntityApp / PythonApp → 具体组件
- 系统至少可以拆成**接入、逻辑、空间、持久化、管理五类进程**
- 进入源码前，必须先记住**接入、逻辑、持久化、管理四条边界**
- KBEngine 的主要取舍：TCP 为主、砍掉 TwoWay RPC、砍掉 BSP 树空间拓扑、砍掉 Reviver、简化注册中心、合并 DBApp+DBAppMgr 为 DBMgr、引入 Redis 和 KCP

当这张系统地图建立起来后，后面再读启动流程、EntityDef、RPC、Space、DB 等章节，就不再是在一堆零散组件之间来回跳，而是在一张稳定框架图上逐步填细节。
