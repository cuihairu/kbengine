# 17. Ghost 系统

> 这一章回答：为什么一个实体在多个 CellApp 上有副本？real 和 ghost 怎么分工？ghost 上的方法调用怎么转接到 real？

## 17.1 本章核心问题

- Cell 世界为什么不是单副本的？real 和 ghost 怎么区分？
- GhostManager 的消息缓冲机制怎么工作？
- RealEntityMethod 怎么把 ghost 上的方法调用转接到 real？
- controlledBy：谁在控制这个 cell 实体？
- 跨 Cell 边界的控制权怎么分布？

## 17.2 为什么需要 Ghost：跨 Cell 边界的可见性

一个 Space 可能被切成多个 Cell（Ch15），分布在不同 CellApp 上。当实体 A 在 Cell 1，但它的 AOI 范围延伸到 Cell 2 时，Cell 2 上的实体需要知道 A 的存在——否则 A 进入 Cell 2 上某个玩家的视野时，无法触发 AOI 事件。

**Ghost 就是解决方案**：在实体 A 的 AOI 覆盖到的其他 Cell 上创建 A 的只读副本。

```
Cell 1（CellApp A）              Cell 2（CellApp B）
  ┌───────────────┐              ┌───────────────┐
  │               │              │               │
  │  Real A ──────┼── AOI ──────┼── Ghost A     │
  │  (权威)       │              │  (只读副本)    │
  │               │              │               │
  │  Player B     │              │  NPC C        │
  │               │              │               │
  └───────────────┘              └───────────────┘

Real A 在 Cell 1 上拥有完整状态和写权限
Ghost A 在 Cell 2 上拥有部分只读状态

当 NPC C 的位置进入 Ghost A 的 AOI 范围时：
  → Ghost A 触发 onEnterView → 通知 Real A → 同步给 Player B 的客户端
```

**Ghost 不是 Cache**：Ghost 不只是为了加速查询。Ghost 是 AOI 系统的必要组件——没有 Ghost，跨 Cell 边界的 AOI 事件无法触发。

### 重要纠偏：KBEngine 里的 Ghost 不只用于“多 Cell AOI”

上面的解释更接近 BigWorld 的多 Cell 空间分割语境。  
在 KBEngine 中，即使一个 Space 通常只有一个 Cell，Ghost 仍有关键职责：实体跨 CellApp 迁移（teleport）期间的消息保序与路由中转。

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
// 如果期间有base的消息发送过来， entity的ghost机制能够转到real上去...
```

```cpp
// 文件：kbe/src/server/cellapp/ghost_manager.h
// ...需要向ghost_route_临时设置一个路由地址， 路由在最后一次收包超过一定时间擦除。
```

可以把它理解成：KBEngine 的 Ghost 既是“分布式可见性副本机制”，也是“迁移窗口的可靠转发机制”。

## 17.3 real / ghost 的区分

### KBEngine

```cpp
// 文件：kbe/src/server/cellapp/entity.inl（简化）
// 判断是否为 real 实体
INLINE bool Entity::isReal(void) const
{
    return realCell_ == 0;    // realCell_ 为 0 表示自己是 real
}

// 判断是否有 ghost
INLINE bool Entity::hasGhost(void) const
{
    return ghostCell_ > 0;    // ghostCell_ > 0 表示存在 ghost
}
```

```cpp
// 文件：kbe/src/server/cellapp/entity.h（简化）
class Entity : public script::ScriptObject
{
protected:
    COMPONENT_ID realCell_;    // real 实体所在的 CellApp（0 = 本机）
    COMPONENT_ID ghostCell_;   // ghost 实体所在的 CellApp（0 = 无 ghost）
};
```

**关键**：`realCell_` 是 dual-use 的——在 real 实体上为 0（表示"我就是 real"），在 ghost 实体上为 real 所在的 CellApp ID。

### BigWorld

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity.hpp（简化）
class Entity : public PyObjectPlus
{
private:
    CellAppChannel* pRealChannel_;   // Real 实体通信通道（Ghost 使用）
    RealEntity* pReal_;              // Real 实体指针（非 NULL = 自己是 real）

public:
    bool isReal() const { return pReal_ != NULL; }
    bool isRealToScript() const { return pReal_ != NULL; }
    RealEntity* pReal() const { return pReal_; }
};
```

