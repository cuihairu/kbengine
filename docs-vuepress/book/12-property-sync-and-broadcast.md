# 12. 属性同步与数据包广播

> 面试高频章节。回答一个核心问题：一个实体属性变更了，怎么高效同步给所有能看到它的客户端？

## 12.1 本章核心问题

- 为什么不是"改了就发"，而是"tick 内收集，tick 末批量发"？
- 一条属性更新的完整链路是怎样的？
- Witness 怎么收集视野内所有实体的变更并广播？
- alias 机制怎么让同步包变小？
- detailLevel（LOD）怎么实现"远处少同步，近处多同步"？
- Volatile 属性是什么？为什么需要独立的更新频率控制？
- 1000 人同屏时，带宽瓶颈在哪？

## 12.2 心智模型：不是"改了就发"，而是"tick 内收集，tick 末批量发"

这是理解属性同步的关键心智模型：

```
一个 tick（100ms）内的时序：

───────────────────────────────────────────────────
│ 脚本逻辑执行                                    │ tick 末
│                                                 │
│ entity.health = 80                              │ Witness::update()
│ entity.health = 75    ← 只记最终值 75           │   ├── 遍历所有 viewEntities
│ entity.position = (10, 0, 20)                   │   ├── 收集每个实体的脏属性
│ entity.mana = 50                                │   ├── 构造 Bundle
│                                                 │   └── 批量发送给客户端
───────────────────────────────────────────────────
```

**为什么不改了就发**：

1. **一个 tick 内属性可能变化多次**：战斗中血量可能在一帧内扣减 3 次，客户端只需要最终值
2. **Bundle 合并更高效**：多条属性变更合并到一个 Bundle 里，减少网络包数量
3. **带宽可预测**：每个 tick 的同步量有上限（N 个可见实体 × M 个脏属性），不会突发

## 12.3 一条属性更新的完整链路

### KBEngine

```
Python: entity.health = 75
  │
  ├── Entity::_tp_setattro("health", 75)
  │     → Entity::onScriptSetAttribute("health", 75)
  │
  ├── PropertyDescription::isSameType() 类型检查
  │     → 设置 Python 属性值
  │
  ├── Entity::onDefDataChanged(propertyDescription, pyData)
  │     │
  │     ├── 检查 isReal() && !initing()     ← 只有 real entity 处理
  │     │
  │     ├── if Persistent → setDirty()       ← 标记需要写库
  │     │
  │     ├── 序列化属性值到 MemoryStream
  │     │     propertyDescription->getDataType()->addToStream(mstream, pyData)
  │     │
  │     ├── if CELL_PUBLIC && hasGhost
  │     │     → 发送给所有 Ghost：CellappInterface::onUpdateGhostPropertys
  │     │     Ghost 收到后更新本地副本
  │     │
  │     ├── if OWN_CLIENT
  │     │     → 直接发给自己的客户端
  │     │
  │     └── if ALL_CLIENTS / OTHER_CLIENTS
  │           → 记录到 changeDefDataLogs[detailLevel]
  │           → 等 tick 末 Witness::update() 批量广播
  │
  └── [tick 末] Witness::update()
        → 遍历 viewEntities
        → 收集每个实体的 changeDefDataLogs
        → 构造 Bundle 发送给观察者客户端
```

### 源码：onDefDataChanged

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp:658（简化）
void Entity::onDefDataChanged(EntityComponent* pEntityComponent,
    const PropertyDescription* propertyDescription, PyObject* pyData)
{
    // 只有 real entity 在非初始化状态才处理
    if (!isReal() || initing())
        return;

    // 持久化标记
    if (propertyDescription->isPersistent())
        setDirty();

    // 序列化属性值
    MemoryStream* mstream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
    propertyDescription->getDataType()->addToStream(mstream, pyData);

    uint32 flags = propertyDescription->getFlags();

    // 广播给 Ghost 实体（跨 CellApp 同步）
    if ((flags & ENTITY_BROADCAST_CELL_FLAGS) > 0 && hasGhost())
    {
        Network::Bundle* pForwardBundle = gm->createSendBundle(ghostCell());
        (*pForwardBundle).newMessage(CellappInterface::onUpdateGhostPropertys);
        (*pForwardBundle) << id();
        (*pForwardBundle) << propertyDescription->getUType();
        // ... 序列化数据
        gm->sendBundle(pForwardBundle);
    }

    // 广播给客户端
    if ((flags & ENTITY_BROADCAST_CLIENT_FLAGS) > 0)
    {
        // 记录脏属性，等 Witness::update() 批量处理
        // ...
    }

    MemoryStream::reclaimPoolObject(mstream);
}
```

## 12.4 Witness：观察者驱动的广播引擎

### KBEngine Witness

```cpp
// 文件：kbe/src/server/cellapp/witness.h（简化）
struct WitnessInfo
{
    int8 detailLevel;                           // 当前所在详情级别（0/1/2）
    Entity* entity;                             // 被观察的实体
    float range;                                // 与观察者的距离
    bool detailLevelLog[3];                     // 进入过哪些详情级别（优化用）
    std::vector<uint32> changeDefDataLogs[3];   // 各级别的脏属性记录
};

