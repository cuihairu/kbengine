# 2. BigWorld：问题、模型与核心概念

> 这一章不打算把 BigWorld 写成一段产品史。目标是让读者先建立"为什么会长成 Login / Base / Cell / DB 这套模型"的心智框架，然后把 Entity / Proxy / Space / Witness / Ghost 这些概念放回同一个运行模型里理解。

## 2.1 本章核心问题

- 大规模在线世界到底遇到了哪些传统游戏服务器很难承受的问题？
- 为什么这些问题最后会逼出 `Login / Base / Cell / DB` 四类分工？
- Entity 为什么一定会出现"多侧表示"？
- `Proxy / Space / Witness / Ghost` 在同一个运行模型里各自扮演什么角色？

## 2.2 先从问题出发，而不是先背术语

BigWorld 一系技术路线真正要解决的，不是"服务器怎么写得更优雅"，而是几类很具体的工程约束：

- **在线人数高**：单进程难以稳定承载所有玩家状态，一台机器的内存/CPU/网络都有上限
- **空间实时性**：场景内位置、可见性、移动和战斗是高频实时问题，tick 频率 10Hz（100ms 预算）
- **长期状态管理**：场景外账号、背包、任务等逻辑更接近长期状态管理，不需要实时空间
- **持久化隔离**：数据既要持久化，又不能让数据库成为实时状态来源
- **会话连续性**：玩家会掉线、重连、切图、跨进程迁移，在线状态不能一断就全丢

把这些问题放在一起看，就会发现：

- "一个玩家对象"并不足以表达所有运行态
- "一个服务器进程"也不足以承载整个世界

BigWorld 的价值不在于发明了一堆术语，而在于把这些约束组织成了一套能长期运行的模型。

## 2.3 为什么会形成 Login / Base / Cell / DB

把问题拆开，就会自然推出几种不同职责：

### 接入问题

客户端第一跳接入、认证、分发，不应该和玩家长期运行态绑死。于是有了：

**LoginApp** — 尽量不承载玩家长期业务状态的接入网关，负责认证、转发和登录阶段的临时会话处理。

### 长期逻辑问题

账号、背包、任务等长期逻辑，以及客户端会话绑定，需要一个长期持有玩家逻辑对象的地方。于是有了：

**BaseApp** — 玩家在线会话的长期锚点。

### 实时空间问题

位置、AOI（Area of Interest）、移动、可见性、空间内实时交互，不适合放进同一套长期逻辑上下文里。于是有了：

**CellApp** — 实时空间运行引擎。

### 持久化问题

数据库应该负责冷状态、恢复状态和在线元信息，不应该直接变成实时世界的一部分。于是有了：

**DBMgr**（KBEngine）/ **DBApp + DBAppMgr**（BigWorld）— 在线/离线边界管理者。

要注意这两者不是严格的一一对应：BigWorld 把持久化执行层和集群协调层拆开，KBEngine 则合并成单个 `DBMgr`。

所以 `Login / Base / Cell / DB` 并不是拍脑袋拆服务，而是被四类问题自然塑造出来的分工。

## 2.4 为什么 Entity 一定会出现"多侧表示"

一旦接受了 Base / Cell 分离，立刻遇到一个问题：一个玩家到底算一个实体，还是两个实体？

BigWorld 一系给出的答案：

- **语义上**它仍然是"同一个实体"
- **运行时上**它有不同侧的表示（multi-representation）

于是就有了：

| 侧 | 驻留进程 | 职责 |
|----|---------|------|
| Base Entity | BaseApp | 长期逻辑主体：背包、任务、社交、DB 交互 |
| Cell Entity | CellApp | 实时空间主体：位置、AOI、移动、战斗 |
| Client 表现 | 客户端 | 网络表现：接收同步、发送操作 |

核心思想：**一个游戏对象不是只能有一个物理驻留点**。不同侧分别承载它的不同职责。

在 BigWorld 源码中，Cell 侧 Entity 有明确的 real/ghost 区分：