BigWorld 的 `pReal_` 指针更直观——非 NULL 表示自己是 real，NULL 表示自己是 ghost。Real 实体通过 `RealEntity` 类管理所有 ghost 的状态。

### isReal() 检查遍布代码

```cpp
// KBEngine 中的典型用法
void Entity::onDefDataChanged(...)
{
    // 只有 real entity 才处理属性变更的广播
    if (!isReal() || initing())
        return;
    // ...
}

// BigWorld 中的典型用法
void Entity::onOwnedPropertyChanged(...)
{
    // 只有 real 才能发起属性变更
    if (!this->isReal())
        return;
    // ...
}
```

**设计原则**：ghost 实体不主动修改状态——所有写操作必须经过 real。这保证了单一写权限。

## 17.4 RealEntity：BigWorld 的 Real 端管理

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp（简化）
class RealEntity
{
public:
    // Haunt：一个 Ghost 的位置记录
    class Haunt
    {
    public:
        Haunt(CellAppChannel* pChannel, GameTime creationTime);

        CellAppChannel& channel() { return *pChannel_; }
        Mercury::Bundle& bundle() { return pChannel_->bundle(); }
        const Mercury::Address& addr() const { return pChannel_->addr(); }

    private:
        CellAppChannel* pChannel_;     // 到 Ghost 所在 CellApp 的通道
        GameTime creationTime_;        // Ghost 创建时间
    };

    typedef BW::vector<Haunt> Haunts;

    // Ghost（Haunt）管理
    void addHaunt(CellAppChannel& channel);
    Haunts::iterator delHaunt(Haunts::iterator iter);
    void deleteGhosts();

    // 控制权管理
    const EntityMailBoxRef& controlledByRef() const { return controlledBy_; }
    bool controlledBySelf() const { return entity_.id() == controlledBy_.id; }
    bool controlledByOther() const;
    void delControlledBy(EntityID deadID);

    // 观察者
    Witness* pWitness() const { return pWitness_; }

private:
    Entity& entity_;
    Witness* pWitness_;
    Haunts haunts_;                       // 所有 Ghost 位置列表
    EntityMailBoxRef controlledBy_;       // 控制者引用
    Mercury::UDPChannel* pChannel_;       // 通信通道
};
```

**Haunt = Ghost 的通信句柄**。RealEntity 维护所有 Ghost 的 Haunt 列表，通过 Haunt 向各个 Ghost 推送属性更新。

### 向所有 Ghost 广播属性更新

```
Real Entity 属性变更（如 position）
  │
  ├── 遍历 haunts_ 列表
  │     for (Haunt& haunt : haunts_)
  │       haunt.bundle() << msgID << entityID << propertyData
  │
  └── 每个 Haunt 的 Bundle 通过对应 CellAppChannel 发送
        → CellApp 收到 onUpdateGhostPropertys
        → Ghost 实体更新本地属性值
```

## 17.5 GhostManager：KBEngine 的消息缓冲机制

### 为什么需要缓冲

Ghost 消息不是直接发送的，而是通过 GhostManager 缓冲后批量发送。原因：

1. **一个 tick 内同一实体可能产生多条 ghost 消息**——多次属性变更只需最后一次
2. **同一 CellApp 上可能有多个实体向同一个目标 CellApp 发送 ghost 消息**——合并到同一个 Bundle
3. **按目标 CellApp 聚合发送，减少包碎片**

```cpp
// 文件：kbe/src/server/cellapp/ghost_manager.h（简化）
class GhostManager : public TimerHandler
{
public:
    // 创建发送 Bundle（复用已有 Bundle）
    Network::Bundle* createSendBundle(COMPONENT_ID componentID);

    // 投递消息
    void pushMessage(COMPONENT_ID componentID, Network::Bundle* pBundle);

    // 定时同步消息与路由
    void handleTimeout(TimerHandle, void* arg);

private:
    // real 实体列表
    std::map<ENTITY_ID, Entity*> realEntities_;