class Witness : public PoolObject, public Updatable
{
public:
    void attach(Entity* pEntity);
    void detach(Entity* pEntity);
    bool update();                              // tick 末主更新函数

    void onEnterView(ViewTrigger* pViewTrigger, Entity* pEntity);
    void onLeaveView(ViewTrigger* pViewTrigger, Entity* pEntity);

    void addUpdateToStream(Network::Bundle* pForwardBundle,
        uint32 flags, EntityRef* pEntityRef);
    void addBaseDataToStream(Network::Bundle* pSendBundle);
    uint32 getEntityVolatileDataUpdateFlags(Entity* otherEntity);

    bool sendToClient(const Network::MessageHandler& msgHandler,
        Network::Bundle* pBundle);

private:
    Entity* pEntity_;                           // 拥有 Witness 的实体
    float viewRadius_;                          // 视野半径
    float viewHysteresisArea_;                  // 滞后区域
    ViewTrigger* pViewTrigger_;                 // AOI 触发器
    VIEW_ENTITIES viewEntities_;                // 可见实体列表
    VIEW_ENTITIES_MAP viewEntities_map_;        // 可见实体映射
    uint32 clientViewSize_;                     // 当前视野内实体数量
    Position3D lastBasePos_;                    // 上次广播的基础位置
};
```

### Witness::update() 的核心逻辑

```
Witness::update() — 每个 tick 末执行
  │
  ├── 1. 处理进入/离开视野事件
  │     onEnterView → 发送 createEntity 到客户端
  │     onLeaveView → 发送 leaveEntity 到客户端
  │
  ├── 2. 更新自身位置（Volatile data）
  │     getEntityVolatileDataUpdateFlags()
  │     → 位置变化超过阈值？方向变化超过阈值？
  │     → addUpdateToStream() 发送位置/朝向
  │
  ├── 3. 广播脏属性
  │     遍历 viewEntities_：
  │       对每个可见实体：
  │         检查 changeDefDataLogs[detailLevel]
  │         → 有脏属性？构造 Bundle 发送
  │
  ├── 4. 处理 entityRef 的进出事件
  │     新进入的实体：发送完整属性初始数据
  │     已存在的实体：只发送变更的属性
  │
  └── 5. flush Bundle → 通过 Channel 发送给客户端
```

### BigWorld Witness

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/witness.hpp（简化）
class Witness : public Updatable
{
private:
    RealEntity& real_;                    // 关联的 real entity
    Entity& entity_;                      // Cell entity
    KnownEntityQueue entityQueue_;        // 优先级队列
    EntityCacheMap aoiMap_;              // AOI 内的实体缓存
    float aoiRadius_;                    // AOI 半径
    RangeListNode* pAoIRoot_;            // AOI 触发器
    int32 bandwidthDeficit_;             // 带宽管理

public:
    void update();
    void addToAoI(Entity* pEntity, bool setManuallyAdded);
    void removeFromAoI(Entity* pEntity, bool clearManuallyAdded);
    bool sendToClient(EntityID entityID, MessageID msgID,
        MemoryOStream& stream, int msgSize);
};
```

**BigWorld 的 Witness::update()** 使用**优先级队列**：

```cpp
// 简化
void Witness::update()
{
    Mercury::Bundle& bundle = this->bundle();

    // 带宽限制
    const int desiredPacketSize =
        int(maxPacketSize_ * throttle) - bandwidthDeficit_ + bundle.size();

    // 优先级队列处理
    while (bundle.size() < desiredPacketSize - 2)
    {
        EntityCache* pCache = entityQueue_.front();  // 取最高优先级
        std::pop_heap(queueBegin, queueEnd--, PriorityCompare());

        if (!pCache->isUpdatable())
            this->handleStateChange(&pCache, queueEnd);
        else
        {
            this->sendQueueElement(pCache);            // 发送更新
            pCache->updatePriority(entity_.position()); // 重算优先级
        }
    }

    this->flushToClient();
}
```

