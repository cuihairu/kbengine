# 网络与消息系统

> 这一页聚焦源码里的真正网络骨架：事件循环怎样驱动所有连接，`Channel` 怎样持有收发状态，`Bundle` 怎样把“消息”落成“包”，以及一条消息最终怎样找到处理函数。

## 先建立一个分层图

KBEngine 的网络层不是一个“大 socket 类”，而是几层配合：

```text
EventDispatcher
  └── EventPoller / Timer / Task
        └── NetworkInterface
              └── Channel
                    ├── PacketReader / PacketSender
                    ├── Bundle
                    └── MessageHandlers
```

这几层分别解决不同问题：

- `EventDispatcher` 负责主循环
- `NetworkInterface` 负责维护一组通道
- `Channel` 负责一条连接的收发状态
- `Bundle` 负责把多条逻辑消息编码进 packet 流
- `MessageHandlers` 负责 `msgID -> handler` 映射

## 第一层：EventDispatcher 是总驱动器

最基础的入口是：

- `kbe/src/lib/network/event_dispatcher.h`

从接口就能看出它的职责不是“只跑网络”，而是统一调度：

- `processOnce()`
- `processUntilBreak()`
- `addTimer()`
- `addTask()`
- `registerReadFileDescriptor()`
- `registerWriteFileDescriptor()`

这意味着 KBEngine 的主线程模型本质上是：

- 文件描述符事件
- 定时器
- 延迟任务

三者统一挂在一个事件循环里，而不是“网络线程 + 逻辑线程 + 定时器线程”三套独立框架。

因此游戏逻辑、网络回调、定时 tick 在主线程上能天然保持串行语义，这也是实体侧大量代码不依赖锁的前提。

## 第二层：NetworkInterface 管的不是一个连接，而是一组连接

核心文件：

- `kbe/src/lib/network/network_interface.h`

`NetworkInterface` 最值得记住的成员和方法：

- `channelMap_`
- `registerChannel()`
- `deregisterChannel()`
- `findChannel()`
- `processChannels()`
- `sendIfDelayed()` / `delayedSend()`

它的角色更接近“某个组件进程的连接总线”。

比如 `Baseapp`、`Cellapp`、`Loginapp` 都会各自持有一个 `NetworkInterface`，其上同时管理：

- 对客户端的外部通道
- 对其他组件的内部通道

所以当你看到某个组件里 `networkInterface().processChannels(...)`，你要理解成“处理该进程当前所有活跃连接”，而不是单个 socket 行为。

## 第三层：Channel 才是一条连接的真正运行态

核心文件：

- `kbe/src/lib/network/channel.h`
- `kbe/src/lib/network/channel.inl`

`Channel` 内部同时持有很多状态：

- 地址与 endpoint
- `PacketReader` / `PacketSender`
- `Bundles`
- KCP / WebSocket / 加密过滤器
- 收发统计
- inactivity 检测
- `MessageHandlers*`

从接口看，最值得优先理解的是：

- `send()`
- `sendto()`
- `createSendBundle()`
- `onPacketReceived()`
- `processPackets()`
- `updateTick()`
- `condemn()`

这说明 `Channel` 不只是一个“可写 socket 包装器”，它还负责：

- 连接收发队列
- 包级解析状态
- 超时与销毁状态
- 协议子类型切换（TCP / KCP / WebSocket）

所以网络层真正的“会话对象”其实是 `Channel`。

## 第四层：Bundle 不是消息本身，而是消息容器

核心文件：

- `kbe/src/lib/network/bundle.h`

`Bundle` 最关键的接口就两个：

- `newMessage(const MessageHandler& msgHandler)`
- `finiMessage()`

加上一组 `operator<<` 用于逐字段写入。

这层设计要点是：

- 一条逻辑消息先通过 `newMessage()` 选择消息定义
- 再把参数序列化进当前 packet
- 如果一个 packet 不够，`Bundle` 会自动跨多个 packet 续写

因此 `Bundle` 承担的是“消息打包器”角色，不是“协议 schema”本身；schema 在 `MessageHandlers / Interface` 宏生成结果里。

这也是为什么源码里高频出现这种模式：

```cpp
Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
(*pBundle).newMessage(LoginappInterface::onLoginAccountQueryResultFromDbmgr);
(*pBundle) << value1 << value2;
pChannel->send(pBundle);
```

