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

<a id="proxy-give-client-to"></a>
## `Proxy.giveClientTo()` 转移的其实是客户端绑定与 Witness 链路

`giveClientTo()` 如果只看名字，很容易被理解成“把客户端控制权字段改到另一个 Proxy”。

源码实际做的事情比这大得多，它切换的是一整条链路：

```text
旧 Proxy
  ├── clientEntityCall / Network::Channel
  ├── Client 上的旧代理实体
  └── Cell 上的 Witness / controlledBy

            ↓ giveClientTo()

新 Proxy
  ├── 接手同一条客户端通道
  ├── 重新向客户端创建新的代理实体
  └── 若已有 cell，再重建 Witness 与控制关系
```

### 第一层：Base 层会先做一轮严格前置校验

直接看 `kbe/src/server/baseapp/proxy.cpp`：

```cpp
// 文件：kbe/src/server/baseapp/proxy.cpp
void Proxy::giveClientTo(Proxy* proxy)
{
    if(isDestroyed()) ...
    if(clientEntityCall_ == NULL || clientEntityCall_->getChannel() == NULL) ...

    if(proxy)
    {
        if(proxy->isDestroyed()) ...
        if(proxy->id() == this->id()) ...
        if(proxy->clientEntityCall() != NULL) ...
        ...
    }
}
```

这里能直接得出几个实际限制：

- 当前 Proxy 必须活着，而且已经绑定客户端通道
- 目标 Proxy 不能销毁、不能是自己、不能已经绑定客户端
- Python 包装层虽然允许把 `None` 传进来，但 `giveClientTo(NULL)` 不会继续执行任何迁移逻辑，实际使用必须传有效目标 Proxy

### 第二层：旧 Proxy 会先拆掉旧的客户端表现链

真正进入迁移分支后，旧 Proxy 首先做的是“拆链”：

```cpp
// 文件：kbe/src/server/baseapp/proxy.cpp
if(cellEntityCall())
{
    (*pBundle).newMessage(CellappInterface::onLoseWitness);
    (*pBundle) << this->id();
    sendToCellapp(pBundle);
}

(*pBundle).newMessage(ClientInterface::onEntityDestroyed);
(*pBundle) << this->id();
sendToClient(ClientInterface::onEntityDestroyed, pBundle);
```

这两步非常关键：

- 如果旧 Proxy 已经有 cell，会先通知 Cell 侧丢失 Witness
- 客户端会先收到 `onEntityDestroyed`，把旧的受控代理实体销毁

也就是说，`giveClientTo()` 不是“让客户端同时拥有两个主 Proxy”，而是明确先清掉旧链路，再接新链路。

### 第三层：客户端通道本身会被交给新 Proxy

后续还是在 `proxy.cpp`：

```cpp
// 文件：kbe/src/server/baseapp/proxy.cpp
Network::Channel* lpChannel = clientEntityCall_->getChannel();

clientEnabled_ = false;
clientEntityCall()->addr(Network::Address::NONE);
Py_DECREF(clientEntityCall());
proxy->setClientType(this->getClientType());
proxy->setLoginDatas(this->getLoginDatas());
this->setClientType(UNKNOWN_CLIENT_COMPONENT_TYPE);
this->setLoginDatas("");
clientEntityCall(NULL);
proxy->onGiveClientTo(lpChannel);
```

这里转移的不是抽象概念，而是同一条真实客户端通道 `lpChannel`。

同时还能看到：

- 客户端类型和登录附带数据会一起迁移到目标 Proxy
- 旧 Proxy 会失去 `clientEntityCall`
- 目标 Proxy 通过 `onGiveClientTo(lpChannel)` 接手这条通道

### 第四层：新 Proxy 会重新创建客户端代理实体

`onGiveClientTo()` 的实现很短，但非常关键：

```cpp
// 文件：kbe/src/server/baseapp/proxy.cpp
void Proxy::onGiveClientTo(Network::Channel* lpChannel)
{
    clientEntityCall(new EntityCall(this->pScriptModule_,
        &lpChannel->addr(), 0, id_, ENTITYCALL_TYPE_CLIENT));

    addr(lpChannel->addr());
    Baseapp::getSingleton().createClientProxies(this);
    onGetWitness();
}
```

而 `createClientProxies()` 会重新把当前 Proxy 注册为该通道的代理实体，并重新下发 `onCreatedProxies`：

