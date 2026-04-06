# 15. 空间拓扑与动态扩容

> BigWorld 核心功能。KBEngine 在这方面简化很多。这一章回答：当一个 MMO 世界大到单个 CellApp 处理不了时，怎么切分空间、动态迁移实体、保持负载均衡？

## 15.1 本章核心问题

- 为什么单个 CellApp 处理不了大世界？
- BigWorld 的 BSP 树怎么把一个 Space 切成多个 Cell？
- 动态负载均衡的 `doBalance` 算法怎么工作？
- grow / shrink 怎么让 Cell 边界动态移动？
- Offload 怎么把实体从一个 Cell 迁移到另一个？
- CellApp 宕机后怎么恢复？
- KBEngine 为什么不做动态负载均衡？取舍是什么？

## 15.2 问题的本质：大世界 > 单进程容量

一个 CellApp 的处理能力有上限——CPU tick 预算、内存容量、网络带宽。当一个 Space 中的实体数量超过单个 CellApp 的处理能力时，需要将 Space 切分成多个 Cell，分布到不同的 CellApp 上。

```
Space（逻辑上的完整世界）
  │
  ├── 方案一：整个 Space 放在一个 CellApp 上
  │     优势：简单，无跨进程通信
  │     劣势：实体数受限、无法水平扩展
  │
  └── 方案二：把 Space 切成多个区域，每个区域一个 Cell
        优势：水平扩展、单区域实体数可控
        劣势：跨区域通信、Ghost 同步、动态迁移复杂
```

BigWorld 选择了方案二，并实现了完整的 BSP 树切分 + 动态负载均衡。KBEngine 选择了方案一——每个 Space 只有一个 Cell。

## 15.3 BigWorld BSP 树：空间分割的数据结构

### BSPNode 类层次

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/bsp_node.hpp（简化）
enum BSPNodeType
{
    BSP_NODE_HORIZONTAL,   // 水平分割线（沿 X 轴分割）
    BSP_NODE_VERTICAL,     // 垂直分割线（沿 Z 轴分割）
    BSP_NODE_LEAF          // 叶子节点 = Cell
};

class BSPNode : public WatcherProvider
{
public:
    virtual void balance(const BW::Rect& range, float loadSafetyBound,
        bool isShrinking) = 0;
    virtual void updateRanges(const BW::Rect& range) = 0;
    virtual float updateLoad() = 0;
    virtual float avgLoad() const = 0;
    virtual float totalSmoothedLoad() const = 0;

    const BW::Rect& range() const { return range_; }
    const EntityBoundLevels& entityBoundLevels() const { return entityBoundLevels_; }

protected:
    BW::Rect range_;                         // 节点管理的空间范围
    EntityBoundLevels entityBoundLevels_;    // 实体在各层级的边界数据
    BW::Rect chunkBounds_;                   // 地形块边界
    int numRetiring_;                        // 正在退休的 Cell 数
    float areaNotLoaded_;                    // 未加载面积
};
```

### InternalNode：分割线 + 两棵子树

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/internal_node.hpp（简化）
class InternalNode : public BSPNode
{
public:
    InternalNode(BSPNode* pLeft, BSPNode* pRight,
        bool isHorizontal, float position);

    // 负载均衡核心
    void balance(const BW::Rect& range, float loadSafetyBound,
        bool isShrinking);
    BalanceDirection doBalance(float loadSafetyBound);
    void balanceChildren(float loadSafetyBound, BalanceDirection balanceDir);

    float updateLoad();
    float avgLoad() const;

    // 分割线管理
    void position(float newPos) { position_ = newPos; }
    float position() const { return position_; }
    bool isHorizontal() const { return isHorizontal_; }

    BSPNode* pLeft() const { return pLeft_; }
    BSPNode* pRight() const { return pRight_; }

private:
    enum BalanceDirection
    {
        BALANCE_LEFT,     // 左子树负载更重，分割线应该左移
        BALANCE_NONE,     // 两边均衡
        BALANCE_RIGHT     // 右子树负载更重，分割线应该右移
    };

    BSPNode* pLeft_;          // 左/下子树
    BSPNode* pRight_;         // 右/上子树
    bool isHorizontal_;       // 分割方向
    float position_;          // 分割线位置
    float balanceAggression_; // 平衡激进度
};
```

### BSP 树的可视化

