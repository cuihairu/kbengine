# 14. Space、AOI 与视野系统

> 面试高频章节。回答一个核心问题：在一个有上千实体的空间中，怎么高效地知道"谁的状态需要同步给谁"？

## 14.1 本章核心问题

- AOI 解决什么问题？为什么不是"谁在附近"，而是"谁的状态需要同步给谁"？
- 九宫格、四叉树、十字链表各自的优劣势？为什么 BigWorld 和 KBEngine 都选了十字链表？
- 十字链表的具体实现（CoordinateSystem / RangeList）是怎样的？
- RangeTrigger 的进入/离开检测算法怎么工作？
- ViewTrigger 怎么把 AOI 事件变成视野管理？
- Hysteresis 防抖为什么必要？
- Witness 的 onEnterView / onLeaveView 状态机怎么工作？
- 实体移动触发 AOI 更新的完整链路是怎样的？
- 两套项目的 AOI 性能差异在哪？

## 14.2 AOI 解决什么问题

### 不是"谁在附近"，而是"谁的状态需要同步给谁"

MMO 服务器每 tick 需要同步数千个实体的状态变更。如果每个实体的变更都广播给所有玩家，1000 个玩家 × 1000 个实体 = 100 万次同步，不可行。

AOI（Area of Interest）的核心任务是：**给定一个观察者和一个视野范围，高效地找出所有需要同步的实体，并在实体进出视野时触发事件**。

```
没有 AOI：
  实体 A 移动 → 广播给所有 1000 个玩家 → 带宽爆炸

有 AOI：
  实体 A 移动 → 只广播给视野内的 20 个玩家 → 带宽可控
```

AOI 要解决的两个核心操作：

1. **范围查询**：给定中心和半径，返回所有在范围内的实体
2. **进出事件**：实体进入/离开某范围时，实时通知

### AOI 与 Witness 的关系

AOI 是"空间层"，负责回答"谁在范围内"。Witness（Ch12）是"同步层"，负责"收集变更并广播给客户端"。两者通过 ViewTrigger 连接：

```
AOI 空间层（本章）              同步层（Ch12）
  │                              │
  RangeTrigger::onEnter  →  Witness::onEnterView
  RangeTrigger::onLeave  →  Witness::onLeaveView
  │                              │
  "实体 B 进入了 A 的视野"    "把 B 的属性同步给 A 的客户端"
```

## 14.3 常见空间索引方案对比

### 方案一：九宫格（Grid）

将空间划分为固定大小的格子，每个实体属于一个格子。查询时检查周围 9 个格子。

```
优势：
  实现简单
  插入/删除 O(1)
  适合均匀分布的场景

劣势：
  格子边界问题——实体在格子边缘时，附近实体可能在相邻格子
  分布不均匀时，热门格子成为瓶颈
  视野半径变化时，格子大小难以适配
```

### 方案二：四叉树（Quadtree）

递归地将空间四等分，直到每个叶子节点内的实体数少于阈值。

```
优势：
  自适应分布——密集区域细分，稀疏区域粗分
  范围查询 O(log n + k)，k 为结果数

劣势：
  更新成本高——实体移动时可能涉及节点的分裂和合并
  不适合高频移动——MMO 中实体每 tick 都在动
  缓存不友好——树节点分散在内存中
```

### 方案三：十字链表（Cross-Linked List / Sorted Linked List）

**维护两条有序双向链表——一条按 X 排序，一条按 Z 排序**。查询时通过链表遍历确定范围。

```
优势：
  更新成本极低——移动时只需调整少量指针（冒泡排序式）
  实现简单——纯指针操作，无树节点分配/释放
  进出事件天然支持——节点互相越过时触发回调

劣势：
  范围查询 O(n)——需要遍历链表（但可被进出事件替代）
  不适合大规模全量查询
```

### 为什么 BigWorld 和 KBEngine 都选了十字链表

**因为 MMO 的实体移动是增量的、小步的**。一个 tick 内实体只移动几米，在有序链表中位置变化很小——只需要和相邻几个节点交换位置。这使得链表的"冒泡"式更新非常高效。

相比之下，九宫格的边界问题和四叉树的分裂合并在高频更新场景下反而更慢。

## 14.4 KBEngine 的十字链表实现

### 整体架构

