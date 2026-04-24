# 9. Space、Cell、AOI 与 Witness

> 本文件是旧稿，相关内容已拆分并扩写到 [`14-space-aoi-and-vision-system.md`](/study/14-space-aoi-and-vision-system.html) 与 [`17-ghost-system.md`](/study/17-ghost-system.html)。阅读主线请以新稿为准。

> 这是 BigWorld 血统最强的一章。真正理解这一章，才会明白 KBEngine 为什么不是“场景里放个对象列表然后广播状态”。

## 11.1 本章核心问题

- Space 如何被组织成服务器内可运行的结构？
- Cell、AOI、Witness 在代码里如何协同？
- 客户端看到哪些实体，究竟是谁在决定？

## 11.2 先给结论：客户端“看到世界”不是一个动作，而是一套持续机制

很多初学者会把“进入场景”理解成：

- 把玩家放进 space
- 一次性把附近实体发给客户端
- 之后谁变了再补消息

KBEngine 不是这么做的。它更接近：

- `SpaceMemory` 维护空间运行时
- 实体通过坐标节点挂进空间坐标系统
- `ViewTrigger` 在 AOI 边界上持续触发进入 / 离开
- `Witness` 记录并整理哪些实体应进入或离开客户端视野
- `update()` 再把这些变化逐步变成客户端消息

所以“看见世界”不是一次建表动作，而是 AOI 驱动的持续同步过程。

## 11.3 `SpaceMemory` 才是真正的运行中 space

源码入口：

- `kbe/src/server/cellapp/spacememory.*`
- `kbe/src/server/cellapp/spacememorys.*`

`SpaceMemory` 的意义不是“space 的配置对象”，而是 CellApp 上真正活着的空间实例。

从 `SpaceMemory::addEntityAndEnterWorld` 可以看到进入世界的最短路径：

1. `addEntity(pEntity)`
2. `addEntityToNode(pEntity)`
3. `onEnterWorld(pEntity)`

再细看：

- `addEntity`
  会设置 `spaceID`、加入 `entities_`、调用 `pEntity->onEnterSpace(this)`
- `addEntityToNode`
  会把实体安装到坐标系统
- `_onEnterWorld`
  若是 real entity 且有 client，并且已经有 witness，则：
  - 先补 space data
  - 再 `pEntity->pWitness()->onEnterSpace(this)`

这说明“实体进入 space”至少有三层含义：

- 属于这个 space
- 进入这个 space 的坐标系统
- 若它受客户端控制，则开始建立客户端世界表现

## 11.4 为什么这里叫 `SpaceMemory` 而不是 `Space`

这点命名其实很有含义。

- `Space` 更偏脚本层实体语义
- `SpaceMemory` 更偏 CellApp 内部运行时语义

它强调的是：

- 这是内存中的、当前 CellApp 持有的、正在运行的空间

这和 BigWorld 一系的思路一致：服务器内真正重要的是空间运行态，而不是一个抽象“地图编号”。

## 11.5 real / ghost 区分说明 Cell 世界不是单副本

源码入口：

- `cellapp/entity.cpp`
- `cellapp/ghost_manager.cpp`
- `cellapp/real_entity_method.cpp`

这一章必须始终带着一个意识：

- Cell 上的实体不全都是真正执行逻辑的 real entity
- 还可能有 ghost entity

从代码里能看到很多地方都在先判断：

- `isReal()`

如果不是 real，很多操作就不能直接做，或者需要通过 `GhostManager` / `RealEntityMethod` 再转去真正拥有控制权的 Cell。

这说明 KBEngine 的空间模型从一开始就不是“所有场景实体都集中在一个进程里”，而是为：

- 跨 CellApp 迁移
- ghost 同步
- 真实控制权分布

预留了基础结构。

## 11.6 `controlledBy` 说明“谁在控制这个 cell 实体”也是一等状态

源码入口：

- `cellapp/entity.cpp`
- `cellapp/entity.inl`

Cell 实体里有一个非常关键但很容易被忽略的状态：

- `controlledBy()`

它表达的不是“这个实体属于谁”，而是：

- 当前哪个 Base 实体在控制它

这层状态对很多路径都很关键：

- 客户端到 Cell 的受控调用校验
- Witness 与 client 关系恢复
- 重连、挤号、控制权转移

这也是为什么前面讲 `onGetWitness` 时，最后一步会显式：

- `controlledBy(baseEntityCall())`

## 11.7 `Witness` 不是“可见实体列表”，而是客户端视野控制器

源码入口：

- `kbe/src/server/cellapp/witness.*`
- `kbe/src/server/cellapp/view_trigger.*`
- `kbe/src/server/cellapp/entityref.*`

如果只看成员名，很容易把 `Witness` 理解成“维护 viewEntities_ 的容器”。这太浅了。

更准确地说，`Witness` 负责三件事：

1. 为某个受客户端控制的实体维护视野触发器
2. 记录进入 / 离开视野的实体状态
3. 把这些状态变化转成客户端网络消息

所以 `Witness` 是：

- AOI 机制和客户端同步机制之间的桥

## 11.8 `ViewTrigger` 决定“谁进入了视野”

源码入口：

- `view_trigger.cpp`
- `witness.cpp`

`ViewTrigger` 本身比较薄，它的关键价值在于：

- 坐标系统判断进入 / 离开后
- 最终回调到 `Witness::onEnterView` / `Witness::onLeaveView`

所以真正的职责分工是：

- 坐标系统 + trigger
  决定几何上是否进入范围
- `Witness`
  决定这个几何事件怎样影响客户端世界状态