```
Space = [0, 1000] × [0, 1000]

                InternalNode(H, y=500)           ← 水平分割，y=500
                /                    \
    InternalNode(V, x=300)      InternalNode(V, x=700)   ← 垂直分割
       /           \                /           \
  LeafNode       LeafNode      LeafNode       LeafNode
  Cell A         Cell B        Cell C         Cell D
  [0,300]×      [300,1000]×    [0,700]×      [700,1000]×
  [0,500]       [0,500]        [500,1000]     [500,1000]
```

每个 LeafNode 对应一个 Cell，由一个 CellApp 管理。BSP 树的分割线可以动态移动——当某个 Cell 负载过重时，分割线向负载轻的方向移动，把一些实体的管辖权转移过去。

### SpaceBranch：CellApp 侧的 BSP 表示

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/space.hpp（简化）
// SpaceBranch 是 CellApp 侧维护的 BSP 节点视图
class SpaceBranch
{
    bool isHorizontal_;       // 分割方向
    float position_;          // 分割线位置

    SpaceNode* pLeft_;        // 左/下子树
    SpaceNode* pRight_;       // 右/上子树

    void visitRect(const BW::Rect& rect, CellInfoVisitor& visitor);
    const CellInfo* pCellAt(float x, float z) const;
};
```

CellApp 使用 SpaceBranch 知道自己的空间范围，以及相邻 Cell 的位置。

## 15.4 动态负载均衡：doBalance 算法

### 核心思路

```
1. 计算左右子树的平均负载
2. 负载差异超过阈值 → 决定平衡方向
3. 移动分割线位置（将部分实体从重载侧转移到轻载侧）
4. 递归到子节点
```

### doBalance 实现

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/internal_node.cpp（简化）
InternalNode::BalanceDirection InternalNode::doBalance(float loadSafetyBound)
{
    const float leftAvgLoad = pLeft_->avgLoad();
    const float rightAvgLoad = pRight_->avgLoad();
    const float nodeAvgLoad = this->avgLoad();

    // 计算负载差异（带符号：正数=左重，负数=右重）
    float loadDiff = (leftAvgLoad > rightAvgLoad) ?
        fabs(leftAvgLoad - nodeAvgLoad) :
        -fabs(rightAvgLoad - nodeAvgLoad);

    // 从负载差异推导平衡方向
    BalanceDirection balanceDir = this->dirFromLoadDiff(loadDiff);

    // 检查是否应该移动分割线
    bool shouldMove = (balanceDir != BALANCE_NONE) &&
        (pFromNode->maxLoad() < loadSafetyBound);

    if (shouldMove)
    {
        // 计算分割线应该移动多远
        float entityLimit = this->entityLimitInDirection(balanceDir,
            loadDiff * balanceAggression_);

        // 可选：限制在地形块（chunk）边界上移动
        float chunkLimit = shouldLimitToChunks ?
            this->chunkLimitInDirection(balanceDir) : entityLimit;

        // 取最近的限制
        newPos = this->closestLimit(position_, entityLimit,
            chunkLimit, balanceDir);
    }

    position_ = newPos;  // 移动分割线
    return balanceDir;
}
```

**关键参数**：

- `balanceAggression_`：平衡激进度。值越大，分割线移动越快。典型值 0.5~1.0
- `loadSafetyBound`：安全负载上界。如果某侧负载已超过此值，不再从它移走实体
- `entityLimitInDirection`：根据实体边界计算分割线的移动上限——不能把一个实体切成两半

### balanceChildren：递归均衡

```cpp
void InternalNode::balanceChildren(float loadSafetyBound,
    BalanceDirection balanceDir)
{
    // 递归到子节点继续均衡
    if (balanceDir == BALANCE_LEFT || balanceDir == BALANCE_NONE)
        pLeft_->balance(range_, loadSafetyBound, isShrinking);

    if (balanceDir == BALANCE_RIGHT || balanceDir == BALANCE_NONE)
        pRight_->balance(range_, loadSafetyBound, isShrinking);
}
```

### EntityBoundLevels：实体边界数据

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/entity_bound_levels.hpp（简化）
class EntityBoundLevels
{
public:
    void merge(const EntityBoundLevels& left, const EntityBoundLevels& right,
        bool isHorizontal, float position);