```
Space（概念上的空间，继承 Entity）
  │
  └── SpaceMemory（实际的空间管理器）
        │
        ├── CoordinateSystem（三条有序双向链表：X / Y / Z）
        │     ├── first_x_coordinateNode_  → X 链表头
        │     ├── first_y_coordinateNode_  → Y 链表头（可选，大多数游戏不用）
        │     └── first_z_coordinateNode_  → Z 链表头
        │
        └── entities_（实体列表）
              └── Entity → EntityCoordinateNode → CoordinateNode
                                              → ViewTrigger → RangeTrigger
                                                          → RangeTriggerNode × 2（正/负边界）
```

### CoordinateNode：链表节点基类

```cpp
// 文件：kbe/src/server/cellapp/coordinate_node.h
class CoordinateNode
{
public:
    // 节点本身的坐标
    virtual float x() const { return x_; }
    virtual float y() const { return y_; }
    virtual float z() const { return z_; }

    // 扩展坐标（由子类实现，如 EntityCoordinateNode 返回实体位置）
    virtual float xx() const { return 0.f; }
    virtual float yy() const { return 0.f; }
    virtual float zz() const { return 0.f; }

    // 链表的前后端指针（三个维度各一对）
    CoordinateNode* pPrevX_; CoordinateNode* pNextX_;
    CoordinateNode* pPrevY_; CoordinateNode* pNextY_;
    CoordinateNode* pPrevZ_; CoordinateNode* pNextZ_;

    // 节点穿越回调（核心！）
    virtual void onNodePassX(CoordinateNode* pNode, bool isfront);
    virtual void onNodePassY(CoordinateNode* pNode, bool isfront);
    virtual void onNodePassZ(CoordinateNode* pNode, bool isfront);

    // 节点标志位
    uint32 flags_;
    // ENTITY / TRIGGER / HIDE / REMOVING / REMOVED /
    // PENDING / POSITIVE_BOUNDARY / NEGATIVE_BOUNDARY

protected:
    float x_, y_, z_;              // 当前坐标
    float old_xx_, old_yy_, old_zz_; // 旧坐标（用于判断移动方向）
};
```

**为什么有两套坐标（x/xx）**：`x()` 是节点在链表中的位置（可能因排序需要暂时和实体不同步），`xx()` 是实体的实际位置。插入和更新时用 `xx()` 计算，链表排序用 `x()`。

### CoordinateSystem：三条链表的管理者

```cpp
// 文件：kbe/src/server/cellapp/coordinate_system.h
class CoordinateSystem
{
public:
    bool insert(CoordinateNode* pNode);
    bool remove(CoordinateNode* pNode);
    void update(CoordinateNode* pNode);

    // 移动节点：从旧位置冒泡到正确位置
    void moveNodeX(CoordinateNode* pNode, float px, CoordinateNode* pCurrNode);
    void moveNodeY(CoordinateNode* pNode, float py, CoordinateNode* pCurrNode);
    void moveNodeZ(CoordinateNode* pNode, float pz, CoordinateNode* pCurrNode);

    static bool hasY;  // 是否启用 Y 轴（大多数 MMO 不需要）

private:
    uint32 size_;
    CoordinateNode* first_x_coordinateNode_;
    CoordinateNode* first_y_coordinateNode_;
    CoordinateNode* first_z_coordinateNode_;
};
```

### insert：新节点加入链表

```cpp
// 文件：kbe/src/server/cellapp/coordinate_system.cpp:66（简化）
bool CoordinateSystem::insert(CoordinateNode* pNode)
{
    if (isEmpty())
    {
        // 空链表，直接成为首节点
        first_x_coordinateNode_ = pNode;
        first_z_coordinateNode_ = pNode;
        pNode->x(pNode->xx());
        pNode->z(pNode->zz());
        pNode->resetOld();
        return true;
    }

    // 非空：先插入到链表头部
    pNode->old_xx(-FLT_MAX);   // 设旧坐标为负无穷
    pNode->x(first_x_coordinateNode_->x());
    first_x_coordinateNode_->pPrevX(pNode);
    pNode->pNextX(first_x_coordinateNode_);
    first_x_coordinateNode_ = pNode;

    // Z 轴同理
    pNode->old_zz(-FLT_MAX);
    pNode->z(first_z_coordinateNode_->z());
    first_z_coordinateNode_->pPrevZ(pNode);
    pNode->pNextZ(first_z_coordinateNode_);
    first_z_coordinateNode_ = pNode;

    // update 冒泡到正确位置
    update(pNode);
    return true;
}
```

