# 附录 B 关键算法速查

> 本章提供书中涉及的关键算法的独立参考，方便查阅。每个算法给出问题定义、核心思路、伪代码和复杂度分析。

---

## B.1 空间索引：十字链表的插入/删除/查询

### 问题

在 2D 平面上维护一组有序实体，高效支持：
- 插入实体
- 删除实体
- 查询 [x₁, x₂] × [z₁, z₂] 矩形内的所有实体

### 核心思路

两条独立的有序链表（X 轴 + Z 轴），每个实体在两条链中各有一个节点。

```
X轴链: ... ↔ NodeA ↔ NodeB ↔ NodeC ↔ ...
Z轴链: ... ↔ NodeC ↔ NodeA ↔ NodeB ↔ ...
```

### 插入

```
insert(entity):
    // X 轴插入
    xNode = new CoordinateNode(entity, X_AXIS)
    xPrev = findInsertPos(xList, entity.x)
    xNode.insertAfter(xPrev)

    // Z 轴插入同理
    zNode = new CoordinateNode(entity, Z_AXIS)
    zPrev = findInsertPos(zList, entity.z)
    zNode.insertAfter(zPrev)
```

### 删除

```
remove(entity):
    entity.xNode.remove()
    entity.zNode.remove()
```

### 查询矩形范围

```
query(x1, x2, z1, z2):
    // 用 RangeTrigger 在两条链上各设两个锚点
    // 从锚点向前/向后遍历，收集范围内的实体
    // 取两个轴的交集
```

### 复杂度

| 操作 | 时间复杂度 | 说明 |
|------|-----------|------|
| 插入 | O(n) | 需遍历找到插入位置 |
| 删除 | O(1) | 已知节点，直接摘除 |
| 范围查询 | O(k) | k 为结果集大小，按需遍历 |

**为什么不用 O(log n) 的平衡树**：游戏实体频繁移动，树结构的旋转/平衡开销远超链表。链表对局部移动（小步更新）特别高效——只需调整前后指针。

---

## B.2 AOI 触发器：RangeTrigger 的进入/离开判定

### 问题

当一个实体移动时，如何高效判定哪些其他实体进入了/离开了它的视野范围？

### 核心思路

每个实体在 X 轴和 Z 轴各设两个**锚点**（正半径 + 负半径），当其他实体的节点越过锚点时，触发进入/离开事件。

```
                 Entity A (pos: 5, 0)
                 ├── negativeX anchor (pos: 0)  ←── range = 5
                 └── positiveX anchor (pos: 10) ←── range = 5

X轴链: ...[negA]...[B]...[EntityA]...[C]...[posA]...
                              ↑移动→
```

### 进入判定

```
当实体 A 移动，其锚点也跟着移动：
  → positiveX 锚点越过实体 B → B 进入 A 的范围（onEnter）
  → negativeX 锚点越过实体 C → C 离开 A 的范围（onLeave）
```

### 双轴联合判定

```
onEnterXRange(entity):
    if (entity 也在 Z 轴范围内):
        触发 onEnterAoI(entity)

onLeaveXRange(entity) 或 onLeaveZRange(entity):
    触发 onLeaveAoI(entity)
```

### 复杂度

| 事件 | 时间复杂度 | 说明 |
|------|-----------|------|
| 实体移动 | O(m) | m = 视野内实体数量变化量 |
| 全量扫描 | O(n) | 仅初始建立时 |

---

## B.3 视野防抖：Hysteresis 区域算法

### 问题

实体在视野边界附近来回移动时，会频繁触发进入/离开事件，导致客户端反复创建/销毁实体。如何防抖？

### 核心思路

设置两个半径：**内半径（view）** 和 **外半径（view + hysteresis）**。

```
                 ┌───────────────────────────┐ 外半径 (leave threshold)
                 │    Hysteresis 区域         │
                 │  ┌─────────────────────┐  │
                 │  │  内半径 (enter thr)  │  │
                 │  │                     │  │
                 │  │       Entity        │  │
                 │  │                     │  │
                 │  └─────────────────────┘  │
                 └───────────────────────────┘
```

### 规则