    float entityBoundForLoadDiff(float loadDiff, bool isHorizontal,
        bool isMax, float defaultPosition) const;

private:
    typedef BW::vector<BW::Rect> Rects;
    Rects entityBounds_;     // 实体边界矩形（用于计算分割线位置）
    Rects entityLoads_;      // 实体负载信息
};
```

EntityBoundLevels 记录了在每个 BSP 层级上实体的分布情况。当分割线移动时，它确保分割线不会穿过实体——而是移动到最近的"无实体"位置。

## 15.5 CellAppGroup：管理一组 CellApp 的负载

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/cell_app_group.hpp（简化）
class CellAppGroup
{
public:
    void addCell();                                    // 扩容：添加新 Cell
    void checkForUnderloaded(float loadLowerBound);    // 缩容：检查是否需要移除 Cell
    float avgLoad(int ifNumRemoved = 0) const;         // 计算平均负载

    typedef BW::set<CellApp*> Set;
    Set set_;                                          // CellApp 集合
    float avgLoad_;                                    // 当前平均负载
};
```

这里的 `CellAppGroup` 不是任意一组 CellApp，而是能够通过 multi-cell space 相互均衡负载的一组 CellApp。

### 扩容：添加新 Cell

```
新 CellApp 加入集群
  │
  ├── CellAppGroup::addCell()
  │     → 先 chooseConnectionSpace()
  │     → 由 Space::addCell() 决定在哪个 space 上新增 cell
  │     → 再由该 space 的 BSP 树完成具体分裂
  │
  └── BSP 树自动 re-balance
```

### 缩容：移除空闲 Cell

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/cell_app_group.cpp（简化）
void CellAppGroup::checkForUnderloaded(float loadLowerBound)
{
    // 如果移除一个 CellApp 后平均负载仍然低于下界
    if (this->avgLoad(1) < loadLowerBound)
    {
        CellApp* pLeastLoaded = NULL;
        float leastLoad = FLT_MAX;

        // 找到负载最轻的 CellApp
        iterator iter = this->begin();
        while (iter != this->end())
        {
            if (!(*iter)->hasOnlyRetiringCells() &&
                (*iter)->smoothedLoad() < leastLoad)
            {
                pLeastLoaded = *iter;
                leastLoad = (*iter)->smoothedLoad();
            }
            ++iter;
        }

        // 让它退休所有 Cells
        if (pLeastLoaded)
            pLeastLoaded->retireAllCells();
    }
}
```

### grow / shrink：Cell 边界动态移动

```
grow（扩容）：
  Cell A 负载过重
    → BSP 树的分割线向邻居 Cell B 移动
    → Cell A 的空间范围缩小
    → 部分实体从 A 转移到 B
    → A 的负载降低

shrink（缩容）：
  Cell A 负载很轻
    → BSP 树的分割线向 Cell A 移动
    → Cell A 的空间范围扩大
    → 从邻居接收更多实体
    → 或者直接退休 Cell A
```

## 15.6 Offload：实体跨 Cell 迁移

当 BSP 分割线移动后，一些实体可能从原来的 Cell 落入新的 Cell。这需要实际的实体迁移。

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/offload_checker.hpp（简化）
class OffloadChecker
{
public:
    void run();   // 执行迁移检查

    void addToOffloads(EntityPtr pEntity,
        CellAppChannel* pOffloadDestination);

private:
    void sendOffloads();
    void sendOffload(OffloadList::const_iterator iOffload);

    typedef std::pair<EntityPtr, CellAppChannel*> OffloadEntry;
    typedef BW::vector<OffloadEntry> OffloadList;

    Cell& cell_;
    OffloadList offloadList_;          // 待迁移的实体列表
    uint numGhostsDeleted_;            // 已删除的 Ghost 数量
};
```

### Offload 执行流程

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/offload_checker.cpp（简化）
void OffloadChecker::run()
{
    if (cell_.space().isShuttingDown())
        return;

    // 检查每个 real entity 是否需要迁移
    Cell::Entities::iterator iEntity = cell_.realEntities().begin();
    while (iEntity != cell_.realEntities().end())
    {
        EntityPtr pEntity = *iEntity;
        EntityGhostMaintainer entityGhostMaintainer(*this, pEntity);
        entityGhostMaintainer.check();
        ++iEntity;
    }

    // 发送待迁移的实体
    this->sendOffloads();
}
```

```
Offload 完整链路：

Cell A（源 Cell）
  │
  ├── OffloadChecker::run()
  │     遍历所有 real entities
  │     检查实体当前位置是否落入其他 Cell 的范围
  │
  ├── 发现实体 E 不再属于 Cell A
  │     addToOffloads(E, targetCellChannel)
  │
  ├── sendOffloads()
  │     序列化实体 E 的完整状态到 Bundle
  │     发送到目标 CellApp
  │
  │     同时：
  │       删除 E 在 Cell A 上的 Ghost
  │       通知 E 的 Base：Cell 已迁移
  │
  └── 目标 Cell B（目标 Cell）
        收到 E 的状态
          → 在本地重建 E
          → 加入 CoordinateSystem（AOI 链表）
          → 恢复 Ghost 关系