```cpp
// 文件：kbe/src/server/baseapp/baseapp.cpp
bool Baseapp::createClientProxies(Proxy* pEntity, bool reload)
{
    Network::Channel* pChannel = pEntity->clientEntityCall()->getChannel();
    pChannel->proxyID(pEntity->id());
    ...
    (*pBundle).newMessage(ClientInterface::onCreatedProxies);
    (*pBundle) << pEntity->rndUUID();
    (*pBundle) << pEntity->id();
    (*pBundle) << pEntity->ob_type->tp_name;
    pEntity->sendToClient(ClientInterface::onCreatedProxies, pBundle);

    pEntity->onClientEnabled();
}
```

所以 `giveClientTo()` 成功后，目标 Proxy 一侧通常会发生两件可见事情：

- 客户端重新收到一条 `onCreatedProxies`
- 目标 Proxy 的 `onClientEnabled()` 也会再次触发

### 第五层：如果目标 Proxy 已经有 cell，Cell 侧还会补一轮 Witness 与控制恢复

`Proxy::onGetWitness()` 并不是无条件成功，它只在目标 Proxy 已经有 `cellEntityCall()` 时继续往 Cell 侧发消息：

```cpp
// 文件：kbe/src/server/baseapp/proxy.cpp
void Proxy::onGetWitness()
{
    if(cellEntityCall())
    {
        (*pBundle).newMessage(CellappInterface::onGetWitnessFromBase);
        (*pBundle) << this->id();
        sendToCellapp(pBundle);
    }
}
```

Cell 侧接到 `onGetWitnessFromBase` 后，会走到 `Entity::onGetWitness(true)`：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
void Entity::onGetWitness(bool fromBase)
{
    ...
    if(fromBase)
    {
        if(clientEntityCall() == NULL)
        {
            PyObject* clientMB = PyObject_GetAttrString(baseEntityCall(), "client");
            ...
            clientEntityCall(client);
        }

        ... // 向客户端补发 spaceID 与客户端属性

        if(pWitness_ == NULL)
            setWitness(Witness::createPoolObject(OBJECTPOOL_POINT));
        else
            pWitness_->resetViewEntities();
    }

    controlledBy(baseEntityCall());
}
```

这里说明 `giveClientTo()` 成功后，若目标已经有 cell：

- Cell 会重新把 `clientEntityCall` 接回来
- 会向客户端补发当前空间与客户端属性
- 会新建或复用 Witness，并重置视野同步状态
- 最后把 `controlledBy` 重新绑到目标 Proxy 自己的 `baseEntityCall`

### 结论：应该怎样准确理解 `giveClientTo()`

最准确的说法是：

- **它把一个客户端连接从旧 Proxy 迁移到新 Proxy**
- **迁移过程同时会拆旧的客户端实体/Witness 链路，再重建新的 Base/Cell 同步链路**
- **如果目标 Proxy 还没有 cell，这次迁移先完成的是 Base 侧客户端绑定；Cell 侧视野与控制恢复要等目标后续拿到 cell**

所以它不是“把两个 Proxy 的业务状态合并”，也不是“简单改一下控制者字段”，而是一个完整的客户端归属切换入口。

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

<a id="cell-entity-move-controllers"></a>
## Cell 实体的 `moveToPoint()` / `navigate()` / `cancelController()` 共享同一套控制器链

CellApp API 里的移动接口很容易被看成三个彼此独立的方法：

- `moveToPoint()` 负责直线移动
- `navigate()` 负责寻路移动
- `cancelController()` 负责删某个控制器

源码实际把它们串成了一条统一控制链。

### 第一层：移动前都会先执行 `stopMove()`，所以同一时刻只有一条移动链

入口在：

- `kbe/src/server/cellapp/entity.cpp`

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
bool Entity::stopMove()
{
    if(pMoveController_)
    {
        cancelController(pMoveController_->id());
        pMoveController_->destroy();
        pMoveController_.reset();
    }

    if(pTurnController_)
    {
        cancelController(pTurnController_->id());
        pTurnController_->destroy();
        pTurnController_.reset();
    }
}
```

而 `moveToPoint()` / `navigate()` 都先调用它：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
uint32 Entity::moveToPoint(...)
{
    stopMove();
    ...
}

uint32 Entity::navigate(...)
{
    ...
    stopMove();
    ...
}
```

这说明：

- 新的移动请求会先清掉当前移动控制器
- 同时也会清掉当前转向控制器
- 这不是“允许多个 Movement controller 并存，再按优先级调度”的模型

### 第二层：`moveToPoint()` 和 `navigate()` 共享同一个 `MoveController`

`moveToPoint()` 的创建链：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
KBEShared_ptr<Controller> p(new MoveController(this, NULL));
new MoveToPointHandler(p, layer(), destination, velocity, distance, faceMovement, moveVertically, userData);
...
pMoveController_ = p;
```