**关键思路**：新节点先插到链表头部（O(1)），然后 `update()` 把它冒泡到正确位置。旧坐标设为 `-FLT_MAX` 意味着"从最左边开始移动"，过程中会触发所有被越过的节点的 `onNodePass` 回调。

### moveNodeX：冒泡排序的核心

```cpp
// 文件：kbe/src/server/cellapp/coordinate_system.cpp:264（简化）
void CoordinateSystem::moveNodeX(CoordinateNode* pNode, float px,
    CoordinateNode* pCurrNode)
{
    if (pCurrNode != NULL)
    {
        pNode->x(pCurrNode->x());

        if (pNode->pPrevX() == pCurrNode)
        {
            // pNode 需要向左（负方向）冒泡
            // 把 pNode 从当前位置摘出，插入到 pCurrNode 前面
            CoordinateNode* pPreNode = pCurrNode->pPrevX();
            pCurrNode->pPrevX(pNode);
            if (pPreNode) pPreNode->pNextX(pNode);
            else first_x_coordinateNode_ = pNode;

            // ... 指针调整 ...
            pNode->pPrevX(pPreNode);
            pNode->pNextX(pCurrNode);
        }
        else
        {
            // pNode 需要向右（正方向）冒泡
            // 把 pNode 插入到 pCurrNode 后面
            CoordinateNode* pNextNode = pCurrNode->pNextX();
            // ... 指针调整 ...
            pNode->pPrevX(pCurrNode);
            pNode->pNextX(pNextNode);
        }

        // 触发穿越回调（双向通知）
        pCurrNode->onNodePassX(pNode, true);   // pNode 越过了 pCurrNode
        pNode->onNodePassX(pCurrNode, false);  // pCurrNode 被 pNode 越过
    }
}
```

这一步是 KBEngine AOI 链路里的关键连接点。每次冒泡交换一对节点，同时触发双方的 `onNodePassX` 回调；真正的进入/离开判定发生在 `RangeTriggerNode` / `RangeTrigger` 一侧。

## 14.5 BigWorld 的十字链表实现

### RangeListNode：链表节点基类

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.hpp
enum RangeListOrder
{
    RANGE_LIST_ORDER_HEAD         = 0,     // 链表头哨兵
    RANGE_LIST_ORDER_ENTITY       = 100,   // 实体节点
    RANGE_LIST_ORDER_LOWER_BOUND  = 190,   // 触发器下界
    RANGE_LIST_ORDER_UPPER_BOUND  = 200,   // 触发器上界
    RANGE_LIST_ORDER_TAIL         = 0xffff // 链表尾哨兵
};

class RangeListNode
{
public:
    enum RangeListFlags
    {
        FLAG_NO_TRIGGERS        = 0,
        FLAG_ENTITY_TRIGGER     = 0x01,   // 想知道实体穿越
        FLAG_LOWER_AOI_TRIGGER  = 0x02,   // 想知道 AOI 下界穿越
        FLAG_UPPER_AOI_TRIGGER  = 0x04,   // 想知道 AOI 上界穿越
        FLAG_IS_ENTITY          = 0x10,   // 自己是实体
        FLAG_IS_LOWER_BOUND     = 0x20    // 自己是下界节点
    };

    // X/Z 双向链表指针
    RangeListNode* pPrevX_;  RangeListNode* pNextX_;
    RangeListNode* pPrevZ_;  RangeListNode* pNextZ_;

    // 标志位：这个节点想接收哪种穿越事件 + 自己能产生哪种穿越事件
    RangeListFlags wantsFlags_;
    RangeListFlags makesFlags_;
    RangeListOrder order_;    // 排序优先级（相同坐标时的稳定排序）

    // 穿越回调
    virtual void crossedX(RangeListNode* node, bool positiveCrossing,
        float oldOthX, float oldOthZ) {}
    virtual void crossedZ(RangeListNode* node, bool positiveCrossing,
        float oldOthX, float oldOthZ) {}