```

## 15.7 Cell 死亡处理

### DeadCellApp：记录死亡进程

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/baseapp/dead_cell_apps.cpp（简化）
class DeadCellApp
{
    static const int MAX_AGE = 10;   // 保留 10 秒
    Mercury::Address addr_;
    uint64 timestamp_;
};
```

### DyingCellApp：恢复实体

```cpp
class DyingCellApp
{
public:
    bool tick(const Bases& bases);   // 每 tick 恢复一批实体
    void onCompleted(Mercury::NetworkInterface& intInterface);

private:
    Mercury::Address addr_;
    BW::map<SpaceID, Mercury::Address> newDestinations_;  // Space → 新 CellApp
};
```

```
CellApp 宕机恢复流程：

1. bwmachined 检测到 CellApp 进程死亡
   │
2. CellAppMgr 收到死亡通知
   │   → 标记该 CellApp 的所有 Cell 为 "dead"
   │
3. BSP 树 re-balance
   │   → 将死亡 Cell 的空间范围分配给邻居 Cell
   │
4. BaseApp 上的 DyingCellApp 开始恢复
   │   → 遍历所有 Base 实体
   │   → 检查哪些 Base 的 Cell 部分在死亡 Cell 上
   │   → 向新的目标 CellApp 发送 restoreEntity
   │
5. 新 CellApp 收到实体
   │   → 从 BaseApp 获取 Base 数据（Backup）
   │   → 重建 Cell 实体
   │   → 加入 AOI 系统
   │
6. 恢复完成
```

**注意**：Cell 宕机恢复依赖 Base 上的 Backup 数据（Ch13）。如果没有 Backup，Cell 上的实时状态会丢失——只能从 Base 的备份恢复。

## 15.8 KBEngine 的简化方案

### 没有 BSP 树

KBEngine 的 Space 非常简单：

```cpp
// 文件：kbe/src/server/cellapp/space.h
class Space : public Entity
{
    BASE_SCRIPT_HREADER(Space, Entity)
public:
    Space(ENTITY_ID id, const ScriptDefModule* pScriptModule);
    ~Space();
};
```

Space 只是一个 Entity，没有空间分割逻辑。真正的管理在 SpaceMemory 中：

```cpp
// 文件：kbe/src/server/cellapp/spacememory.h
class SpaceMemory
{
    // 每个 Space 只有一个 Cell
    Cell* pCell_;

    // 一个 CoordinateSystem 管理所有实体
    CoordinateSystem coordinateSystem_;

    // 实体列表
    SPACE_ENTITIES entities_;
};
```

**关键限制**：一个 SpaceMemory 只有一个 `Cell*` 和一个 `CoordinateSystem`。这意味着一个 Space 的所有实体都在同一个 CellApp 上。

### 没有动态负载均衡

KBEngine 没有任何自动负载均衡机制：

- 无 BSP 树 / 空间分割
- 无分割线动态调整
- 无 Offload 实体迁移
- 无 CellApp 死亡恢复

### 但有脚本驱动的跨 CellApp 迁移（Teleport）

```cpp
// Cellapp 中的 teleport 机制
void Cellapp::reqTeleportToCellApp(Network::Channel* pChannel,
    MemoryStream& s)
{
    // 手动将实体从当前 CellApp 迁移到目标 CellApp
    // 通过脚本调用触发
}
```

它更接近"业务显式迁移实体"，不是 BigWorld 那种由 BSP 边界变化自动驱动的负载均衡。

### 为什么简化？取舍分析

| 维度 | BigWorld（完整 BSP） | KBEngine（简化） |
|------|---------------------|-----------------|
| 复杂度 | 高（BSP 树 + 动态均衡 + Offload + 死亡恢复） | 低（单 Cell + 手动 teleport） |
| 自动化 | 全自动（检测→均衡→迁移→恢复） | 手动（开发者自行调度） |
| 水平扩展 | 天然支持（Space 切分成多个 Cell） | 受限（一个 Space 一个 CellApp） |
| 实体密度 | 均匀分布（负载均衡） | 可能不均 |
| 开发成本 | 高（BSP 均衡是复杂的分布式算法） | 低 |
| 运维成本 | 低（自动处理） | 高（需要人工监控和调整） |