**与 KBEngine 的关键区别**：BigWorld 使用**优先级队列 + 带宽预算**——如果带宽不够，低优先级的实体更新会被推迟到下一个 tick。KBEngine 没有带宽预算机制，每个 tick 尝试同步所有变更。

## 12.5 EntityCache：BigWorld 的观察者-被观察者关系管理

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_cache.hpp（简化）
class EntityCache
{
private:
    EntityConstPtr pEntity_;
    Priority priority_;                           // 基于距离的更新优先级
    DetailLevel detailLevel_;                     // 当前 LOD 级别
    IDAlias idAlias_;                             // 客户端侧的实体 ID 别名
    EventNumber lastEventNumber_;                 // 最后发送的事件号
    VolatileNumber lastVolatileUpdateNumber_;     // 最后发送的 volatile 更新号
    EventNumber lodEventNumbers_[MAX_LOD_LEVELS]; // 每个 LOD 级别的事件戳

public:
    // 优先级计算（基于距离）
    void updatePriority(const Vector3& origin)
    {
        float distSQ = this->getDistanceSquared(origin);
        Priority delta = AoIUpdateSchemes::apply(updateSchemeID_, distSQ);
        priority_ += delta;
    }

    // 添加变更属性到流
    bool addChangedProperties(BinaryOStream& stream,
        Mercury::Bundle* pBundleForHeader, bool shouldSelectEntity);

    // 更新 LOD 级别
    bool updateDetailLevel(Mercury::Bundle& bundle,
        float lodPriority, bool hasSelectedEntity);

    static const int MAX_LOD_LEVELS = 4;
};
```

## 12.6 alias 机制：为什么属性同步包这么小

### 问题

属性同步需要在包中标识"哪个属性变了"。如果用完整的 utype（2 字节 uint16），每个属性占 2 字节。一个实体有 50 个属性，光属性 ID 就要 100 字节。

### 解决方案：aliasID

```cpp
// 文件：kbe/src/lib/entitydef/property.h（简化）
class PropertyDescription
{
    /**
     * 别名 ID：当暴露的方法或广播的属性总个数小于 255 时，
     * 使用 1 字节的 aliasID 代替 2 字节的 utype 传输
     */
    INLINE int16 aliasID() const;
    INLINE uint8 aliasIDAsUint8() const;
    INLINE void aliasID(int16 v);

    uint16 utype_;       // 全局唯一类型 ID（2 字节）
    int16 aliasID_;      // 局部别名 ID（可压缩为 1 字节）
};
```

### 使用条件

```cpp
// 在属性同步时
if (pScriptModule_->usePropertyDescrAlias())
{
    // 属性总数 < 255 → 用 1 字节 aliasID
    (*pSendBundle) << propertyDescription->aliasIDAsUint8();
}
else
{
    // 属性总数 >= 255 → 用 2 字节 utype
    (*pSendBundle) << propertyDescription->getUType();
}
```

**alias 分配算法**：`usePropertyDescrAlias()` 检查实体的广播属性总数是否小于 255。如果是，aliasID 从 0 开始顺序分配，用 1 字节传输。省下来的带宽在 2000 个可见实体 × 50 属性的场景下非常可观。

### BigWorld 的对应机制

BigWorld 使用 `IDAlias`（1 字节）压缩实体 ID，以及 `internalIndex()`（方法/属性的内部索引）减少传输开销。

```cpp
// EntityCache 中的 IDAlias
IDAlias idAlias_;    // 0xff = NO_ID_ALIAS，其他为 1 字节实体 ID 别名
```

## 12.7 detailLevel：不是所有属性都实时同步

### 问题

远处实体只需要知道位置和朝向，不需要知道精确的装备详情、buff 列表等。如果所有属性都实时同步，1000 个可见实体 × 全量属性 = 带宽爆炸。

### KBEngine 三级 detailLevel

```cpp
// WitnessInfo 中的 detailLevel：0 / 1 / 2
struct WitnessInfo
{
    int8 detailLevel;                           // 0=近, 1=中, 2=远
    bool detailLevelLog[3];                     // 进入过哪些级别
    std::vector<uint32> changeDefDataLogs[3];   // 每个级别的脏属性记录
};
```

**工作方式**：

1. **属性定义时分配 detailLevel**：每个 CLIENT_VISIBLE 属性属于一个 detailLevel
2. **距离计算**：每 tick 根据与观察者的距离确定 detailLevel
3. **按级别过滤**：只同步当前 detailLevel 及更高级别的属性

```
实体 A 观察实体 B（距离 30 米）：
  detailLevel = 0（近距离）→ 同步全部属性（位置、朝向、装备、buff、名称...）