    // 判断是否需要与对方触发穿越事件
    bool wantsCrossingWith(RangeListNode* pOther) const
    {
        return (wantsFlags_ & pOther->makesFlags_);
    }
};
```

**与 KBEngine 的关键区别**：

| 维度 | KBEngine CoordinateNode | BigWorld RangeListNode |
|------|------------------------|----------------------|
| 维度 | X / Y / Z 三条链表 | X / Z 两条链表（无 Y） |
| 穿越回调 | `onNodePassX/Y/Z` | `crossedX/Z` |
| 穿越过滤 | 无（所有穿越都回调） | `wantsFlags_` × `makesFlags_` 按需过滤 |
| 排序稳定 | 无 | `RangeListOrder` 优先级 |
| 边界节点 | `POSITIVE/NEGATIVE_BOUNDARY` flag | `LOWER/UPPER_BOUND` order |

### shuffleXThenZ：BigWorld 的冒泡算法

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.cpp:29
void RangeListNode::shuffleXThenZ(float oldX, float oldZ)
{
    this->shuffleX(oldX, oldZ);
    this->shuffleZ(oldX, oldZ);
}

void RangeListNode::shuffleX(float oldX, float oldZ)
{
    float ourPosX = this->x();
    float othPosX;

    // 向左冒泡（负 X 方向）
    while (pPrevX_ != NULL &&
            (ourPosX < (othPosX = pPrevX_->x()) ||
                (isEqual(ourPosX, othPosX) &&
                order_ <= pPrevX_->order_)))
    {
        // 双向穿越通知（带 wantsFlags 过滤）
        if (this->wantsCrossingWith(pPrevX_))
            this->crossedX(pPrevX_, true, pPrevX_->x(), pPrevX_->z());
        if (pPrevX_->wantsCrossingWith(this))
            pPrevX_->crossedX(this, false, oldX, oldZ);

        // 链表指针交换（经典的冒泡式节点交换）
        if (pNextX_ != NULL) pNextX_->pPrevX_ = pPrevX_;
        pPrevX_->pNextX_ = pNextX_;
        pNextX_ = pPrevX_;
        pPrevX_ = pPrevX_->pPrevX_;
        if (pPrevX_ != NULL) pPrevX_->pNextX_ = this;
        pNextX_->pPrevX_ = this;
    }

    // 向右冒泡（正 X 方向）—— 结构对称
    // ...
}
```

**算法分析**：

- 每次循环交换一对相邻节点——O(1) 指针操作
- 循环次数 = 节点移动距离（在链表中越过的节点数）
- MMO 中实体每帧移动距离很小（可能只越过 0~3 个节点），所以实际接近 O(1)
- **最坏情况 O(n)**（实体传送），但传送是低频操作

**wantsCrossingWith 过滤**：不是所有穿越都需要处理。实体节点可能不关心其他实体的穿越（只关心触发器的穿越）。通过 `wantsFlags_ & makesFlags_` 位运算，跳过不关心的穿越事件。

## 14.6 RangeTrigger：进入/离开检测

### KBEngine RangeTrigger

```cpp
// 文件：kbe/src/server/cellapp/range_trigger.h
class RangeTrigger
{
public:
    RangeTrigger(CoordinateNode* origin, float xz, float y);

    bool install();     // 安装到 CoordinateSystem
    bool uninstall();   // 卸载
    bool reinstall(CoordinateNode* pCoordinateNode);

    // 进入/离开事件（由子类实现）
    virtual void onEnter(CoordinateNode* pNode) = 0;
    virtual void onLeave(CoordinateNode* pNode) = 0;

    // 穿越回调：从 CoordinateNode::onNodePass 转发
    virtual void onNodePassX(RangeTriggerNode* pRangeTriggerNode,
        CoordinateNode* pNode, bool isfront);
    virtual void onNodePassY(RangeTriggerNode* pRangeTriggerNode,
        CoordinateNode* pNode, bool isfront);
    virtual void onNodePassZ(RangeTriggerNode* pRangeTriggerNode,
        CoordinateNode* pNode, bool isfront);

protected:
    float range_xz_, range_y_;                    // 触发半径
    CoordinateNode* origin_;                       // 中心实体节点
    RangeTriggerNode* positiveBoundary_;           // 正边界节点（origin + range）
    RangeTriggerNode* negativeBoundary_;           // 负边界节点（origin - range）
};
```

### RangeTriggerNode：边界节点

```cpp
// 文件：kbe/src/server/cellapp/range_trigger_node.h
class RangeTriggerNode : public CoordinateNode
{
public:
    // 边界位置 = 中心实体位置 ± range
    virtual float xx() const;  // { return origin->xx() + range_xz_ 或 -range_xz_ }
    virtual float zz() const;

    // 判断另一个节点是否在范围内（X/Y/Z 分开判断）
    INLINE bool isInXRange(CoordinateNode* pNode);
    INLINE bool isInYRange(CoordinateNode* pNode);
    INLINE bool isInZRange(CoordinateNode* pNode);

    // 判断旧位置是否在范围内（用于检测离开）
    INLINE bool wasInXRange(CoordinateNode* pNode);
    INLINE bool wasInZRange(CoordinateNode* pNode);

    // 穿越回调
    virtual void onNodePassX(CoordinateNode* pNode, bool isfront);
    virtual void onNodePassZ(CoordinateNode* pNode, bool isfront);

protected:
    float range_xz_, range_y_, old_range_xz_, old_range_y_;
    RangeTrigger* pRangeTrigger_;
};
```