```cpp
// 文件：programming/bigworld/server/cellapp/entity.hpp:138
/**
 *  Instances of this class are used to represent a generic game object on the
 *  cell. An entity may be real or ghosted. A ghost entity is an entity that
 *  is a copy of a real entity that lives on an adjacent cell.
 */
class Entity : public PyObjectPlus
{
    // ...
    bool isReal() const;
    bool isRealToScript() const;
    RealEntity * pReal() const;

    void initReal( BinaryIStream & data, ... );
    void initGhost( BinaryIStream & data );
    // ...
};
```

同一个 Entity 类，`initReal` 和 `initGhost` 是两条完全不同的初始化路径。

KBEngine 的 Cell 侧 Entity 也有同样的设计：

```cpp
// 文件：kbe/src/server/cellapp/entity.h:46
class Entity : public script::ScriptObject
{
    // ...
    INLINE bool isReal(void) const;
    // ...
};
```

## 2.5 Proxy：把"玩家在线连接"带进实体模型

`Proxy` 不是普通实体，而是把"客户端会话"带进实体模型的关键对象。

**它最重要的双重身份：**

- 它是一个 Entity（继承自 Base/Entity）
- 它是客户端会话的入口

一个玩家不只是数据库里的一条记录，也不只是世界里的一个坐标点，他**还是一个受客户端控制、可能重连、可能挤号、可能切换控制权的在线会话对象**。Proxy 就是这层语义的集中承载点。

### BigWorld Proxy

```cpp
// 文件：programming/bigworld/server/baseapp/proxy.hpp:74
/**
 *  This class is used to represent a proxy. A proxy is a special type of base.
 *  It has an associated client.
 */
class Proxy: public Base
{
    // ...
    // 断线原因枚举
    enum ClientDisconnectReason
    {
        CLIENT_DISCONNECT_CLIENT_REQUESTED,
        CLIENT_DISCONNECT_GIVEN_TO_OTHER_PROXY,   // 挤号
        CLIENT_DISCONNECT_RATE_LIMITS_EXCEEDED,
        CLIENT_DISCONNECT_TIMEOUT,
        CLIENT_DISCONNECT_BASE_RESTORE,           // Base 恢复
        CLIENT_DISCONNECT_SHUTDOWN,
        CLIENT_DISCONNECT_CELL_RESTORE_FAILED,
    };
    // ...
};
```

注意 `CLIENT_DISCONNECT_GIVEN_TO_OTHER_PROXY` — 挤号是 Proxy 级别的语义，不是 Entity 级别的。

### KBEngine Proxy

```cpp
// 文件：kbe/src/server/baseapp/proxy.h:28
class Proxy : public Entity
{
    // ...
    bool sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle);
    bool sendToClient(Network::Bundle* pBundle, bool immediately = false);
    double getRoundTripTime() const;
    double getTimeSinceHeardFromClient() const;
    // ...
};
```

Proxy 持有 `sendToClient` 能力，普通 Entity 没有。在 BaseApp 上，只有 Proxy 才有客户端连接。

## 2.6 Space、Cell、AOI、Witness：为什么是核心而不是附属

普通服务端直觉里，很容易把"可见性"理解成一个附加模块。但在 BigWorld 一系里，它是核心问题之一。

原因：

- 世界很大
- 实体很多
- 客户端不可能也不应该收到整个世界

系统必须回答：**当前这个客户端到底该看到谁？**

这就逼出了几层关键结构：

### Space — 世界运行空间

一个逻辑上的"世界"或"场景"。同一个游戏可以有多个 Space（多个副本、多个地图）。

```cpp
// 文件：programming/bigworld/server/cellapp/space.hpp:49
/**
 *  This class is used to represent a space.
 */
class Space : public TimerHandler, public GeometryMapper
{
    SpaceID id() const { return id_; }
    Cell * pCell() const { return pCell_; }
    // ...
};
```

KBEngine 用 `SpaceMemory` 做类似的事：

```cpp
// 文件：kbe/src/server/cellapp/spacememory.h:20
class SpaceMemory
{
    void addEntity(Entity* pEntity);
    void addEntityAndEnterWorld(Entity* pEntity, bool isRestore = false);
    void removeEntity(Entity* pEntity);
    void onEntityAttachWitness(Entity* pEntity);
    // ...
};
```