实体 A 观察实体 C（距离 80 米）：
  detailLevel = 1（中距离）→ 同步部分属性（位置、朝向、名称、装备外观...）

实体 A 观察实体 D（距离 200 米）：
  detailLevel = 2（远距离）→ 只同步位置和朝向
```

**优化**：`detailLevelLog` 记录实体进入过哪些级别。如果实体从 level 2 变到 level 1，只需发送 level 1 的增量——因为 level 2 的属性之前已经同步过了。

### BigWorld DataLoDLevels

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_lod_level.hpp（简化）
class DataLoDLevel
{
    float low() const;      // 优先级下阈值
    float high() const;     // 优先级上阈值
    float start() const;    // 起始距离
    float hyst() const;    // 滞后值（防抖）
};

class DataLoDLevels
{
    bool needsMoreDetail(int level, float priority) const;
    bool needsLessDetail(int level, float priority) const;
};
```

**EntityCache 的 LOD 切换**：

```cpp
bool EntityCache::updateDetailLevel(Mercury::Bundle& bundle,
    float lodPriority, bool hasSelectedEntity)
{
    const DataLoDLevels& lodLevels =
        this->pEntity()->pType()->description().lodLevels();

    // 需要更多细节（实体更近了）
    if (lodLevels.needsMoreDetail(detailLevel_, lodPriority))
    {
        detailLevel_--;    // 数字越小越详细
        this->addChangedProperties(bundle, &bundle, hasSelectedEntity);
        return true;
    }
    // 需要更少细节（实体更远了）
    else if (lodLevels.needsLessDetail(detailLevel_, lodPriority))
    {
        detailLevel_++;
        this->addChangedProperties(bundle, &bundle, hasSelectedEntity);
        return true;
    }

    return false;
}
```

**BigWorld 比 KBEngine 多了**：

1. **滞后值（hysteresis）**：防抖——实体在 LOD 边界来回移动时不会频繁切换
2. **优先级阈值**：不直接用距离，而是经过 `AoIUpdateSchemes` 转换的优先级值
3. **4 级 LOD**（`MAX_LOD_LEVELS = 4`），KBEngine 3 级

## 12.8 Volatile 属性：独立的更新频率控制

位置和朝向是最高频同步的属性。它们不需要每次变化都同步——只要变化量超过阈值才发。

```cpp
// 文件：kbe/src/lib/entitydef/volatileinfo.h（简化）
class VolatileInfo : public script::ScriptObject
{
public:
    static const float ALWAYS;    // = 0.f，总是更新
    static const float NEVER;     // = -1.f，从不更新

    VolatileInfo(float position = VolatileInfo::ALWAYS,
                 float yaw = VolatileInfo::ALWAYS,
                 float roll = VolatileInfo::ALWAYS,
                 float pitch = VolatileInfo::ALWAYS):
        position_(position),
        yaw_(yaw),
        roll_(roll),
        pitch_(pitch),
        optimized_(true)
    {}

    float position() const { return position_; }
    float yaw() const { return yaw_; }
    float roll() const { return roll_; }
    float pitch() const { return pitch_; }

    void updateToNEVER();
    void updateToALWAYS();

private:
    float position_;     // 位置更新阈值
    float yaw_;          // 偏航角更新阈值
    float roll_;         // 翻滚角更新阈值
    float pitch_;        // 俯仰角更新阈值
    bool optimized_;
};
```

**在 Witness::addUpdateToStream 中的使用**：