### 进入/离开检测算法

RangeTrigger 用**两个边界节点**实现范围检测：

```
                  负边界               正边界
X 轴链表:  ... ← [origin - range] ← ... 实体 ... → [origin + range] → ...

当实体节点 B 在链表中越过负边界时：
  如果 B 之前不在范围内，现在在范围内 → onEnter(B)
  如果 B 之前在范围内，现在不在范围内 → onLeave(B)

判断逻辑：
  wasInXRange(B) && isInZRange(B)  → 之前在范围内
  isInXRange(B) && isInZRange(B)   → 现在在范围内

  之前不在 && 现在在 → Enter
  之前在 && 现在不在 → Leave
```

**为什么需要两个边界**：一个范围触发器有两个边界（正/负），实体可以从任一侧进入或离开。边界节点和实体节点混在同一个有序链表中，当它们互相越过时触发检测。

```
实体 B 向右移动，越过负边界节点：

  链表状态（移动前）：  ... B ... [负边界] ... origin ... [正边界] ...
  链表状态（移动后）：  ... [负边界] ... B ... origin ... [正边界] ...

  → B 越过负边界 → wasInXRange(B)=false, isInXRange(B)=true → Enter!
```

### BigWorld RangeTrigger

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_trigger.hpp
class RangeTrigger
{
public:
    RangeTrigger(RangeListNode* pCentralNode, float range, ...);

    virtual void triggerEnter(Entity& entity) = 0;
    virtual void triggerLeave(Entity& entity) = 0;

    bool contains(RangeListNode* pQuery) const;
    bool isInXRange(float x, float range) const;
    bool isInZRange(float z, float range) const;

protected:
    RangeListNode* pCentralNode_;
    RangeTriggerNode upperBound_;   // 上界节点
    RangeTriggerNode lowerBound_;   // 下界节点
    float oldEntityX_, oldEntityZ_; // 旧坐标
};
```

BigWorld 的 `volatile float` 计算值得注意——它强制浮点精度与链表排序一致，避免因精度差异导致的漏检：

```cpp
bool isInXRange(float x, float range) const
{
    float subX = pCentralNode_->x();
    // volatile 确保计算精度与链表排序时一致
    volatile float lowerBound = subX - range;
    volatile float upperBound = subX + range;
    return (lowerBound < x) && (x <= upperBound);
}
```

## 14.7 ViewTrigger：视野特化

### KBEngine ViewTrigger

```cpp
// 文件：kbe/src/server/cellapp/view_trigger.h
class ViewTrigger : public RangeTrigger
{
public:
    ViewTrigger(CoordinateNode* origin, float xz = 0.0f, float y = 0.0f);

    // 进入/离开视野
    virtual void onEnter(CoordinateNode* pNode);
    virtual void onLeave(CoordinateNode* pNode);

    Witness* pWitness() const;

private:
    Witness* pWitness_;   // 关联的 Witness
};
```

ViewTrigger 是 RangeTrigger 的特化——范围半径是玩家的视野距离，进入/离开事件直接转发给 Witness：

```
ViewTrigger::onEnter(pNode)
  │
  ├── pNode 必须是 EntityCoordinateNode（实体节点）
  │
  └── pWitness_->onEnterView(pEntity)
        → 将实体加入 viewEntities_
        → 标记为 ENTER_PENDING
        → 等 tick 末 Witness::update() 发送给客户端

ViewTrigger::onLeave(pNode)
  │
  └── pWitness_->onLeaveView(pEntity)
        → 将实体从 viewEntities_ 移除
        → 标记为 LEAVE_PENDING
        → 等 tick 末 Witness::update() 发送给客户端
```

### Hysteresis 防抖区域

视野系统的一个关键问题：**实体在视野边界来回移动时，会频繁触发 Enter/Leave 事件**。

```
没有防抖：
  实体 B 在视野半径边缘来回移动
  tick 1: B 进入 → 创建实体 + 全量属性同步
  tick 2: B 离开 → 销毁实体
  tick 3: B 进入 → 又创建 + 全量属性同步
  → 带宽浪费 + 客户端闪烁