`navigate()` 的创建链：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
KBEShared_ptr<Controller> p(new MoveController(this, NULL));
new NavigateHandler(p, destination, velocity, distance, faceMovement, maxMoveDistance, paths_ptr, userData);
...
pMoveController_ = p;
```

`MoveController` 本身再决定挂的是哪种 handler：

```cpp
// 文件：kbe/src/server/cellapp/move_controller.cpp
if(utype == MoveToPointHandler::MOVE_TYPE_NAV)
    pMoveToPointHandler_ = new NavigateHandler();
else if(utype == MoveToPointHandler::MOVE_TYPE_ENTITY)
    pMoveToPointHandler_ = new MoveToEntityHandler();
else if(utype == MoveToPointHandler::MOVE_TYPE_POINT)
    pMoveToPointHandler_ = new MoveToPointHandler();
```

所以更准确的理解是：

- `MoveController` 是统一的“移动控制器壳”
- `moveToPoint()` / `navigate()` / `moveToEntity()` 的区别，主要落在内部 handler 类型不同

### 第三层：`navigate()` 不是直接开跑，而是先算路径，算不出来就返回 `0`

`navigate()` 先走 `navigatePathPoints()`：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
VECTOR_POS3D_PTR paths_ptr(new std::vector<Position3D>());
navigatePathPoints(*paths_ptr, destination, maxSearchDistance, layer);
if (paths_ptr->size() <= 0)
{
    return 0;
}
```

而 `navigatePathPoints()` 会去当前空间的导航句柄上算路径：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
NavigationHandlePtr pNavHandle = pSpace->pNavHandle();
...
if (pNavHandle->findStraightPath(layer, position_, destination, outPaths) < 0)
{
    return false;
}
```

这说明 `navigate()` 的真实边界是：

- 当前实体必须已经有有效 `space`
- 该空间必须已经挂上导航句柄
- 路径点为空时，它不会创建任何控制器，而是直接返回 `0`

### 第四层：`NavigateHandler` 本质上是“多段 MoveToPoint”

`NavigateHandler` 直接继承 `MoveToPointHandler`：

```cpp
// 文件：kbe/src/server/cellapp/navigate_handler.cpp
NavigateHandler::NavigateHandler(...)
    : MoveToPointHandler(...),
      destPosIdx_(0),
      paths_(paths_ptr)
{
    destPos_ = (*paths_)[destPosIdx_++];
}
```

走到一个路径点后，它不会立刻结束，而是切到下一个路径点：

```cpp
// 文件：kbe/src/server/cellapp/navigate_handler.cpp
bool NavigateHandler::requestMoveOver(const Position3D& oldPos)
{
    if(destPosIdx_ == ((int)paths_->size()))
        return MoveToPointHandler::requestMoveOver(oldPos);
    else
        destPos_ = (*paths_)[destPosIdx_++];
}
```

所以 `navigate()` 更准确地说是：

- 先算一组路径点
- 再让同一套移动控制器沿这些路径点逐段推进

它不是另一套完全独立于 `moveToPoint()` 的运动系统。

### 第五层：`cancelController()` 对移动控制器有一层特殊语义

脚本入口里有一段很关键的特殊处理：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
if(PyUnicode_Check(pyargobj))
{
    if (strcmp(PyUnicode_AsUTF8AndSize(pyargobj, NULL), "Movement") == 0)
    {
        pobj->stopMove();
    }
}
...
if ((pobj->pMoveController_ && pobj->pMoveController_->id() == id) ||
    (pobj->pTurnController_ && pobj->pTurnController_->id() == id))
{
    pobj->stopMove();
}
else
{
    pobj->cancelController(id);
}
```

这说明：

- 传 `"Movement"` 时，不是删某一个对象，而是直接停整条当前移动/转向链
- 就算你传的是当前 `pMoveController_` 或 `pTurnController_` 的数值 ID，底层也会统一走 `stopMove()`
- 只有其它控制器（例如 proximity 一类）才会走通用的 `pControllers_->remove(id)`

所以 API 文档里如果只写“按 ID 删除控制器”，是不够准确的。

### 第六层：移动控制器本身也参与实体迁移/恢复

Cell 实体在序列化时会把控制器一并写入流：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
void Entity::addControllersToStream(KBEngine::MemoryStream& s)
{
    if(pControllers_)
    {
        // 必须先清理移动相关的Controllers
        stopMove();
        pControllers_->addToStream(s);
    }
}
```

恢复时再重建：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
void Entity::createControllersFromStream(KBEngine::MemoryStream& s)
{
    ...
    pControllers_->createFromStream(s);
}
```

这说明移动/控制器语义不是一次性的脚本辅助对象，而是 Cell 实体运行时的一部分状态。

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