网络消息在 KBEngine 里就是这样落地的：先选消息 handler，再序列化字段，再交给 channel 发送。

## 第五层：MessageHandlers 决定“收到 msgID 后调用谁”

`MessageHandlers` 本身的定义分散在接口宏和网络公共代码里，但从使用方式已经很清楚：

- 每个组件会设置自己的主消息表
- 同一个进程收到消息后，会按当前消息表查 handler

例如：

- `LoginappInterface::messageHandlers`
- `BaseappInterface::messageHandlers`
- `CellappInterface::messageHandlers`
- `ClientInterface::messageHandlers`

在启动时可以看到类似代码：

- `Loginapp` 把 `pMainMessageHandlers` 指向 `LoginappInterface::messageHandlers`
- `Baseapp` 指向 `BaseappInterface::messageHandlers`
- `Cellapp` 指向 `CellappInterface::messageHandlers`

所以消息分发的本质不是反射，而是：

1. 读出 `msgID`
2. 在对应接口表中查 handler
3. 把 `MemoryStream` 交给该 handler

## PacketReader：消息分发真正发生的地方

如果你只想跟一次“收到包以后到底怎么拆”的路径，直接看：

- `kbe/src/lib/network/packet_reader.cpp`

这里最重要的函数是：

- `PacketReader::processMessages()`

它做的事包括：

- 从 packet 中不断取出 `msgID`
- 查对应 `MessageHandler`
- 检查消息长度是否合法
- 处理分片消息
- 找到目标 handler 后执行业务回调

这层的意义非常重要：

- `Bundle` 解决发送端“怎么写”
- `PacketReader` 解决接收端“怎么拆”

两边合起来，才是完整消息协议栈。

## 典型链路一：登录响应怎样从 Loginapp 发给客户端

从源码能直接看到这一模式：

- `kbe/src/server/loginapp/loginapp.cpp`

高频写法是：

```text
(*pBundle).newMessage(ClientInterface::onLoginSuccessfully)
→ pChannel->send(pBundle)
```

这条链路的关键含义是：

- `Loginapp` 不直接操作底层 socket
- 它只负责选择 `ClientInterface` 中的消息类型并写参数
- 真正的发送由 `Channel` / `PacketSender` / `EndPoint` 完成

所以组件逻辑层和底层收发层是分开的。

## 典型链路二：客户端远程调用怎样到达 Cell

另一条很值得跟的是：

- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/cellapp/cellapp.cpp`

客户端消息先由 Base 侧接住，然后通常转发到 Cell：

```text
Baseapp
  → newMessage(CellappInterface::onRemoteCallMethodFromClient)
  → pChannel->send(pBundle)
  → Cellapp::onRemoteCallMethodFromClient(...)