有防抖（滞后区域）：
  进入半径 = 50m
  离开半径 = 55m（比进入半径大 5m 的滞后区域）

  B 从远处靠近：
    50m → Enter（进入视野）
  B 在边界附近移动：
    52m → 不触发（仍在视野内）
    48m → 不触发（仍在视野内）
  B 离开：
    55m → Leave（真正离开视野）
```

KBEngine 通过 `viewHysteresisArea_` 配置实现，BigWorld 的 `DataLoDLevel` 中的 `hyst()` 值同样用于防抖。

## 14.8 Witness：从 AOI 事件到客户端同步

### KBEngine Witness 的视野状态机

Witness 维护了两种 pending 状态——实体不是立即同步给客户端，而是等到 tick 末统一处理：

```
ViewTrigger::onEnter(entity)
  → 加入 viewEntities_，标记 ENTER_PENDING

ViewTrigger::onLeave(entity)
  → 标记 LEAVE_PENDING

Witness::update()（tick 末）
  │
  ├── 处理 ENTER_PENDING 的实体
  │     → 发送 createEntity 消息到客户端
  │     → 包含实体的全部可见属性
  │     → 状态变为 NORMAL
  │
  ├── 处理 LEAVE_PENDING 的实体
  │     → 发送 leaveEntity 消息到客户端
  │     → 从 viewEntities_ 移除
  │
  └── 处理 NORMAL 实体的脏属性
        → 收集 changeDefDataLogs
        → 构造 Bundle 批量发送
```

**为什么不是立即发消息**（Ch12 也讲过）：

1. 一个 tick 内实体可能反复进出视野（边界防抖）
2. 多个实体的进入/离开事件可以合并到一个 Bundle
3. 进入时需要发送完整属性数据，和后续的脏属性更新可以合并

### EntityCoordinateNode：实体节点与触发器的关联

```cpp
// 文件：kbe/src/server/cellapp/entity_coordinate_node.h
class EntityCoordinateNode : public CoordinateNode
{
public:
    EntityCoordinateNode(Entity* pEntity);

    // 扩展坐标 = 实体的实际位置
    virtual float xx() const;
    virtual float yy() const;
    virtual float zz() const;

    // 管理关注此实体的触发器节点
    bool addWatcherNode(CoordinateNode* pNode);
    bool delWatcherNode(CoordinateNode* pNode);

    // 查询范围内的实体（静态查询）
    static void entitiesInRange(std::vector<Entity*>& foundEntities,
        CoordinateNode* rootNode, const Position3D& originPos,
        float radius, int entityUType = -1);

protected:
    Entity* pEntity_;
    typedef std::vector<CoordinateNode*> WATCHER_NODES;
    WATCHER_NODES watcherNodes_;    // 关联的触发器边界节点列表
};
```

`watcherNodes_` 记录了所有"关注这个实体"的 RangeTriggerNode。当实体被删除时，需要通知所有 watcher。

### BigWorld Witness 的视野管理

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/witness.hpp（简化）
class Witness : public Updatable
{
public:
    void addToAoI(Entity* pEntity, bool setManuallyAdded);
    void removeFromAoI(Entity* pEntity, bool clearManuallyAdded);

private:
    EntityCacheMap aoiMap_;           // AOI 内所有实体的缓存
    KnownEntityQueue entityQueue_;    // 优先级队列（带宽管理）
    float aoiRadius_;                 // AOI 半径
    RangeListNode* pAoIRoot_;         // AOI 触发器根节点
    int32 bandwidthDeficit_;          // 带宽赤字（限流用）
};
```

BigWorld 的 Witness 比 KBEngine 多了**优先级队列 + 带宽预算**机制（Ch12 已讲）。

## 14.9 实体移动触发 AOI 更新的完整链路

### KBEngine