## 11.9 `Witness::onEnterView` 为什么是关键函数

源码入口：

- `Witness::onEnterView`

这段代码非常值得逐行读，因为它清楚地展示了“看到一个实体”在服务器里不是一句话。

它至少会做这些事：

- 若该实体原本正处于 `LEAVE_CLIENT_PENDING`
  则需要撤销离开态或改回进入态
- 若此前完全不在视野中
  则创建一个新的 `EntityRef`
- 标记 `ENTITYREF_FLAG_ENTER_CLIENT_PENDING`
- 放进 `viewEntities_` 和 `viewEntities_map_`
- 调用 `pEntity->addWitnessed(pEntity_)`
- 再触发 `onEnteredView`

这说明“进入视野”不是立即发消息，而是先把运行时状态放进一个待收束状态机里。

## 11.10 `onLeaveView` 同样不是立即删除，而是进入待离开状态

源码入口：

- `Witness::_onLeaveView`

这里最关键的一点是：

- 它不会立刻把实体 ref 从结构里删掉
- 而是标记 `LEAVE_CLIENT_PENDING`

并且：

- 从实体的 witnessed 关系中解绑
- 暂时把 `pEntity` 设为 `NULL`

这说明 KBEngine 的视野同步不是“事件来了就直接删结构”，而是：

- 先把变化记录成待同步状态
- 再由后续更新循环把它真正反映到客户端

这对于处理抖动、边界来回穿越和消息顺序问题非常重要。

## 11.11 `resetViewEntities` 说明重连恢复不是简单补发一个 enter world

源码入口：

- `Witness::resetViewEntities`

这段代码在前面重连章节已经提过，但放在这里更容易理解其真正价值。

它做的是：

- 清掉已经待离开的引用
- 把还在视野里的实体全部改成 `ENTER_CLIENT_PENDING`
- 重新计算 alias

这实际上等于告诉引擎：

- “把当前这份服务器视野状态，当作要重新完整同步给客户端的一份新基线”

所以重连恢复不是只发一个“你又在线了”，而是：

- 重新建立一整套客户端可见世界

## 11.12 `Witness::onEnterSpace` 说明进入世界是如何真正通知客户端的

源码入口：

- `Witness::onEnterSpace`

这里会明确发两类核心消息给客户端：

- `onSetEntityPosAndDir`
  强制同步位置和朝向
- `onEntityEnterSpace`
  告诉客户端进入了哪个 space

发完后再：

- `installViewTrigger()`

这说明进入世界的客户端语义是分层的：

1. 先告诉客户端“你现在在这里”
2. 再启动视野触发器，让它后续不断收到附近实体变化

## 11.13 `installViewTrigger` 里的细节体现了 AOI 系统有很多边界问题

源码入口：

- `Witness::installViewTrigger`

这段代码里的注释很有价值，它反映出 AOI 系统最难处理的不是正常路径，而是边界时序：

- 视野触发器尚未完全安装时，实体可能已经因为回调被销毁
- 需要先安装 hysteresis trigger，再安装主 view trigger
- 否则某些极端情况下进入 / 离开事件会错乱

这说明 KBEngine 的 AOI 实现不是课堂上那种“画个圈然后查范围”的简化模型，而是已经踩过很多线上边界坑的工程代码。

## 11.14 为什么要有 hysteresis 区域

从 `onEnterView` / `onLeaveView` 的分支可以看到，系统不仅有主 view trigger，还会有一个 hysteresis area trigger。

它解决的主要问题是：

- 实体在视野边界附近抖动时
- 不要频繁 enter / leave

所以这里的设计目标不是单纯“几何上精确”，而是：

- 让客户端视野变化在工程上更稳定

这也是 MMO 视野系统一个非常典型的取舍。

## 11.15 一个建议的源码走读路径

如果你要把这一章真正跟通，我建议按下面顺序：

1. `spacememory.cpp`
   看 `addEntityAndEnterWorld / addEntity / removeEntity`
2. `entity.cpp`
   看 `onEnterSpace / onLeaveSpace / controlledBy / isReal`
3. `witness.cpp`
   看 `onEnterSpace / onEnterView / onLeaveView / resetViewEntities`
4. `view_trigger.cpp`
   看它怎样把 AOI 事件回调给 Witness
5. `ghost_manager.cpp`
   再补 ghost / real 的跨 Cell 语义

跟的时候一直问自己：

- 这个实体是 real 还是 ghost？
- 它是否有客户端？
- 是否已经有 witness？
- 这个变化是几何事件，还是客户端同步事件？

## 11.16 和 BigWorld 模型的直接对应

这一章最能看出 BigWorld 血统的几个点是：

- Space 运行态在 Cell 侧
- 视野不是全局广播，而是 witness 驱动
- real / ghost 区分是空间分布式运行的基础
- 客户端控制权、空间位置、可见性是三层不同状态

也就是说，KBEngine 虽然在实现上做了自己的取舍，但这一整套 Space / Cell / Witness 思路，明显仍然站在 BigWorld 那条技术路线之上。

## 11.17 本章小结

这一章最重要的不是记住多少个类名，而是建立下面这条主线：

- `SpaceMemory` 维护空间运行态
- 实体进入 space 后挂入坐标系统
- `ViewTrigger` 产生 AOI 进入 / 离开事件
- `Witness` 把这些事件整理成客户端视野状态机
- `update()` 再把状态变化持续发给客户端
- real / ghost 与 `controlledBy` 让这套模型能够跨 CellApp 继续成立

把这条链真正看懂，后面你再读 teleport、ghost 同步、客户端可见性优化时，整个模型都会顺得多。