**KBEngine 的假设**：大多数 MMO 场景不需要在一个 Space 内放数万实体。如果需要大世界，可以分成多个 Space（跨 Space 走 Base 层转发）。这种简化让引擎代码量大幅减少，但牺牲了"单 Space 内水平扩展"的能力。

## 15.9 两套项目的空间拓扑对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 空间分割 | 无（一个 Space 一个 Cell） | BSP 树（Space 切成多个 Cell） |
| 分割数据结构 | 无 | InternalNode + LeafNode |
| 负载均衡 | 无 | doBalance 递归均衡 |
| 边界移动 | 无 | grow / shrink（分割线动态调整） |
| 实体迁移 | 手动 teleport | OffloadChecker 自动迁移 |
| 扩容 | 手动创建新 Space | CellAppGroup::addCell() |
| 缩容 | 手动销毁 Space | CellAppGroup::checkForUnderloaded() |
| 死亡恢复 | 基础灾难恢复框架 | DeadCellApp + DyingCellApp |
| 实体边界 | 无 | EntityBoundLevels |
| 激进度控制 | 无 | balanceAggression_ |
| 地形块感知 | 无 | chunkBounds_（限制分割线在 chunk 边界） |
| CellApp 管理 | 无组概念 | CellAppGroup |

## 15.10 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| Space | `kbe/src/server/cellapp/space.h` |
| SpaceMemory | `kbe/src/server/cellapp/spacememory.h` |
| SpaceMemorys | `kbe/src/server/cellapp/spacememorys.h` |
| Cell | `kbe/src/server/cellapp/cell.h` |
| Teleport | `kbe/src/server/cellapp/cellapp.cpp`（reqTeleportToCellApp） |

### BigWorld

| 概念 | 文件 |
|------|------|
| BSPNode | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/bsp_node.hpp` |
| InternalNode | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/internal_node.hpp` |
| InternalNode 实现 | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/internal_node.cpp` |
| EntityBoundLevels | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/entity_bound_levels.hpp` |
| CellAppGroup | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/cell_app_group.hpp` |
| CellAppGroup 实现 | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/cell_app_group.cpp` |
| OffloadChecker | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/offload_checker.hpp` |
| OffloadChecker 实现 | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/offload_checker.cpp` |
| DeadCellApp | `BigWorld-Engine-14.4.1/programming/bigworld/server/baseapp/dead_cell_apps.cpp` |
| SpaceBranch | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/space.hpp` |

## 15.11 源码走读路径

### 路径一：理解 BSP 树的结构和分割

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/bsp_node.hpp` — BSPNode 基类，BSPNodeType 枚举
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/internal_node.hpp` — InternalNode，分割线 + 子树
3. 对比 KBEngine：`kbe/src/server/cellapp/spacememory.h` — 没有 BSP，单 Cell

### 路径二：跟踪动态负载均衡算法

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/internal_node.cpp` — `doBalance()` 负载差异计算和分割线移动
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/entity_bound_levels.hpp` — EntityBoundLevels 实体边界
3. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellappmgr/cell_app_group.cpp` — `checkForUnderloaded()` 缩容检查

### 路径三：跟踪实体迁移流程

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/offload_checker.hpp` — OffloadChecker 类声明
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/offload_checker.cpp` — `run()` 遍历实体 + `sendOffloads()` 发送迁移

## 15.12 小结

- **BSP 树是 BigWorld 空间分割的核心**：递归地把 Space 切成多个 Cell，每个 Cell 分配给一个 CellApp
- **InternalNode 管理分割线**：水平/垂直分割，左右子树，可动态调整分割线位置
- **doBalance 是负载均衡的核心算法**：计算左右子树负载差异，决定分割线移动方向和距离
- **grow / shrink 让 Cell 边界动态移动**：分割线移动后，实体管辖权随之转移
- **OffloadChecker 实现实体跨 Cell 迁移**：发现不属于当前 Cell 的实体，序列化并发送给目标 Cell
- **EntityBoundLevels 确保分割线不穿过实体**：记录实体分布，限制分割线移动范围
- **CellAppGroup 管理扩容/缩容**：addCell() 添加新 Cell，checkForUnderloaded() 移除空闲 Cell
- **KBEngine 不做动态负载均衡**：一个 Space 只有一个 Cell，简化了实现但牺牲了单 Space 水平扩展能力
- **取舍的核心**：BigWorld 面向超大世界（同 Space 数万实体），KBEngine 面向中等规模（多 Space 分治）