```
Python: entity.position = (10, 0, 20)
  │
  ├── Entity::onDefDataChanged(positionProperty)
  │     → 标记脏属性
  │
  ├── EntityCoordinateNode::update()
  │     → 更新节点坐标
  │     → 通知 CoordinateSystem::update(this)
  │
  ├── CoordinateSystem::update(pNode)
  │     ├── moveNodeX(pNode, new_xx, pCurrNode)
  │     │     冒泡到正确位置
  │     │     每次交换触发 onNodePassX 回调
  │     │
  │     ├── moveNodeZ(pNode, new_zz, pCurrNode)
  │     │     同上
  │     │
  │     └── moveNodeY（如果启用）
  │
  ├── RangeTriggerNode::onNodePassX(pNode, isfront)
  │     → RangeTrigger::onNodePassX(this, pNode, isfront)
  │     → 检测进入/离开
  │     → 如果 Enter → ViewTrigger::onEnter → Witness::onEnterView
  │     → 如果 Leave → ViewTrigger::onLeave → Witness::onLeaveView
  │
  └── [tick 末] Witness::update()
        → 批量发送视野变更到客户端
```

### BigWorld

```
Entity 位置更新
  │
  ├── EntityRangeListNode::shuffleXThenZ(oldX, oldZ)
  │     ├── shuffleX(oldX, oldZ)
  │     │     冒泡 + crossedX 回调
  │     ├── shuffleZ(oldX, oldZ)
  │     │     冒泡 + crossedZ 回调
  │
  ├── RangeTriggerNode::crossedX/Z(node, positiveCrossing, ...)
  │     → RangeTrigger 检测 contains()
  │     → triggerEnter() 或 triggerLeave()
  │
  └── Witness::addToAoI / removeFromAoI
        → EntityCache 加入/移除
        → 更新优先级队列
        → [tick 末] Witness::update() 批量发送
```

## 14.10 两套项目的 AOI 系统对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 链表维度 | X / Y / Z 三条（Y 可选） | X / Z 两条 |
| 链表节点基类 | CoordinateNode | RangeListNode |
| 实体节点 | EntityCoordinateNode | EntityRangeListNode |
| 触发器 | RangeTrigger + RangeTriggerNode | RangeTrigger + RangeTriggerNode |
| 边界表示 | positiveBoundary + negativeBoundary | upperBound_ + lowerBound_ |
| 视野触发器 | ViewTrigger : RangeTrigger | Witness 中的 AOI 触发器 |
| 穿越回调 | onNodePassX/Y/Z | crossedX/Z |
| 穿越过滤 | 无（全部回调） | wantsFlags_ × makesFlags_ |
| 排序稳定 | 无 | RangeListOrder 优先级 |
| 冒泡算法 | moveNodeX/Y/Z | shuffleX/shuffleZ |
| 防抖机制 | viewHysteresisArea_ | hyst() 值 |
| 带宽管理 | 无 | bandwidthDeficit + 优先级队列 |
| 浮点精度 | 无特殊处理 | volatile float 强制精度一致 |
| 静态查询 | entitiesInRange() | 无内置（通过进出事件维护） |

**核心差异分析**：

1. **穿越过滤**：BigWorld 用 `wantsFlags_` 位运算过滤不关心的穿越事件，减少不必要的回调。KBEngine 没有这层优化——所有穿越都触发回调，由 RangeTriggerNode 内部判断是否需要处理

2. **排序稳定**：BigWorld 的 `RangeListOrder` 确保相同坐标时节点的确定性排序（实体 < 下界 < 上界），避免穿越事件的歧义。KBEngine 没有显式的优先级，依赖节点类型标志位隐式处理

3. **浮点精度**：BigWorld 用 `volatile float` 防止编译器优化导致精度不一致。KBEngine 没有这个处理——在高精度场景下可能出现漏检

## 14.11 性能分析：十字链表 vs 其他方案

### 时间复杂度

| 操作 | 十字链表 | 九宫格 | 四叉树 |
|------|---------|--------|--------|
| 插入 | O(d) | O(1) | O(log n) |
| 删除 | O(d) | O(1) | O(log n) |
| 移动更新 | O(d) | O(1) | O(log n) |
| 范围查询 | O(n) | O(k) | O(log n + k) |
| 进出事件 | O(d)（天然） | 需额外实现 | 需额外实现 |

**d = 移动距离（越过的节点数），n = 总实体数，k = 结果数**

### 实际场景分析

```
场景：一个 CellApp 管理的空间，2000 个实体

典型移动：实体每 tick 移动 5 米
  → 链表中平均越过 0~3 个节点
  → d ≈ 2，更新耗时 ≈ 常数

传送操作：实体瞬移到另一位置
  → 链表中可能越过数百个节点
  → d ≈ n，但传送是低频操作

全量范围查询：
  → 十字链表 O(n) = 2000
  → 如果用进出事件维护 viewEntities，则不需要全量查询
```

