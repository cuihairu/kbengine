# 空间、AOI 与视野同步

> 这一页聚焦 KBEngine 里最像 BigWorld 的那部分实现：空间权威放在 Cell，客户端可见集由 Witness 决定，跨 Cell 边界靠 ghost 与临时路由把链路缝起来。

## 先建立空间层的骨架

源码里这部分不是一个单类系统，而是几个对象协作：

```text
SpaceMemory
  ├── CoordinateSystem
  ├── Entity
  ├── Witness
  ├── ViewTrigger / RangeTrigger
  └── GhostManager
```

如果只记一个结论，就是：

- `SpaceMemory` 管空间运行态
- `CoordinateSystem` 管空间内节点索引
- `Witness` 管“谁该被客户端看到”
- `GhostManager` 管跨 Cell 的 real / ghost 同步与迁移后路由

## Space 在 KBEngine 里不是抽象概念，而是 Cell 侧实体 + 运行态容器

最容易误解的一点是：`Space` 和 `SpaceMemory` 不是一个东西。

源码里可以看到：

- `kbe/src/server/cellapp/space.h`
- `kbe/src/server/cellapp/spacememory.h`

`Space` 本身继承自 `Entity`，它是脚本可见的空间实体；
而 `SpaceMemory` 才是运行时容器，内部真正持有：

- `CoordinateSystem`
- 空间内实体集合
- 空间几何与地图映射

因此在阅读源码时要区分：

- `Space` 更偏实体语义
- `SpaceMemory` 更偏运行态语义

## 空间创建的真实入口在 Cellapp

最值得直接跟的函数是：

- `kbe/src/server/cellapp/cellapp.cpp`
- `Cellapp::onCreateCellEntityInNewSpaceFromBaseapp`

这条链能把“Base 请求进空间”一次性看清：

1. 从消息流里读出 `entityType / entityID / spaceID / componentID / hasClient`
2. 调 `SpaceMemorys::createNewSpace(spaceID, entityType)`
3. 在该 Cell 上创建对应 `Entity`
4. 解析 Cell 数据流 `createCellDataFromStream`
5. 建立 `baseEntityCall`
6. 如果该实体有客户端，则提前准备 `clientEntityCall` 与 `Witness`
7. `space->addEntity(e)`
8. `e->initializeEntity(cellData, true)`
9. `space->addEntityToNode(e)`
10. 如果有客户端，调用 `e->onGetWitness()`

这条路径说明两个关键点：

- 空间不是先存在一个纯容器，再把实体挂进去；空间实体和空间运行态是联动建立的。
- 有客户端的实体在进入空间时，会提前补齐 Witness 相关结构，确保后续同步链能立即工作。

## CoordinateSystem：AOI 的底层索引不是网格，而是坐标轴链表

最关键的文件：

- `kbe/src/server/cellapp/coordinate_system.h`
- `kbe/src/server/cellapp/coordinate_system.cpp`
- `kbe/src/server/cellapp/coordinate_node.h`

从接口能看出它的设计非常明确：

- `insert()`
- `remove()`
- `update()`
- `moveNodeX() / moveNodeY() / moveNodeZ()`

这说明 KBEngine 的空间索引不是简单哈希桶，而是基于坐标轴排序节点的结构。

与 `RangeTrigger` 搭配使用时，这种结构的优势是：

- 实体移动时只需要在局部调整坐标节点
- 触发器边界经过节点时，可以直接判定“进入 / 离开范围”

所以 AOI 不是一个独立的大模块，而是 `CoordinateNode + CoordinateSystem + RangeTrigger` 的协作结果。

## RangeTrigger 决定“经过边界时发生什么”

关键文件：

- `kbe/src/server/cellapp/range_trigger.h`
- `kbe/src/server/cellapp/range_trigger.cpp`
- `kbe/src/server/cellapp/view_trigger.h`

`RangeTrigger` 做的是最底层的几何判定：

- 边界节点安装
- 节点穿越边界时的进入/离开判定
- X/Y/Z 三轴上的范围检测

你在源码里能看到它并不是每帧暴力扫描，而是利用边界节点与实体节点的相对移动来触发回调。

因此 AOI 触发的本质不是“定期计算一遍可见集”，而是“坐标更新时驱动边界事件”。

## Witness：客户端看到什么，由它决定

最该先读的文件：

- `kbe/src/server/cellapp/witness.h`
- `kbe/src/server/cellapp/witness.cpp`

`Witness` 内部有几组核心状态：

- `viewRadius_`
- `viewHysteresisArea_`
- `pViewTrigger_`
- `pViewHysteresisAreaTrigger_`
- `viewEntities_map_`

这说明它不是“简单可见集容器”，而是完整的观察者状态机。

从接口上看，最值得跟的是：

- `attach()`
- `onEnterSpace()`
- `onEnterView()`
- `onLeaveView()`
- `update()`
- `sendToClient()`