    // ghost 路由信息
    std::map<ENTITY_ID, ROUTE_INFO> ghost_route_;

    // 按 CellApp 分组的消息缓冲
    std::map<COMPONENT_ID, std::vector<Network::Bundle*>> messages_;
};
```

### 消息缓冲流程

```
Real Entity 属性变更
  │
  ├── GhostManager::createSendBundle(targetCellAppID)
  │     → 查找该 CellApp 的消息缓冲
  │     → 如果有未满的 Bundle，复用
  │     → 否则创建新 Bundle
  │
  ├── 写入属性更新消息到 Bundle
  │
  ├── GhostManager::pushMessage(targetCellAppID, bundle)
  │     → 放入 messages_[targetCellAppID] 缓冲
  │
  └── 定时器触发 GhostManager::handleTimeout()
        → syncMessages() 发送 messages_
        → syncGhosts() 维护 realEntities_
        → checkRoute() 清理过期 ghost_route_
```

### BigWorld 的对应机制

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/buffered_ghost_messages.hpp（简化）
class BufferedGhostMessages
{
public:
    // 按顺序播放缓冲消息
    void playSubsequenceFor(EntityID id, const Mercury::Address& srcAddr);
    void playNewLifespanFor(EntityID id);

    // 添加消息到缓冲
    void add(EntityID entityID, const Mercury::Address& srcAddr,
        BufferedGhostMessage* pMessage);

    // 消息序列边界
    static bool isSubsequenceStart(Mercury::MessageID id)
    { return id == CellAppInterface::ghostSetReal.id(); }

    static bool isSubsequenceEnd(Mercury::MessageID id)
    { return id == CellAppInterface::delGhost.id() ||
               id == CellAppInterface::ghostSetNextReal.id(); }
};
```

BigWorld 的缓冲更复杂——支持**消息序列**（subsequence）概念。一组相关的 ghost 消息（从 `ghostSetReal` 到 `delGhost`）被视为一个序列，确保顺序处理。

## 17.6 RealEntityMethod：ghost 上的方法调用转接

### 问题

Ghost 实体只有只读副本。当 Ghost 上的脚本调用一个方法时，这个方法可能需要修改状态——但 ghost 没有写权限。

### 解决方案：RealEntityMethod

```cpp
// 文件：kbe/src/server/cellapp/real_entity_method.h（简化）
class RealEntityMethod : public script::ScriptObject
{
public:
    RealEntityMethod(MethodDescription* pMethodDescription,
        ENTITY_ID ghostEntityID, COMPONENT_ID realCell);

    // tp_call：将方法调用转发到 real entity
    PyObject* callmethod(PyObject* args, PyObject* kwds);

private:
    MethodDescription* pMethodDescription_;
    ENTITY_ID ghostEntityID_;     // ghost 实体 ID
    COMPONENT_ID realCell_;       // real 所在的 CellApp
};
```

### 转接机制

```
Ghost 上的 Python 调用: entity.onDamage(100)
  │
  ├── Entity::onScriptGetAttribute("onDamage")
  │     → 检查 isReal()
  │     → 如果是 ghost 且命中了 cell method：创建 RealEntityMethod(NULL, methodDesc, this)
  │
  ├── RealEntityMethod::tp_call(args=(100,))
  │     │
  │     ├── 创建 Bundle
  │     │     GhostManager::createSendBundle(realCell_)
  │     │
  │     ├── 先往临时流写入组件属性 UID（没有组件则写 0）
  │     ├── methodDescription_->addToStream(mstream, args)
  │     ├── Bundle 写入 CellappInterface::onRemoteRealMethodCall
  │     │     << ghostEntityID_
  │     │     << [组件 UID + 方法参数流]
  │     │
  │     └── 发送到 real 所在的 CellApp
  │           GhostManager::pushMessage(realCell_, bundle)
  │
  └── Real Entity 收到消息
        Entity::onRemoteRealMethodCall(stream)
          → 反序列化方法 ID 和参数
          → 在 real entity 上执行实际逻辑
          → 结果通过属性同步回 ghost
```

### 与 RemoteEntityMethod 的区别