```

这条链路说明了一个关键事实：

- 网络层并不知道“这是技能释放”还是“移动请求”
- 对网络层来说，这只是某个 `msgID` 命中某个 `CellappInterface` handler

业务语义是在 handler 落地之后才开始的。

## 客户端侧也复用同一套思路

不要把服务端和客户端协议栈想成两套完全不同系统。

在客户端库里：

- `kbe/src/lib/client_lib/clientapp.cpp`
- `kbe/src/lib/client_lib/clientobjectbase.cpp`
- `kbe/src/lib/client_lib/client_interface.h`

可以看到完全一致的模式：

- `processChannels(...)`
- `ClientInterface::messageHandlers`
- `onCreatedProxies`
- `onUpdatePropertys`

也就是说，客户端收到包后同样是：

1. packet reader 拆消息
2. 按 `ClientInterface` 查 handler
3. 进入 `ClientObjectBase::onCreatedProxies()` 或 `onUpdatePropertys()`

所以 KBEngine 的“客户端 SDK”并不是另一门协议，而是同一消息系统在客户端侧的镜像实现。

<a id="client-entity-isplayer-control"></a>
## `isPlayer()`、`player()` 和 `controlledBy()` 其实不是一回事

客户端 API 里最容易让人误读的一条就是 `Entity.isPlayer()`。

如果只看接口名，很容易把它理解成：

- 这个实体现在是不是“被当前客户端控制”
- 这个实体是不是“当前客户端拥有操作权”

源码说明这两个理解都不精确。

### 第一层：`isPlayer()` 只是“身份判断”，不是“控制权判断”

在客户端 SDK 里，`isPlayer()` 的实现非常直接：

```csharp
// 文件：kbe/res/sdk_templates/client/unity/Entity.cs
public bool isPlayer()
{
    return id == KBEngineApp.app.entity_id;
}
```

```cpp
// 文件：kbe/res/sdk_templates/client/ue4/Source/KBEnginePlugins/Engine/Entity.cpp
bool Entity::isPlayer()
{
    return id() == KBEngineApp::getSingleton().entity_id();
}
```

也就是说，它只判断一件事：

- **当前这个实体的 `id`，是不是本次客户端连接记录下来的 `entity_id`**

这本质上是一个“身份比对”，而不是“控制状态比对”。

### 第二层：`entity_id` 是在哪里来的

这个 `entity_id` 不是随便填进去的，它是在服务端通知“你这个连接对应的玩家代理实体创建好了”时写入的。

Unity 客户端里这条链路很清楚：

```csharp
// 文件：kbe/res/sdk_templates/client/unity/KBEngine.cs
public void Client_onCreatedProxies(UInt64 rndUUID, Int32 eid, string entityType)
{
    entity_uuid = rndUUID;
    entity_id = eid;
    entity_type = entityType;
    ...
}
```

随后 `player()` 也只是按这个 `entity_id` 去实体表里查：

```csharp
// 文件：kbe/res/sdk_templates/client/unity/KBEngine.cs
public Entity player()
{
    Entity e;
    if(entities.TryGetValue(entity_id, out e))
        return e;

    return null;
}
```

因此客户端语义上：

- `player()` = 当前连接对应的玩家实体
- `isPlayer()` = “我是不是这个玩家实体”

它们都不直接回答“当前是不是我在控制移动”。

### 第三层：真正的“控制”来自 `controlledBy()`

“控制”这件事在源码里是另一条链，源头在 CellApp 侧实体的 `controlledBy_`：

```cpp
// 文件：kbe/src/server/cellapp/entity.inl
INLINE EntityCall* Entity::controlledBy() const
{
    return controlledBy_;
}
```

CellApp API 文档里 `controlledBy` 也写得很明确：

- 它表示该实体由哪个客户端关联的服务端实体来控制移动
- 如果为 `None`，则实体由服务端移动

所以这里的“控制”，更准确地说是：

- **移动/驱动权限**
- 而不是“这个实体是不是当前连接自己的 player 身份”

### 第四层：服务端如何把“控制状态变化”通知给客户端

CellApp 改变控制者时，走的是 `Entity::setControlledBy()`：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
bool Entity::setControlledBy(EntityCall* controllerBaseEntityCall)
{
    ...
    controlledBy(controllerBaseEntityCall);
    ...
    sendControlledByStatusMessage(controllerBaseEntityCall, 1 or 0);
}
```

真正发给客户端的是 `ClientInterface::onControlEntity`：

```cpp
// 文件：kbe/src/server/cellapp/entity.cpp
void Entity::sendControlledByStatusMessage(EntityCall* baseEntityCall, int8 isControlled)
{
    ...
    (*pForwardBundle).newMessage(ClientInterface::onControlEntity);
    (*pForwardBundle) << id();
    (*pForwardBundle) << isControlled;
    ...
}
```

客户端收到后，再把这个状态写到实体对象上：

```csharp
// 文件：kbe/res/sdk_templates/client/unity/KBEngine.cs
public void Client_onControlEntity(Int32 eid, sbyte isControlled)
{
    ...
    var isCont = isControlled != 0;
    ...
    entity.isControlled = isCont;
    entity.onControlled(isCont);
}
```

所以“控制权”在客户端真正对应的是：

- `entity.isControlled`
- `entity.onControlled(...)`
- 以及客户端内部的 `_controlledEntities`

而不是 `isPlayer()`。

### 第五层：为什么这两个概念一定要分开

客户端实现里专门把这两种状态拆开了。

当收到 `Client_onControlEntity()` 时，如果目标实体不是当前玩家自身，才会进入 `_controlledEntities`：

```csharp
// 文件：kbe/res/sdk_templates/client/unity/KBEngine.cs
if (isCont)
{
    // 如果被控制者是玩家自己，那表示玩家自己被其它人控制了
    // 所以玩家自己不应该进入这个被控制列表
    if (player().id != entity.id)
    {
        _controlledEntities.Add(entity);
    }
}
```

这段代码直接说明了两件事：