### Cell — 空间运行的最小承载单元

BigWorld 中，一个 Space 可以被分成多个 Cell（通过 BSP 树动态分割）。每个 Cell 承载一块区域内的 real entity。

```cpp
// 文件：programming/bigworld/server/cellapp/cell.hpp:45
/**
 *  This class is used to represent a cell.
 */
class Cell
{
    class Entities
    {
        bool add( Entity * pEntity );
        bool remove( Entity * pEntity );
        // ...
    };
    // ...
};
```

KBEngine 的简化：没有 BSP 树动态分割，一个 SpaceMemory 对应一块空间。

### AOI（Area of Interest）— 可见范围判定

决定"谁在附近"的机制。不是简单的距离检测，而是一整套空间索引 + 触发器系统。具体算法（十字链表、RangeTrigger）在 Ch14 详解。

### Witness — 把 AOI 事件变成客户端同步

Witness 不是"方便一点的观察者类"，而是**大规模在线世界里客户端可见性控制的核心执行点**。

```cpp
// 文件：programming/bigworld/server/cellapp/witness.hpp:25
/**
 *  This class is a witness to the movements and perceptions of a RealEntity.
 *  It is created when a client is attached to this entity. Its main activity
 *  centres around the management of an Area of Interest list.
 */
class Witness : public Updatable
{
    RealEntity & real() { return real_; }
    // ...
};
```

关键点：**Witness 是当客户端附着到实体时才创建的**。不是每个实体都有 Witness，只有"被客户端控制"的实体才有。

KBEngine 同样：

```cpp
// 文件：kbe/src/server/cellapp/witness.h:57
/**
 *  这个模块用来监视我们感兴趣的entity数据，如：view，属性更新，
 *  调用entity的方法并将其传输给监视者。
 */
class Witness : public PoolObject, public Updatable
{
    // ...
    struct WitnessInfo
    {
        int8 detailLevel;       // 当前所在详情级别
        Entity* entity;         // 所表达的entity
        float range;            // 当前与这个entity的距离
        bool detailLevelLog[3]; // 进入过哪些详情级别（属性广播优化）
        // ...
    };
    // ...
};
```

注意 `detailLevel` — 不是所有可见实体都同步全部属性，远处只同步位置，近处才同步全部。这是 Witness 级别的优化，不是网络层的。

## 2.7 Ghost：分布式空间协作的必要存在

一旦空间可以分布式运行（多个 CellApp），又会碰到一个问题：

> 某个实体真实逻辑在别的 Cell 上，但本地又需要知道它的存在和部分状态，怎么办？

Ghost 不是"复制一份实体玩玩"，而是**在分布式空间运行中，为远端真实实体提供局部可见和协作副本**。

BigWorld 中，Entity 的 real/ghost 由 `RealEntity` 管理：

```cpp
// 文件：programming/bigworld/server/cellapp/real_entity.hpp:46
/**
 *  An object of this type is used by Entity to store additional data
 *  when the entity is "real" (as opposed to ghosted).
 */
class RealEntity
{
    // ...
    class Haunt  // Ghost 驻留在哪个 CellAppChannel
    {
        CellAppChannel * pChannel_;
        GameTime creationTime_;
        // ...
    };
    // ...
};
```

一个 RealEntity 知道自己有哪些 Ghost（通过 Haunt 列表），Ghost 知道自己的 real 在哪（通过 `realAddr`）。

KBEngine 没有 BigWorld 那样完整展开成一套 `RealEntity + Haunt` 命名体系，但 real/ghost 仍然是明确的一等运行态，而不是文档层面的概念借用：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
KBE_ASSERT(isReal() == true && "Entity::changeToGhost(): not is real.\n");
// ...
KBE_ASSERT(isReal() == false && "Entity::changeToReal(): not is ghost.\n");
```

也就是说，KBEngine 同样把 ghost 视为 Cell 侧分布式空间协作的一部分，只是实现组织比 BigWorld 更收敛。

## 2.8 Mailbox vs EntityCall：远端实体的"邮箱地址"

### BigWorld Mailbox

Mailbox 是远端实体的"邮箱地址"。持有 Mailbox 就能向那个实体发消息，不需要知道它在哪台机器上。

```cpp
// 文件：programming/bigworld/server/cellapp/mailbox.hpp:30
/**
 *  This class implements a mailbox that can send to a server object.
 *  This object may be on a cell or may be a base.
 */
