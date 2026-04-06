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