```cpp
// 文件：kbe/src/server/cellapp/witness.cpp:956（简化）
void Witness::addUpdateToStream(Network::Bundle* pForwardBundle,
    uint32 flags, EntityRef* pEntityRef)
{
    Entity* otherEntity = pEntityRef->pEntity();

    // 根据配置选择优化级别
    static uint8 type = g_kbeSrvConfig.getCellApp().entity_posdir_updates_type;
    static uint16 threshold = g_kbeSrvConfig.getCellApp()
        .entity_posdir_updates_smart_threshold;

    bool isOptimized = true;
    if ((type == 2 && clientViewSize_ <= threshold) || type == 0)
        isOptimized = false;

    if (isOptimized)
    {
        switch (flags)
        {
        case UPDATE_FLAG_XZ:       // 只有 XZ 平面位移
        {
            Position3D relativePos = otherEntity->position()
                - this->pEntity()->position();
            pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
            // 3 字节编码（Ch10 讲过）
            break;
        }
        case UPDATE_FLAG_XYZ:      // 三维位移
        {
            Position3D relativePos = otherEntity->position()
                - this->pEntity()->position();
            pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
            pForwardBundle->appendPackY(relativePos.y);
            // 5 字节编码
            break;
        }
        case UPDATE_FLAG_YAW:      // 只有朝向
            (*pForwardBundle) << angle2int8(dir.yaw());  // 1 字节
            break;
        case UPDATE_FLAG_XYZ_YAW:  // 位移 + 朝向
            // PackXZ + PackY + angle2int8 = 6 字节
            break;
        }
    }
}
```

**Volatile 更新标志的计算**：

```cpp
uint32 Witness::getEntityVolatileDataUpdateFlags(Entity* otherEntity)
{
    uint32 flags = UPDATE_FLAG_NULL;

    // 检查位置变化是否超过阈值
    if (positionChanged && changeDistance > volatileInfo.position_)
        flags |= UPDATE_FLAG_XZ;

    // 检查朝向变化是否超过阈值
    if (yawChanged && yawDelta > volatileInfo.yaw_)
        flags |= UPDATE_FLAG_YAW;

    return flags;
}
```

### BigWorld VolatileInfo

BigWorld 的 Volatile 定义在 .def 文件的 `<Volatile>` 块中：

```xml
<ClientAvatar>
    <Volatile>
        <position/>
        <yaw/>
        <pitch>    20    </pitch>    <!-- 超过 20 度才同步 -->
    </Volatile>
</ClientAvatar>
```

## 12.9 BigWorld AoIUpdateSchemes：可插拔的更新策略

```cpp
// BigWorld 使用 AoIUpdateSchemes 抽象 AOI 更新策略
// EntityCache::updatePriority() 中：
Priority delta = AoIUpdateSchemes::apply(updateSchemeID_, distSQ);
```

不同的实体类型可以使用不同的 AOI 更新策略（比如 NPC 的更新频率可以低于玩家）。KBEngine 没有这层抽象——所有实体使用统一的更新逻辑。

## 12.10 广播的效率边界：为什么 MMO 有"最大同屏人数"

### 带宽计算

假设一个场景：
- N 个客户端同屏
- 每个客户端看到 M 个实体
- 每个实体平均 K 个脏属性
- tick 频率 10Hz

**每个客户端每秒接收的数据量**：

```
每 tick = M × K × 属性大小
每秒 = 10 × M × K × 属性大小

假设：
  M = 200（可见实体数）
  K = 5（平均脏属性数）
  属性大小 = 8 bytes（含 aliasID + 值）

每 tick = 200 × 5 × 8 = 8000 bytes = 8 KB
每秒 = 80 KB/s
```

**1000 人同屏**：

```
M = 1000, K = 3（远处实体只同步位置）
每 tick = 1000 × 3 × 5 = 15000 bytes ≈ 15 KB
每秒 = 150 KB/s
```

这已经接近很多玩家的上行带宽上限（注意：是服务器发往客户端的**下行**，但服务器需要给 N 个客户端各发 150KB/s）。

**服务器总带宽**：

```
N = 1000 个客户端
每个客户端 150 KB/s 下行
服务器总下行 = 1000 × 150 KB/s = 150 MB/s ≈ 1.2 Gbps
```

这就是为什么 MMO 有"最大同屏人数"约束——不是 CPU 算力不够，而是**带宽先到天花板**。

### 两套项目的优化手段

| 优化手段 | KBEngine | BigWorld |
|---------|----------|----------|
| alias 压缩 | aliasID（1 字节替代 2 字节） | IDAlias + internalIndex |
| 坐标压缩 | PackXZ(3B) / PackY(2B) / PackXYZ(4B) | 无内置 |
| 朝向压缩 | angle2int8（1 字节） | 类似 |
| LOD | 3 级 detailLevel | 4 级 + 滞后 |
| 带宽预算 | 无 | bandwidthDeficit + 优先级队列 |
| 脏属性合并 | tick 末批量 | tick 末批量 |
| Volatile 阈值 | 有（.def 中配置） | 有（.def 中配置） |
| 距离相关优先级 | 无 | 有（Priority 基于距离递增） |