class ServerEntityMailBox: public PyEntityMailBox
{
    virtual const Mercury::Address address() const { return addr_; }
    virtual EntityID id() const { return id_; }
    virtual EntityMailBoxRef::Component component() const = 0;
    // ...
};

// 两种特化
class CellEntityMailBox: public ServerEntityMailBox { /* ... */ };
class BaseEntityMailBox: public ServerEntityMailBox { /* ... */ };
```

`addr_` + `id_` + `component` 三元组唯一确定一个远端实体。

### KBEngine EntityCall

KBEngine 用 `EntityCall` 做同样的事，但命名不同：

```cpp
// 文件：kbe/src/lib/entitydef/entity_call.h:15
class EntityCall : public EntityCallAbstract
{
    EntityCall(ScriptDefModule* pScriptModule, const Network::Address* pAddr,
        COMPONENT_ID componentID, ENTITY_ID eid, ENTITYCALL_TYPE type);
    // ...
    virtual void newCall(Network::Bundle& bundle);
    virtual RemoteEntityMethod* createRemoteMethod(MethodDescription* pMethodDescription);
    // ...
};
```

对应关系：

| BigWorld | KBEngine | 含义 |
|----------|----------|------|
| Mailbox | EntityCall | 远端实体的引用 |
| CellEntityMailBox | CellEntityCall | 指向 Cell 侧实体 |
| BaseEntityMailBox | BaseEntityCall | 指向 Base 侧实体 |
| ClientEntityMailBox | ClientEntityCall | 指向客户端实体 |

这里只建立概念对应。到了 RPC 和 AOI 章节还要进一步区分：客户端这侧的方法调用会受到 Witness / 可见性约束，不能简单等同于普通的 Base/Cell 远程调用。

核心差异：BigWorld 有 TwoWay RPC（带返回值的 Mailbox 调用），KBEngine 只有单向 EntityCall + CallbackMgr。这在 Ch11 详述。

## 2.9 统一运行模型

把前面的对象放回一张统一模型图：

```
客户端
  │
  │ TCP/KCP
  ▼
LoginApp ──── 认证 ────▶ DBMgr（查账号）
  │
  │ 分配 BaseApp
  ▼
BaseApp
  ├── Proxy（客户端会话锚点）
  ├── Base Entity（长期逻辑：背包/任务/社交）
  │     │
  │     │ EntityCall（创建 Cell Entity）
  │     ▼
  └──▶ CellApp
        ├── Cell Entity（实时空间：位置/移动/战斗）
        ├── Space（世界空间）
        ├── Witness（AOI → 客户端同步）
        ├── Ghost（跨 Cell 可见性副本）
        │
        │ 必要时向 Base 收束空间态
        ▼
      Base / DBMgr（持久化入口）