**结论**：十字链表的 O(n) 全量查询不是问题，因为 MMO 服务器不需要全量查询——进出事件天然维护了视野列表。移动更新的 O(d) 在实际场景中接近 O(1)。

### 空间复杂度

| 方案 | 每实体额外空间 |
|------|--------------|
| 十字链表 | 6 个指针（prev/next × 3 维）= 48 字节 |
| 九宫格 | 哈希表项 = ~32 字节 |
| 四叉树 | 树节点共享 = ~16 字节/实体 |

十字链表的空间开销适中。考虑到 MMO 服务器内存不是瓶颈（每个 CellApp 几千个实体），这点开销完全可接受。

## 14.12 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| Space | `kbe/src/server/cellapp/space.h` |
| SpaceMemory | `kbe/src/server/cellapp/spacememory.h` |
| CoordinateSystem | `kbe/src/server/cellapp/coordinate_system.h` |
| CoordinateSystem 实现 | `kbe/src/server/cellapp/coordinate_system.cpp` |
| CoordinateNode | `kbe/src/server/cellapp/coordinate_node.h` |
| EntityCoordinateNode | `kbe/src/server/cellapp/entity_coordinate_node.h` |
| RangeTrigger | `kbe/src/server/cellapp/range_trigger.h` |
| RangeTriggerNode | `kbe/src/server/cellapp/range_trigger_node.h` |
| ViewTrigger | `kbe/src/server/cellapp/view_trigger.h` |
| Witness | `kbe/src/server/cellapp/witness.h` |
| SpaceMemorys | `kbe/src/server/cellapp/spacememorys.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| RangeListNode | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.hpp` |
| shuffle 算法 | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.cpp` |
| EntityRangeListNode | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_range_list_node.hpp` |
| RangeTrigger | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_trigger.hpp` |
| Space | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/space.hpp` |
| Cell | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/cell.hpp` |
| Witness | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/witness.hpp` |
| EntityCache | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/entity_cache.hpp` |
| AoIUpdateSchemes | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/aoi_update_schemes.hpp` |

## 14.13 源码走读路径

### 路径一：理解十字链表的数据结构

1. `kbe/src/server/cellapp/coordinate_node.h` — CoordinateNode 基类，prev/next 指针，onNodePass 回调
2. `kbe/src/server/cellapp/coordinate_system.h` — CoordinateSystem，三条链表头指针
3. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.hpp` — RangeListNode，wantsFlags/makesFlags，RangeListOrder

### 路径二：跟踪实体移动触发的 AOI 更新

1. `kbe/src/server/cellapp/coordinate_system.cpp:264` — `moveNodeX()` 冒泡交换
2. `kbe/src/server/cellapp/range_trigger_node.h` — `onNodePassX()` 触发进入/离开检测
3. `kbe/src/server/cellapp/view_trigger.h` — `onEnter()` / `onLeave()` 转发到 Witness
4. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.cpp:44` — `shuffleX()` 冒泡 + crossedX 回调

### 路径三：对比 BigWorld 的穿越过滤机制

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_list_node.hpp` — `wantsFlags_` / `makesFlags_` 定义
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/range_trigger.hpp` — `isInXRange()` volatile float 精度处理
3. 对比 KBEngine 无过滤 vs BigWorld 按需过滤的差异

## 14.14 小结

- **AOI 解决的是"谁的状态需要同步给谁"**：不是简单的范围查询，而是进出事件驱动的视野管理
- **十字链表是 MMO 的最优选择**：增量的小步移动使冒泡更新接近 O(1)，进出事件天然支持
- **CoordinateSystem / RangeList 是三条/两条有序双向链表**：X / Z（/ Y）各一条，实体和触发器边界混合排列
- **RangeTrigger 用两个边界节点检测进出**：正/负边界节点在链表中和实体节点一起排序，互相越过时触发检测
- **ViewTrigger 是视野特化**：将 AOI 事件转发给 Witness，触发 onEnterView / onLeaveView
- **Hysteresis 防抖防止边界抖动**：进入半径 < 离开半径，避免频繁进出
- **Witness 的 pending 状态机延迟到 tick 末处理**：不是立即发消息，而是批量发送
- **BigWorld 多了穿越过滤和精度保护**：wantsFlags 过滤不关心的穿越，volatile float 防止精度不一致
- **十字链表的"弱点"（O(n) 全量查询）在实际场景中不是问题**：MMO 通过进出事件维护视野列表，不需要全量查询