其中 `Witness::update()` 才是“把这一帧需要同步给客户端的东西真正发出去”的总入口。

所以：

- AOI 判定负责知道谁进入/离开范围
- Witness 负责把这些变化变成客户端协议消息和属性更新

## `onGetWitness()` 是空间同步真正开始的节点

在 `kbe/src/server/cellapp/entity.cpp` 中，`Entity::onGetWitness()` 是一个非常关键的转折点。

它不是单纯脚本钩子，而是：

- 创建或激活 Witness
- 让实体开始进入“可被客户端同步”的状态
- 再向脚本层触发 `onGetWitness`

因此“实体有客户端”和“实体已经开始同步给客户端”不是完全同一时刻；
真正切入客户端可见链的是 `onGetWitness()`。

## 视野同步不是一次性快照，而是持续更新

这也是很多人第一次读会误会的地方。

`Witness` 的职责不是“创建时把周围实体发一遍就结束”，而是持续维护：

- 当前 view 内有哪些实体
- 各实体当前 detailLevel
- 哪些属性发生过变化
- 哪些消息该在这一帧推给客户端

所以视野同步是长期运行的增量机制，而不是“进入场景时的一次初始化包”。

## ghost：跨 Cell 边界时不能直接断链，所以需要副本

关键文件：

- `kbe/src/server/cellapp/ghost_manager.h`
- `kbe/src/server/cellapp/ghost_manager.cpp`
- `kbe/src/server/cellapp/entity.cpp`

从注释和接口看，`GhostManager` 负责两类事：

- real 向 ghost 的同步
- 实体迁移期间的临时消息路由

核心成员非常说明问题：

- `realEntities_`
- `ghost_route_`
- `messages_`

这意味着 ghost 不只是“远端副本”，还是跨 Cell 迁移时保持消息连续性的缓冲层。

## real / ghost 关系在 Cell 侧是显式状态

在 `kbe/src/server/cellapp/entity.h` / `entity.cpp` 里可以直接看到：

- `ghostCell_`
- `isReal()`
- `changeToGhost()`
- `changeToReal()`
- `onUpdateGhostPropertys()`

从实现语义上看：

- real 是当前权威实体
- ghost 是边界或迁移期间的副本
- 属性变更会由 real 通过 `CellappInterface::onUpdateGhostPropertys` 推给 ghost

所以 ghost 同步并不是“数据库复制”或“状态恢复”，而是空间权威在分布式 Cell 之间的在线复制。

## GhostManager 的第二个职责：迁移后路由缝合

`ghost_manager.h` 里的注释其实讲得很直白：

- 某个实体迁移走后，本机短时间内可能还会收到发往旧地址的消息
- 这时 `ghost_route_` 记录一个临时转发表
- 收到包后继续转发到新的 real 或 ghost 所在 Cell

这非常关键，因为它说明：

- ghost 系统不仅是“可见性副本”
- 还是“迁移期间的消息连续性机制”

没有这层路由，跨 Cell 传送时就很容易出现旧消息直接丢失、链路断裂的情况。

## 谁决定客户端该看到哪些实体

精确回答这个问题：

- 空间索引层：`CoordinateSystem + RangeTrigger`
- 观察者层：`Witness`
- 最终发送层：`Witness::update()` / `sendToClient()`

所以不是 `SpaceMemory` 单独决定，也不是实体自己决定。

真正的职责分工是：

- `SpaceMemory` 提供空间运行态与节点容器
- `RangeTrigger` 产生“进入 / 离开”事件
- `Witness` 维护每个客户端的可见集和同步状态

## 读源码的最短路径

如果你准备在 IDE 里读一遍空间/AOI 主线，建议顺序是：

1. `kbe/src/server/cellapp/cellapp.cpp` → `onCreateCellEntityInNewSpaceFromBaseapp`
2. `kbe/src/server/cellapp/spacememory.h` / `spacememory.cpp`
3. `kbe/src/server/cellapp/coordinate_system.cpp`
4. `kbe/src/server/cellapp/range_trigger.cpp`
5. `kbe/src/server/cellapp/witness.cpp`
6. `kbe/src/server/cellapp/entity.cpp` → `onGetWitness / onLoseWitness`
7. `kbe/src/server/cellapp/ghost_manager.cpp`
8. `kbe/src/server/cellapp/entity.cpp` → `onUpdateGhostPropertys / changeToGhost / changeToReal`

这样读可以从“实体进空间”一直走到“客户端看到世界”和“跨 Cell 不断链”。

## 与主线章节的关系

这页的作用是给源码阅读做地图。

如果你要看完整叙事版，请回到：

- `/study/14-space-aoi-and-vision-system.html`
- `/study/15-space-topology-and-dynamic-scaling.html`
- `/study/17-ghost-system.html`

主线负责讲机制，这一页负责告诉你这些机制在源码里分别落在哪。 