```
当其他实体：
  → 从外进入内半径 → onEnterView（确认进入）
  → 从内越过外半径 → onLeaveView（确认离开）
  → 在 Hysteresis 区域内移动 → 不触发任何事件
```

### 效果

实体在边界附近的微小移动不会触发事件。只有明确离开外半径才会被判定为"离开视野"。

---

## B.4 空间拓扑：BSP 树的分割与平衡（BigWorld）

### 问题

一个大型 Space 可能横跨多个 CellApp，如何确定每个 CellApp 负责哪个子空间？

### 核心思路

用 BSP（Binary Space Partitioning）树递归分割空间：

```
                    Space (1000×1000)
                    ┌───────────────┐
                    │               │
         Horizontal│     Left      │ Right
        ───────────┤    (500×1000)  │(500×1000)
                    │               │
                    └───────────────┘
```

### BSP 节点类型

```cpp
enum BSPNodeType {
    BSP_NODE_HORIZONTAL,   // 水平分割线
    BSP_NODE_VERTICAL,     // 垂直分割线
    BSP_NODE_LEAF          // 叶子节点（对应一个 Cell）
};
```

### 平衡算法

```
doBalance():
    1. 收集所有 Leaf 的负载（从 EntityProfiler 汇总）
    2. 找到负载最高和最低的相邻 Leaf
    3. 移动分割线（grow/shrink）
       → 负载高的 Leaf 让出空间给负载低的 Leaf
    4. 迁移边界上的实体到新 Leaf
    5. 更新 BSP 树的分割线坐标
```

---

## B.5 动态均衡：InternalNode::doBalance 的 grow/shrink

### 问题

Cell 之间负载不均衡时，如何动态调整 Cell 边界？

### 核心思路

```
grow(direction):
    // 本 Cell 向 direction 方向扩展
    // 吸收邻居 Cell 的部分空间
    // 迁移该空间内的实体到本 Cell

shrink(direction):
    // 本 Cell 向 direction 方向收缩
    // 让出空间给邻居 Cell
    // 迁移该空间内的实体到邻居 Cell
```

### 触发条件

```
当 Cell 负载 > metaLoadBalanceTolerance 时触发
当 Cell 负载 < metaLoadBalanceTolerance / 2 时也触发（反向均衡）
```

---

## B.6 寻路：NavMesh + A* 变体

### 问题

在世界空间中找到从 A 点到 B 点的最短可行路径。

### NavMesh（导航网格）

将可行走区域划分为多边形网格：

```
┌────────┬────────┐
│ Poly 1 │ Poly 2 │
│   ●A   ├────────┤
│        │ Poly 3 │
├────────┤   ●B   │
│ Poly 4 │        │
└────────┴────────┘
```

### A* 在 NavMesh 上的变体

```
1. 找到起点所在的多边形 → 终点所在的多边形
2. 构建邻接图（共享边的多边形互为邻居）
3. A* 搜索：
   - 节点 = 多边形
   - 边 = 共享边的中点（或最优点）
   - 代价 = 中点间距离
   - 启发式 = 到终点的直线距离
4. 路径平滑：去除不必要的拐点
```

### 复杂度

| 步骤 | 复杂度 |
|------|--------|
| 定位多边形 | O(log n)（加速结构） |
| A* 搜索 | O(n log n)（n = 多边形数） |
| 路径平滑 | O(k)（k = 原始路径节点数） |

---

## B.7 负载均衡：Rendezvous Hash（BigWorld）

### 问题

如何将实体均匀分配到 CellApp，且在 CellApp 数量变化时最小化迁移量？

### 核心思路

```
hash(entityKey, cellAppID) → score
为每个实体选择 score 最高的 CellApp
```

当新增/移除 CellApp 时，只有 hash 值受影响的实体会被迁移。

### KBEngine 的简化方案

KBEngine 不使用 Rendezvous Hash，而是用简单的轮询或负载查询分配。

---

## B.8 实体 ID 分配：区间分配与 componentID 编码

### KBEngine 区间分配

```
DBMgr 启动时：
    componentIDStart_ = 高位 * BASE_APP_ID_BEGIN
    每个 BaseApp 启动时申请一个 entityID 区间
    区间用完后向 DBMgr 申请新区间
```