| 维度 | RemoteEntityMethod（Ch11） | RealEntityMethod |
|------|---------------------------|------------------|
| 用途 | 跨进程 RPC（EntityCall） | Ghost→Real 方法转接 |
| 目标 | 任意进程上的实体 | 本实体的 real 副本 |
| 路由 | 按 ENTITYCALL_TYPE | 固定发到 realCell_ |
| 消息 | CellappInterface::onEntityCall | CellappInterface::onRemoteRealMethodCall |
| 返回值 | 无（纯单向） | 无（结果通过属性同步回传） |

**本质相同**：两者都是把 Python 方法调用变成网络消息发送。区别在于路由方式——RemoteEntityMethod 按 EntityCall 地址路由，RealEntityMethod 固定路由到 real 所在的 CellApp。

## 17.7 controlledBy：谁在控制这个 Cell 实体

### 为什么需要控制权

MMO 中一个实体可能被不同的"控制者"驱动：

1. **玩家客户端**：通过键盘/手柄输入驱动移动
2. **AI 脚本**：NPC 的行为由 AI 逻辑驱动
3. **其他实体**：一个实体被另一个实体控制（如骑乘、附身）
4. **无人控制**：纯静态物体

### KBEngine controlledBy

```cpp
// 文件：kbe/src/server/cellapp/entity.h（简化）
class Entity : public script::ScriptObject
{
protected:
    // 当前实体的坐标朝向受谁控制
    EntityCall* controlledBy_;

public:
    EntityCall* controlledBy() const { return controlledBy_; }
    void controlledBy(EntityCall* baseEntityCall);

    bool isControlledNotSelfClient() const
    {
        return controlledBy_ == NULL || controlledBy_->id() != id();
    }
};
```

### BigWorld controlledBy

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp（简化）
class RealEntity
{
public:
    const EntityMailBoxRef& controlledByRef() const { return controlledBy_; }

    bool controlledBySelf() const
    {
        return entity_.id() == controlledBy_.id;
    }

    bool controlledByOther() const
    {
        return !this->controlledBySelf() && (controlledBy_.id != 0);
    }

    void delControlledBy(EntityID deadID);

private:
    EntityMailBoxRef controlledBy_;   // 控制者的 MailBox 引用
};
```

### 控制权的使用场景

```
场景 1：玩家登录
  LoginApp → BaseApp → CellApp
  │
  ├── 创建 Avatar 实体
  ├── onGetWitness() 最后设置 controlledBy = baseEntityCall()
  │     → cell 侧把"谁拥有控制权"锚定到对应 base mailbox
  └── 客户端断线
        → controlledBy = NULL
        → 实体变为无人控制状态
        → 超时后写库或销毁

场景 2：NPC 被 AI 控制
  │
  ├── controlledBy = NULL 或其他 base mailbox
  │     → 由服务器端逻辑决定如何驱动
  └── 如果 NPC 被玩家"驯服"
        → controlledBy = 玩家的 EntityCall
        → 玩家的输入驱动 NPC 行为

场景 3：骑乘
  │
  ├── Mount 实体（坐骑）
  │     controlledBy = Rider 对应的 base mailbox
  │     → Rider 的移动输入驱动 Mount 的位置
  └── Rider 下坐骑
        → Mount.controlledBy = NULL
        → Rider 恢复自身控制
```

### controlledBy 与 Witness 的关系

```
controlledBy 和 Witness 是两个正交的概念：

controlledBy：谁驱动这个实体的行为（移动、动作）
Witness：这个实体的 AOI 数据同步给谁的客户端

一个实体可以：
  - 有 controlledBy + 有 Witness（正常玩家：客户端既控制又观察）
  - 无 controlledBy + 有 Witness（旁观者：不控制但观察）
  - 有 controlledBy + 无 Witness（被骑乘的坐骑：被控制但不直接观察）
  - 无 controlledBy + 无 Witness（普通 NPC）