## 12.11 两套项目的属性同步对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 同步模型 | tick 末批量 | tick 末批量 + 优先级队列 |
| 观察者管理 | Witness + ViewTrigger | Witness + RangeList |
| 被观察者缓存 | WitnessInfo（3 级） | EntityCache（4 级） |
| LOD 级别 | 3 级 | 4 级 + 滞后 |
| 带宽控制 | 无 | bandwidthDeficit 预算 |
| 优先级调度 | 无（全部同步） | 优先级队列（低优先级可延迟） |
| 脏属性记录 | changeDefDataLogs[3] | propertyEventStamps |
| 实体 ID 压缩 | 无（直接 ENTITY_ID） | IDAlias（1 字节） |
| 属性 ID 压缩 | aliasID（1 字节） | internalIndex |
| 位置编码 | PackXZ/PackY/PackXYZ | 无特殊压缩 |
| 朝向编码 | angle2int8（1 字节） | 类似 |
| Ghost 属性同步 | onUpdateGhostPropertys | DATA_GHOSTED 标记 |
| 更新策略 | 统一 | AoIUpdateSchemes 可插拔 |
| 带宽统计 | NetworkStats | PacketReceiverStats |

## 12.12 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| Witness | `kbe/src/server/cellapp/witness.h` |
| WitnessInfo | `kbe/src/server/cellapp/witness.h`（内嵌结构体） |
| Witness update | `kbe/src/server/cellapp/witness.cpp` |
| addUpdateToStream | `kbe/src/server/cellapp/witness.cpp:956` |
| onDefDataChanged | `kbe/src/server/cellapp/entity.cpp:658` |
| VolatileInfo | `kbe/src/lib/entitydef/volatileinfo.h` |
| PropertyDescription | `kbe/src/lib/entitydef/property.h` |
| aliasID | `kbe/src/lib/entitydef/property.h` |
| ViewTrigger | `kbe/src/server/cellapp/view_trigger.h` |
| EntityRef | `kbe/src/server/cellapp/entityref.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| Witness | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/witness.hpp` |
| EntityCache | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_cache.hpp` |
| DataDescription | `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_description.hpp` |
| DataLoDLevels | `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_lod_level.hpp` |
| EntityDataFlags | `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_description.hpp` |
| onOwnedPropertyChanged | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity.cpp` |

## 12.13 源码走读路径

### 路径一：跟踪一次属性变更的完整同步链路

1. `kbe/src/server/cellapp/entity.cpp:658` — `onDefDataChanged()` 属性变更入口
2. `kbe/src/server/cellapp/witness.cpp` — `update()` tick 末批量广播
3. `kbe/src/server/cellapp/witness.cpp:956` — `addUpdateToStream()` 构造同步包

### 路径二：理解 LOD / detailLevel 机制

1. `kbe/src/server/cellapp/witness.h` — `WitnessInfo` 结构体中的 detailLevel 和 detailLevelLog
2. `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/data_lod_level.hpp` — DataLoDLevels
3. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_cache.hpp` — `updateDetailLevel()`

### 路径三：对比带宽优化手段

1. `kbe/src/lib/entitydef/property.h` — aliasID 压缩
2. `kbe/src/lib/common/memorystream.h` — PackXZ/PackY 坐标压缩
3. `kbe/src/lib/entitydef/volatileinfo.h` — Volatile 更新阈值
4. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/witness.cpp` — bandwidthDeficit 带宽预算

## 12.14 小结

- **属性同步是 tick 末批量发的**：一个 tick 内多次修改只同步最终值，减少网络包数量
- **onDefDataChanged 是变更入口**：判断是否 real、是否持久化、广播给 ghost 和客户端
- **Witness 是观察者驱动的广播引擎**：每 tick 末收集所有可见实体的脏属性，构造 Bundle 批量发送
- **alias 机制把属性 ID 从 2 字节压到 1 字节**：属性总数 < 255 时启用
- **detailLevel/LOD 实现按距离分级同步**：远处实体只同步位置朝向，近处实体同步全部
- **Volatile 属性有独立的更新频率阈值**：位置和朝向变化超过阈值才发
- **BigWorld 有带宽预算和优先级队列**：带宽不够时低优先级更新被延迟
- **1000 人同屏的瓶颈是带宽**：N × M × K × tickFreq 决定了服务器总下行带宽