### componentID 编码

```
componentID = (高位标志) + (组件类型编码) + (序号)
通过 componentID 可判断：
    - 组件类型（BaseApp / CellApp / ...）
    - 同类型组件的序号
```

---

## B.9 消息压缩：alias 分配算法

### 问题

属性同步时，完整的 entityID (4 字节) 和 propertyUtype (2 字节) 占用大量带宽。如何压缩？

### alias 机制

```
当实体首次进入客户端视野时：
  1. 分配 alias = 递增短 ID (1-2 字节)
  2. 发送 onEntityEnterWorld(entityID, alias, ...)

后续同步中：
  → 用 alias 代替 entityID
  → 用 PropertyDescrAlias 代替 propertyUtype

alias 复用：实体离开视野后 alias 可回收
```

### 带宽节省估算

```
每条属性同步消息节省：
  entityID: 4 字节 → 1-2 字节 (alias)
  propertyUtype: 2 字节 → 0 字节 (如果有 alias 则隐含在消息 ID 中)

对于 1000 个实体 × 10 属性 × 10Hz = 100,000 条/秒
每条省 4 字节 = 400 KB/s → 3.2 Mbps 带宽节省
```

---

## B.10 对象池：世代号复用（BigWorld Handle\<T\>）

### 问题

频繁创建/销毁对象（如 MemoryStream、Packet）导致内存碎片和 GC 压力。

### 核心思路

```
Handle<T>:
    - index: 对象在池中的位置
    - generation: 世代号（每次该位置被复用时递增）

PackedObjectPool:
    - objects_: 对象数组
    - generations_: 世代号数组
    - freeList_: 空闲位置链表

allocate():
    index = freeList_.pop()
    generations_[index]++
    return Handle<T>(index, generations_[index])

dereference(handle):
    if (generations_[handle.index] == handle.generation):
        return objects_[handle.index]   // 有效
    else:
        return null                      // 已被复用，Handle 失效
```

### 安全性

世代号机制防止**悬垂引用**：即使旧 Handle 指向的位置已被新对象复用，世代号不匹配就会检测到。

---

## B.11 属性同步：detailLevel 分级过滤

### 问题

不是所有属性都需要实时同步给所有客户端。远处实体只需同步位置，近处才需要同步完整属性。

### 核心思路

```
detailLevel 分级：
  - DETAIL_LEVEL_LOW:   仅位置 + 朝向（远处）
  - DETAIL_LEVEL_MEDIUM: + 关键属性（中距离）
  - DETAIL_LEVEL_HIGH:  全部属性（近处）

属性定义中标注 detailLevel：
  <Property name="HP" detailLevel="MEDIUM"/>
  <Property name="position" detailLevel="LOW"/>
  <Property name="inventory" detailLevel="HIGH"/>
```

### 过滤逻辑

```
Witness update():
    for entity in visibleEntities:
        distance = calcDistance(thisEntity, entity)
        if (distance < nearRadius):
            syncLevel = HIGH
        elif (distance < midRadius):
            syncLevel = MEDIUM
        else:
            syncLevel = LOW

        for prop in entity.dirtyProperties:
            if (prop.detailLevel <= syncLevel):
                addToBundle(prop)
```

---

## B.12 广播优化：Bundle payload 复用

### 问题

一个实体的属性变更需要同步给 N 个客户端。如果每次都独立构造 Bundle，序列化开销 × N。

### 核心思路

```
一次序列化，多次发送：

1. 构造 payload（公共部分）：
   Bundle payload
     → entityID alias slot (占位，每个客户端不同)
     → 属性数据（所有客户端相同）

2. 对每个客户端：
   Bundle finalBundle
     → header: 消息 ID + 该客户端的 alias
     → payload: 复用公共部分（memcpy）

3. 每个 finalBundle 通过对应 Channel 发送
```

### 效果

```
N 个客户端 × M 个属性：
  - 无优化：N × M 次序列化
  - 有优化：1 次序列化 + N 次 memcpy

对于 100 个客户端看到同一实体：
  序列化开销降低 ~99%
```