```

### 重连与控制权恢复

```
客户端断线重连
  │
  ├── BaseApp 检测到旧连接断开
  │     → 保留 Avatar 的 Base 和 Cell 实体
  │     → controlledBy 设为 NULL（暂停控制）
  │
  ├── 客户端重新连接
  │     → 验证身份
  │     → 恢复 Proxy
  │
  ├── 恢复 controlledBy
  │     → Avatar.cell.controlledBy = newProxy.entityCall
  │     → 恢复 Witness
  │
  └── 客户端收到完整世界状态
        → 继续游戏

如果旧连接没断开就重连（挤号）：
  → BaseApp 踢掉旧连接
  → 旧连接的 controlledBy 清除
  → 新连接接管 controlledBy
```

## 17.8 跨 Cell 边界的控制权分布

当一个实体跨越 Cell 边界时，它的 Ghost 在新 Cell 上创建，Real 可能需要迁移。

### Ghost 创建流程

```
Real Entity A 在 Cell 1
  │
  ├── A 的 AOI 范围延伸到 Cell 2 的区域
  │
  ├── Cell 1 检测到需要创建 Ghost
  │     RealEntity::addHaunt(cell2Channel)
  │
  ├── 发送 createGhost 消息到 Cell 2
  │     包含：entityID + entityType + 位置 + Ghost 需要的属性
  │
  └── Cell 2 创建 Ghost A
        → 加入 CoordinateSystem（AOI 链表）
        → 设置 realCell_ = Cell 1 的 ID
        → 标记为 Ghost（isReal() == false）
```

### Ghost 销毁流程

```
Real Entity A 移动，AOI 不再覆盖 Cell 2
  │
  ├── Cell 1 检测到 Ghost 不再需要
  │     RealEntity::delHaunt(hauntIter)
  │
  ├── 发送 delGhost 消息到 Cell 2
  │
  └── Cell 2 销毁 Ghost A
        → 从 CoordinateSystem 移除
        → 清理 AOI 关系
        → 删除 Ghost 实体
```

### EntityGhostMaintainer（BigWorld）

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_ghost_maintainer.hpp（简化）
class EntityGhostMaintainer : public CellInfoVisitor
{
public:
    EntityGhostMaintainer(OffloadChecker& offloadChecker, EntityPtr pEntity);
    void check();

    virtual void visit(CellInfo& cellInfo);

private:
    void checkEntityForOffload();
    bool markHaunts();                  // 标记现有的 Haunt
    void createOrUnmarkRequiredHaunts(); // 创建新的或取消标记
    void deleteMarkedHaunts();           // 删除不再需要的 Haunt

    OffloadChecker& offloadChecker_;
    EntityPtr pEntity_;
};
```

`EntityGhostMaintainer::check()` 在每帧（或定期）执行，确保 Real Entity 的 Ghost 集合与 AOI 覆盖范围一致：

```
check() 流程：
  │
  ├── 1. markHaunts()：标记所有现有 Haunt 为"待删除"
  │
  ├── 2. 遍历 AOI 覆盖到的 Cell
  │     visit(cellInfo)
  │       → 如果该 Cell 已有 Haunt → 取消"待删除"标记
  │       → 如果该 Cell 没有 Haunt → 创建新 Haunt
  │
  └── 3. deleteMarkedHaunts()：删除仍标记为"待删除"的 Haunt
```

## 17.9 Ghost 属性同步

### KBEngine onUpdateGhostPropertys

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp（简化）
void Entity::onUpdateGhostPropertys(KBEngine::MemoryStream& s)
{
    ENTITY_PROPERTY_UID utype;
    s >> utype;

    // 查找属性描述
    PropertyDescription* pPropertyDescription =
        pScriptModule()->findCellPropertyDescription(utype);

    if (pPropertyDescription == NULL)
    {
        ERROR_MSG("onUpdateGhostPropertys: not found propertyID");
        return;
    }

    // 从流中读取属性值并设置
    PyObject* pyVal = pPropertyDescription->createFromStream(&s);
    PyObject_SetAttrString(static_cast<PyObject*>(this),
        pPropertyDescription->getName(), pyVal);
    Py_DECREF(pyVal);
}
```

### BigWorld DATA_GHOSTED 标记

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_description.hpp（简化）
enum EntityDataFlags
{
    DATA_GHOSTED = 0x01,        // 同步到 Ghost 实体
    DATA_OTHER_CLIENT = 0x02,   // 发送到其他客户端
    DATA_OWN_CLIENT = 0x04,     // 发送到自身客户端
    DATA_BASE = 0x08,           // 发送到 Base
    DATA_PERSISTENT = 0x20,     // 持久化到数据库
};
```