1. **玩家自己也可能处于 `isControlled == true`**
   这表示“我这个 player 实体现在被别人控制”
2. **`isPlayer()` 仍然可能为 `true`**
   因为它只看 `id == entity_id`

所以至少会有这几种不同状态：

| 状态 | `isPlayer()` | `isControlled` | 含义 |
|------|--------------|----------------|------|
| 自己的玩家实体，自己正常操作 | `true` | `false` | 当前连接的 player，本地自己驱动 |
| 自己的玩家实体，被别人接管控制 | `true` | `true` | 仍然是自己的 player，但移动控制权不在本地 |
| 不是自己的玩家实体，但被我控制 | `false` | `true` | 当前连接额外控制了另一个可见实体 |
| 普通旁观实体 | `false` | `false` | 只是视野中的普通实体 |

也就是说：

- `isPlayer()` 回答“**这是不是我的 player 身份实体**”
- `isControlled` 回答“**这个实体当前是否处于 controlEntity 控制态**”

它们是两个维度，不是同一个维度。

### 第六层：为什么引擎需要 `isPlayer()`

`isPlayer()` 在客户端不只是一个便利函数，它还影响 `ownerOnly` 客户端属性的处理。

`kbcmd` 生成客户端代码时，会明确生成类似判断：

```cpp
if (pProp_xxx->isOwnerOnly() && !entity->isPlayer())
{
}
else
{
    onXxxChanged(oldVal);
}
```

这说明引擎需要先知道：

- 这个实体是不是“当前连接自己的 player 实体”

只有这样，客户端才能正确区分：

- 哪些属性只有 owner/player 自己能看到
- 哪些属性所有观察者都能看到

因此 `isPlayer()` 的意义首先是**身份识别**，其次才是给业务层提供一个便捷 API。

### 结论：如何正确理解 API 文档里的“当前客户端的 Player”

对 `Entity.isPlayer()` 最准确的理解应该是：

- **它返回当前实体是否就是本次客户端连接对应的 player 实体**

而不是：

- “当前客户端正在控制的实体”
- “当前客户端拥有移动控制权的实体”

如果你要继续追“控制权”这条链，最短源码路径是：

1. `kbe/src/server/cellapp/entity.h` / `entity.inl` → `controlledBy()`
2. `kbe/src/server/cellapp/entity.cpp` → `setControlledBy()` / `sendControlledByStatusMessage()`
3. `kbe/res/sdk_templates/client/unity/KBEngine.cs` 或 `ue4/KBEngine.cpp` → `Client_onControlEntity()`
4. `kbe/res/sdk_templates/client/unity/Entity.cs` 或 `ue4/Entity.cpp` → `isPlayer()`

## 内部通信和外部通信的共同点与差异

共同点：

- 都走 `Channel`
- 都走 `Bundle`
- 都用 `MessageHandlers` 查 handler

差异：

- 外部连接要处理 handshake、版本校验、脚本协议摘要
- 内部连接更多依赖组件接口表和固定消息流
- 外部连接可能启用 KCP / WebSocket / 加密过滤器
- 内部连接更偏向组件间 RPC 与状态同步

因此“内部消息系统”和“客户端协议”在抽象层面是同一体系，在具体接口表和握手流程上不同。

## 读源码的最短路径

如果你现在准备在 IDE 里跟一遍网络栈，建议按这个顺序：

1. `kbe/src/lib/network/event_dispatcher.h`
2. `kbe/src/lib/network/network_interface.h`
3. `kbe/src/lib/network/channel.h`
4. `kbe/src/lib/network/bundle.h`
5. `kbe/src/lib/network/packet_reader.cpp`
6. `kbe/src/server/loginapp/loginapp.cpp`
7. `kbe/src/server/baseapp/baseapp.cpp`
8. `kbe/src/server/cellapp/cellapp.cpp`
9. `kbe/src/lib/client_lib/clientobjectbase.cpp`

这样读的价值在于：先建立共用网络骨架，再看服务端组件如何用它，最后看客户端如何对称消费它。

## 与主线章节的关系

如果你想看更完整的叙事版讲解，主线仍在：

- `/study/08-network-infrastructure.html`
- `/study/10-serialization-bundle-and-messages.html`
- `/study/11-rpc-entitycall-and-communication-patterns.html`

这一页的职责是把这些章节背后的共用源码骨架压缩成一张“网络阅读地图”。