```

八步主线：

1. 客户端通过 `LoginApp` 接入
2. 玩家长期逻辑实体停在 `BaseApp` 的 Base Entity
3. 进入世界后在 `CellApp` 创建 Cell Entity
4. 若该玩家受客户端控制，则通过 `Proxy` 绑定连接
5. 所在 `Space` 决定它处于哪个世界上下文
6. `Witness` 负责把周围实体的可见性变化同步给客户端
7. 若实体跨 Cell 运行，`Ghost` 帮助维持分布式空间协作
8. `DBMgr` 负责持久化、恢复和在线元信息

这些术语并不是散落的定义，而是同一个系统模型里的不同支点。

## 2.10 为什么这套模型今天看起来仍然成立

即使技术栈变了，BigWorld 一系模型仍然有生命力：

| 问题 | 2002 年成立 | 2026 年仍然成立 |
|------|-----------|---------------|
| 大规模在线状态需要分层 | ✓ | ✓ |
| 实时空间和长期业务不同 | ✓ | ✓ |
| 客户端可见性不能靠全量广播 | ✓ | ✓ |
| 持久化不应直接充当实时状态主源 | ✓ | ✓ |
| 单进程无法承载整个世界 | ✓ | ✓ |

KBEngine 继续沿用这套模型，不是历史包袱，而是这些问题本身还没消失。

对比今天的微服务架构：

- 微服务解决的是"无状态请求的水平扩展"
- BigWorld/KBEngine 解决的是"有状态实体的分布式运行"
- 两者面对的问题域不同，不能用同一套方案

## 2.11 关键源码入口

### BigWorld

| 概念 | 文件路径 |
|------|---------|
| Base 实体 | `server/baseapp/base.hpp` |
| Proxy | `server/baseapp/proxy.hpp` |
| Cell Entity | `server/cellapp/entity.hpp` |
| Real/Ghost | `server/cellapp/real_entity.hpp` |
| Space | `server/cellapp/space.hpp` |
| Cell | `server/cellapp/cell.hpp` |
| Witness | `server/cellapp/witness.hpp` |
| Mailbox | `server/cellapp/mailbox.hpp` |
| LoginApp | `server/loginapp/loginapp.hpp` |
| DBApp | `server/dbapp/dbapp.hpp` |

### KBEngine

| 概念 | 文件路径 |
|------|---------|
| Base Entity | `kbe/src/server/baseapp/entity.h` |
| Proxy | `kbe/src/server/baseapp/proxy.h` |
| Cell Entity | `kbe/src/server/cellapp/entity.h` |
| SpaceMemory | `kbe/src/server/cellapp/spacememory.h` |
| Witness | `kbe/src/server/cellapp/witness.h` |
| EntityCall | `kbe/src/lib/entitydef/entity_call.h` |
| LoginApp | `kbe/src/server/loginapp/loginapp.h` |
| DBMgr | `kbe/src/server/dbmgr/dbmgr.h` |

## 2.12 源码走读路径

建议按以下顺序在 IDE 中打开文件：

### 路径一：理解 Entity 的多侧表示

1. BigWorld: `server/cellapp/entity.hpp` — 看类声明，关注 `isReal()` / `initReal()` / `initGhost()`
2. BigWorld: `server/cellapp/real_entity.hpp` — 看 RealEntity 的 Haunt 列表
3. KBEngine: `kbe/src/server/cellapp/entity.h` — 对比 `isReal()` 和 real/ghost 判定
4. KBEngine: `kbe/src/server/baseapp/entity.h` — 看 Base 侧 Entity 的 DB 相关方法

### 路径二：理解 Proxy 的双重身份

1. BigWorld: `server/baseapp/base.hpp` — 先看 Base 的基础能力
2. BigWorld: `server/baseapp/proxy.hpp` — 看 Proxy 继承 Base 后加了什么（客户端连接管理）
3. KBEngine: `kbe/src/server/baseapp/proxy.h` — 对比 `sendToClient` 方法族

### 路径三：理解可见性控制链

1. BigWorld: `server/cellapp/space.hpp` — Space 持有 Cell
2. BigWorld: `server/cellapp/cell.hpp` — Cell 持有 Entities
3. BigWorld: `server/cellapp/witness.hpp` — Witness 绑定到 RealEntity
4. KBEngine: `kbe/src/server/cellapp/witness.h` — 看 WitnessInfo 的 detailLevel

## 2.13 小结

这一章最重要的收获：

- **BigWorld 模型首先是问题驱动的**，不是术语驱动的
- `Login / Base / Cell / DB` 是四类不同问题的自然分工
- 实体**必须允许多侧表示**，才能支撑长期逻辑和实时空间分离
- **Proxy 是实体模型里"在线会话"语义的承载点**，不是普通 Entity
- **Witness 不是附加功能**，而是大规模在线世界客户端可见性控制的核心执行点
- **Ghost 是分布式空间协作的必要机制**，不是可选的缓存策略
- **Mailbox/EntityCall 是远端实体的引用句柄**，持有它就能通信，无需知道物理位置

把这层心智模型建立起来之后，再去读 KBEngine 的整体架构和源码实现，很多设计就不会显得突兀。