BigWorld 用 `DATA_GHOSTED` 标记需要同步到 Ghost 的属性。不是所有属性都同步——只同步 Ghost 需要的（AOI 相关的、客户端可见的）。

### 同步的三个消息类型

```cpp
// 文件：kbe/src/server/cellapp/cellapp_interface.h（简化）
// real → ghost：属性更新
CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateGhostPropertys, NETWORK_VARIABLE_MESSAGE)

// ghost → real：方法调用转接
CELLAPP_MESSAGE_DECLARE_STREAM(onRemoteRealMethodCall, NETWORK_VARIABLE_MESSAGE)

// real → ghost：Volatile 数据更新（位置、朝向）
CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateGhostVolatileData, NETWORK_VARIABLE_MESSAGE)
```

## 17.10 Real ↔ Ghost 状态转换

### KBEngine changeToGhost / changeToReal

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp（简化）

// Real 变为 Ghost（实体迁移到另一个 CellApp）
void Entity::changeToGhost(COMPONENT_ID realCell, KBEngine::MemoryStream& s)
{
    KBE_ASSERT(isReal() == true);

    realCell_ = realCell;    // 记录 real 所在的 CellApp
    ghostCell_ = 0;

    // 添加路由到 GhostManager
    GhostManager* gm = Cellapp::getSingleton().pGhostManager();
    gm->addRoute(id(), realCell_);

    // 序列化完整状态到流
    addToStream(s);

    // 清理 Real 特有的资源
    if (pControllers_)
    {
        pControllers_->clear();
        SAFE_RELEASE(pControllers_);
    }
    if (pWitness())
    {
        pWitness()->clear(this);
        Witness::reclaimPoolObject(pWitness_);
        pWitness_ = NULL;
    }
}

// Ghost 变为 Real（实体迁移到本 CellApp）
void Entity::changeToReal(COMPONENT_ID ghostCell, KBEngine::MemoryStream& s)
{
    KBE_ASSERT(isReal() == false);

    ghostCell_ = ghostCell;
    realCell_ = 0;    // 设为 0 表示自己是 real

    // 从流中恢复完整状态
    createFromStream(s);
}
```

**状态转换发生在实体迁移（Offload/Teleport）时**：

```
实体从 CellApp A 迁移到 CellApp B
  │
  ├── CellApp A：Entity::changeToGhost(B_cellAppID, stream)
  │     序列化状态到 stream
  │     清理 Controller、Witness
  │     变为 Ghost（只是路由中转）
  │
  ├── 发送 stream 到 CellApp B
  │
  └── CellApp B：Entity::changeToReal(A_cellAppID, stream)
        从 stream 恢复状态
        变为 Real
        重建 Controller、Witness
```

## 17.11 两套项目的 Ghost 系统对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| Real 判断 | `realCell_ == 0` | `pReal_ != NULL` |
| Ghost 判断 | `realCell_ != 0` | `pReal_ == NULL` |
| Real 端管理 | 无专门类（Entity 直接管理） | RealEntity 类 + Haunts 列表 |
| Ghost 端管理 | GhostManager（消息缓冲+路由） | BufferedGhostMessages（序列缓冲） |
| 方法转接 | RealEntityMethod | 类似机制（Ghost 上的调用转发） |
| 属性同步 | onUpdateGhostPropertys | DATA_GHOSTED 标记 + 属性推送 |
| Ghost 创建 | 无独立类（Entity 状态切换） | EntityGhostMaintainer 定期检查 |
| 控制权 | EntityCall* controlledBy_ | EntityMailBoxRef controlledBy_ |
| 状态转换 | changeToGhost / changeToReal | Offload 机制 |
| Haunt 管理 | 无 Haunt 列表（但有迁移期 ghost 路由） | RealEntity::addHaunt / delHaunt |
| 消息缓冲 | per-CellApp Bundle 缓冲 | BufferedGhostMessages 序列缓冲 |

**核心差异**：

1. **BigWorld 的 Haunt 列表**：RealEntity 维护所有 Ghost 的通道列表，可以直接向每个 Ghost 推送数据。KBEngine 没有 Haunt 概念，但通过 `GhostManager::ghost_route_` 处理迁移窗口的临时路由。

2. **BigWorld 的 EntityGhostMaintainer**：定期检查 Ghost 集合是否与 AOI 覆盖一致。KBEngine 没有这层 AOI 一致性巡检，主要依赖迁移流程中的 `changeToGhost / changeToReal` 与路由超时清理。

3. **消息缓冲策略**：KBEngine 按 CellApp 分组缓冲 Bundle，BigWorld 按消息序列缓冲并保证顺序

## 17.12 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| Entity（isReal/hasGhost） | `kbe/src/server/cellapp/entity.h` |
| Entity 内联实现 | `kbe/src/server/cellapp/entity.inl` |
| GhostManager | `kbe/src/server/cellapp/ghost_manager.h` |
| GhostManager 实现 | `kbe/src/server/cellapp/ghost_manager.cpp` |
| RealEntityMethod | `kbe/src/server/cellapp/real_entity_method.h` |
| onUpdateGhostPropertys | `kbe/src/server/cellapp/entity.cpp` |
| changeToGhost/changeToReal | `kbe/src/server/cellapp/entity.cpp` |
| Ghost 消息接口 | `kbe/src/server/cellapp/cellapp_interface.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| RealEntity + Haunt | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp` |
| Entity（isReal） | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity.hpp` |
| BufferedGhostMessages | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/buffered_ghost_messages.hpp` |
| EntityGhostMaintainer | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_ghost_maintainer.hpp` |
| DATA_GHOSTED | `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_description.hpp` |

## 17.13 源码走读路径

### 路径一：理解 real / ghost 的区分

1. `kbe/src/server/cellapp/entity.inl` — `isReal()` / `hasGhost()` 判断
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity.hpp` — `pReal_` 指针判断
3. `kbe/src/server/cellapp/entity.cpp` — `onDefDataChanged()` 中的 `isReal()` 守卫

### 路径二：跟踪 Ghost 方法的转接

1. `kbe/src/server/cellapp/real_entity_method.h` — RealEntityMethod 类声明
2. `kbe/src/server/cellapp/entity.cpp` — `onRemoteRealMethodCall()` 接收端处理
3. 对比 Ch11 的 RemoteEntityMethod 路由逻辑

### 路径三：理解 Ghost 的创建和维护

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp` — `addHaunt()` / `delHaunt()`
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_ghost_maintainer.hpp` — `check()` 流程
3. `kbe/src/server/cellapp/ghost_manager.h` — 消息缓冲和路由

## 17.14 小结

- **Ghost 是跨 Cell 边界 AOI 的必要组件**：没有 Ghost，跨 Cell 的实体进出视野事件无法触发
- **对 KBEngine 来说，Ghost 还承担迁移窗口的路由中转职责**：实体跨 CellApp 传送时，消息可经 ghost 机制转发到 real
- **real 拥有完整状态和写权限，ghost 只有部分只读副本**：所有写操作必须经过 real
- **isReal() 检查遍布代码**：属性变更、方法调用、控制器操作都先检查 isReal()
- **GhostManager 缓冲 ghost 消息并批量发送**：按 CellApp 分组，复用 Bundle
- **RealEntityMethod 把 ghost 上的方法调用转接到 real**：序列化参数 → 发送到 realCell_ → real 执行
- **controlledBy 管理实体的控制权**：玩家/AI/其他实体可以驱动实体的行为
- **controlledBy 和 Witness 是正交概念**：控制权是"谁驱动"，Witness 是"谁观察"
- **BigWorld 用 Haunt 列表管理所有 Ghost**：RealEntity 维护 Ghost 的通道和生命周期
- **BigWorld 的 EntityGhostMaintainer 定期检查 Ghost 集合一致性**：标记-清除模式
- **状态转换（changeToGhost/changeToReal）发生在实体迁移时**：序列化完整状态，清理/重建资源
