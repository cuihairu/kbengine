# 8. 网络基础设施：I/O 模型与进程间通信

> 这一章回答：游戏服务器的网络层为什么用 Reactor 而不是 Proactor？epoll 和 select 怎么选？Channel 和 Endpoint 是什么关系？两套项目的网络层有什么本质差异？

## 8.1 本章核心问题

- I/O 多路复用（select / poll / epoll / kqueue）各自的特点和选择依据？
- Reactor 模式的四个参与者在代码里怎么对应？
- Channel 与 Endpoint 的职责划分？
- TCP vs UDP：两套项目为什么做了不同选择？
- InterfaceTable / MessageHandlers：消息路由怎么实现？

## 8.2 I/O 多路复用：为什么游戏服务器选 epoll

### select / poll / epoll / kqueue 对比

| 维度 | select | poll | epoll | kqueue |
|------|--------|------|-------|--------|
| 最大 FD 数 | 1024（FD_SETSIZE） | 无限制 | 无限制 | 无限制 |
| 时间复杂度 | O(n) 遍历 | O(n) 遍历 | O(1) 事件通知 | O(1) 事件通知 |
| 每次调用 | 全量传 fd_set | 全量传 pollfd | 只返回就绪 fd | 只返回就绪 fd |
| 触发模式 | 水平触发 | 水平触发 | 水平/边缘 | 水平/边缘 |
| 适用场景 | 少量连接 | 中等连接 | 大量连接（Linux） | 大量连接（BSD/Mac） |

**游戏服务器为什么选 epoll**：
- CellApp / BaseApp 每个进程维护数百到数千个 Channel（客户端 + 内部进程）
- select 每次传全部 fd_set 内核 ↔ 用户态拷贝开销大
- epoll 只返回就绪事件，O(1) 通知，不受总连接数影响

### KBEngine EventPoller

```cpp
// 文件：kbe/src/lib/network/event_poller.h（简化）
class EventPoller
{
public:
    virtual int processPendingEvents(double maxWait) = 0;
    static EventPoller* create();   // 工厂方法

    bool registerForRead(int fd, InputNotificationHandler* handler);
    bool registerForWrite(int fd, OutputNotificationHandler* handler);

protected:
    FDReadHandlers fdReadHandlers_;   // map<int, InputNotificationHandler*>
    FDWriteHandlers fdWriteHandlers_; // map<int, OutputNotificationHandler*>
};
```

工厂方法自动选择：

```cpp
// 文件：kbe/src/lib/network/event_poller.cpp
EventPoller* EventPoller::create()
{
#ifdef HAS_EPOLL
    return new EpollPoller();   // Linux：epoll
#else
    return new SelectPoller();  // 其他：select
#endif
}
```

### BigWorld EventPoller

```cpp
// 文件：lib/network/event_poller.hpp（简化）
class EventPoller : public InputNotificationHandler  // 自身也实现回调
{
    static EventPoller* create();
};

// 工厂方法多了一个 poll 选项
EventPoller* EventPoller::create()
{
#if defined(HAS_POLL)
    return new PollPoller();      // Emscripten 平台
#elif defined(HAS_EPOLL)
    return new EPoller();         // Linux
#else
    return new SelectPoller();    // Windows/Mac
#endif
}
```

BigWorld 多了 `PollPoller`（用于 Emscripten/WebAssembly 平台）。

### epoll 实现：EPoller::processPendingEvents

```cpp
// 文件：lib/network/event_poller.cpp（BigWorld，简化）
int EPoller::processPendingEvents(double maxWait)
{
    static const int MAX_EVENTS = 10;
    struct epoll_event events[MAX_EVENTS];
    int maxWaitMs = int(ceil(maxWait * 1000));

    int nfds = epoll_wait(epfd_, events, MAX_EVENTS, maxWaitMs);

    for (int i = 0; i < nfds; ++i) {
        if (events[i].events & (EPOLLERR | EPOLLHUP))
            this->triggerError(events[i].data.fd);
        else {
            if (events[i].events & EPOLLIN)
                this->triggerRead(events[i].data.fd);
            if (events[i].events & EPOLLOUT)
                this->triggerWrite(events[i].data.fd);
        }
    }
    return nfds;
}
```

KBEngine 的 `EpollPoller::processPendingEvents` 逻辑相同。

## 8.3 Reactor 模式：四个参与者的代码映射

Reactor 模式的四个参与者：

| Reactor 参与者 | 代码对应 |
|---------------|---------|
| **Handle**（资源句柄） | `int fd`（socket 文件描述符） |
| **Sync Demultiplexer**（同步多路分离器） | `EventPoller`（封装 epoll_wait / select） |
| **Event Handler**（事件处理器） | `InputNotificationHandler` / `OutputNotificationHandler` |
| **Reactor**（反应器） | `EventDispatcher`（注册/注销/事件循环） |

```
EventDispatcher::processOnce()
  │
  ├── processTasks()          ← 非网络异步任务
  ├── processTimers()         ← 定时器
  ├── processStats()          ← 统计
  └── processNetwork()
        │
        └── EventPoller::processPendingEvents(maxWait)
              │
              └── epoll_wait(fd, events, ...)     ← Sync Demultiplexer
                    │
                    ├── triggerRead(fd)           ← 找到 Handler 并调用
                    │     └── handler->handleInputNotification(fd)
                    └── triggerWrite(fd)
                          └── handler->handleOutputNotification(fd)
```

**为什么不是 Proactor**：Proactor 需要异步 I/O（Windows IOCP / Linux io_uring）。两个项目设计于 2002-2008 年，Linux 异步 I/O（AIO）不成熟，epoll 是最佳选择。Proactor 模型的编程复杂度也更高——回调嵌套、错误处理、缓冲区管理都比 Reactor 复杂。

## 8.4 Channel 与 Endpoint

### Endpoint：对 socket fd 的封装

```cpp
// 文件：kbe/src/lib/network/endpoint.h（简化）
class EndPoint : public PoolObject
{
    void socket(int type);                    // SOCK_STREAM / SOCK_DGRAM
    int setnonblocking(bool nonblocking);
    int bind(uint16_t port, uint32_t addr);
    int listen(int backlog = 5);
    EndPoint* accept(uint16_t* port, uint32_t* addr);
    int send(const void* data, int size);
    int recv(void* data, int size);
    // ...
};
```

Endpoint 只做一件事：**封装操作系统的 socket API**。

### Channel：Endpoint + 消息处理 + 生命周期

```cpp
// 文件：kbe/src/lib/network/channel.h（简化）
class Channel : public TimerHandler, public PoolObject
{
    // 关键成员
    NetworkInterface& networkInterface_;
    EndPoint endpoint_;
    PacketReader packetReader_;
    Bundle* pBundle_;
    MessageHandlers* pMessageHandlers_;

    // 生命周期
    void destroy();
    void send(Bundle* pBundle);
    void onPacketReceived(Packet* pPacket);
};
```

**Channel = Endpoint + PacketReader + Bundle + MessageHandlers**。它不只是 socket 的封装，而是：

- 管理消息的**分包/组包**（PacketReader）
- 管理消息的**发送缓冲**（Bundle）
- 管理消息的**路由分发**（MessageHandlers）
- 管理**连接生命周期**（超时/断线/重连）

### BigWorld Channel：抽象基类

```cpp
// 文件：lib/network/channel.hpp（简化）
class Channel : public ReferenceCount
{
    virtual Bundle* newBundle() = 0;
    virtual bool hasUnsentData() const = 0;
    virtual bool isExternal() const = 0;
    virtual bool isTCP() const = 0;
    virtual void setEncryption(BlockCipherPtr pBlockCipher) = 0;
    virtual double roundTripTimeInSeconds() const = 0;
    // ...
};
```

BigWorld 的 Channel 是抽象的——有 `UDPChannel`、`TCPChannel` 等子类。KBEngine 的 `Channel` 则是一个具体实现类，但它并不等于“只有 TCP”：同一个类里还能挂 `ProtocolType`、`ProtocolSubType`、`ikcp` 和 `PacketFilter`，因此它更像“统一连接对象”，而不是只服务于单一传输协议。

## 8.5 TCP vs UDP：不同路径的不同选择

### KBEngine

- **内部通信（进程间）**：TCP
- **外部通信（客户端）**：TCP / UDP / KCP（可配置）

TCP 的优势：实现简单、调试方便、天然有序可靠。

### BigWorld

- **内部通信（进程间）**：UDP + 自建可靠性层
- **外部通信（客户端）**：TCP

BigWorld 选择内部 UDP 的原因：
- **延迟敏感**：Cell ↔ Base 的实时同步，UDP 延迟更低
- **自定义可靠性**：通过 `ReliableType` 控制哪些消息需要可靠传输

```cpp
// 文件：lib/network/bundle.hpp（BigWorld）
enum ReliableTypeEnum {
    RELIABLE_NO = 0,        // 完全不可靠（位置更新）
    RELIABLE_DRIVER = 1,    // 可靠驱动消息
    RELIABLE_PASSENGER = 2, // 搭便车（随驱动消息一起可靠传输）
    RELIABLE_CRITICAL = 3   // 关键消息（必须可靠）
};
```

**KBEngine 没有 `ReliableType`**——因为内部用 TCP，所有消息天然可靠。

### 与 Aeron 的对比

Aeron 也是 "UDP + 自建可靠性"，和 BigWorld 的思路类似：
- Aeron 提供**背压感知**（back-pressure awareness）——消费者来不及处理时通知生产者减速
- BigWorld 没有显式背压机制，通过 Channel 缓冲和超时丢弃处理

## 8.6 InterfaceTable / MessageHandlers：消息路由表

### KBEngine MessageHandlers

```cpp
// 文件：kbe/src/lib/network/message_handler.h（简化）
class MessageHandler
{
    std::string name;
    MessageID msgID;
    MessageArgs* pArgs;
    int32 msgLen;
    bool exposed;           // 是否客户端可调用
    MessageHandlers* pMessageHandlers;

    // 统计
    volatile uint32 send_size, send_count, recv_size, recv_count;

    virtual void handle(Channel* pChannel, MemoryStream& s) {
        pArgs->createFromStream(s);
    }
};

class MessageHandlers
{
    typedef std::map<MessageID, MessageHandler*> MessageHandlerMap;
    MessageHandler* find(MessageID msgID);
private:
    MessageHandlerMap msgHandlers_;
};
```

**消息路由 = 静态 map 查找**：MessageID → MessageHandler。组件启动时通过 `DEFINE_IN_INTERFACE` 宏注册所有消息。

### BigWorld InterfaceTable

```cpp
// 文件：lib/network/interface_table.hpp（简化）
class InterfaceTable : public TimerHandler
{
    void serve(const InterfaceElement& ie, InputMessageHandler* pHandler);
    void onBundleStarted(Channel* pChannel);
    void onBundleFinished(Channel* pChannel);

    const char* msgName(MessageID msgID) const { return table_[msgID].name(); }
private:
    typedef BW::vector<InterfaceElementWithStats> Table;
    Table table_;
    BundleEventHandler* pBundleEventHandler_;
};
```

BigWorld 用 vector（不是 map），消息 ID 直接作为索引。还多了：
- **Bundle 事件回调**：`onBundleStarted` / `onBundleFinished`——用于追踪消息边界
- **统计定时器**：定期收集每个消息的收发统计
- **与 bwmachined 集成**：`registerWithMachined` 向注册中心宣告接口

## 8.7 两套项目的网络层架构对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 内部传输 | TCP | UDP + 自建可靠性 |
| 外部传输 | TCP / UDP / KCP | TCP |
| EventPoller | Epoll / Select | EPoll / Poll / Select |
| Channel | 具体类（统一承载 TCP/UDP/KCP 连接态） | 抽象类（UDP/TCP 子类） |
| 可靠性分级 | 无（TCP 天然可靠） | `ReliableType` 四级 |
| 消息路由 | `MessageHandlers`（map） | `InterfaceTable`（vector） |
| Bundle 事件 | 无 | `BundleEventHandler` 回调 |
| Dispatcher 分层 | 无 | `attach`/`detach` 父子分层 |
| 加密支持 | 无内置 | `BlockCipher` 加密接口 |
| 回复处理 | 无内置请求-回复 | `ReplyMessageHandler` + 超时异常 |
| 进程发现 | `Machine` 进程（UDP 广播） | `bwmachined` 守护进程（UDP 广播） |

## 8.8 进程发现与协调：Machine 的角色

两个项目都不是"DBMgr 集中协调"。**Machine 进程**才是组件发现和进程协调的核心。

### KBEngine Machine

```cpp
// 文件：kbe/src/server/machine/machine.h（简化）
class Machine : public ServerApp, public Singleton<Machine>
{
    // 组件广播地址 → 注册到本机 Machine
    void onBroadcastInterface(Network::Channel* pChannel,
        COMPONENT_TYPE componentType, COMPONENT_ID componentID, ...);

    // 某进程想找另一类进程的地址
    void onFindInterfaceAddr(Network::Channel* pChannel,
        COMPONENT_TYPE findComponentType, ...);

    // 查询所有组件信息
    void onQueryAllInterfaceInfos(Network::Channel* pChannel, ...);

    // 查询所有 Machine 进程
    void onQueryMachines(Network::Channel* pChannel, ...);

    // 远程启动/停止进程
    void startserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);
    void stopserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);
};
```

**通信方式**：UDP 广播 + 单播。每台物理机运行一个 Machine 实例，各组件启动时向本机 Machine 广播自己的地址和类型（`onBroadcastInterface`），Machine 收集后供其他组件查询（`onFindInterfaceAddr`）。

**职责**：

| 职责 | 方法 | 说明 |
|------|------|------|
| 组件注册 | `onBroadcastInterface` | 收集本机所有组件的地址/类型/状态 |
| 组件查找 | `onFindInterfaceAddr` | 按 componentType 查找目标进程地址 |
| 全量查询 | `onQueryAllInterfaceInfos` | 返回所有已注册组件信息 |
| 远程启停 | `startserver` / `stopserver` | 跨机器远程启动/停止进程 |
| 进程创建 | `startLinuxProcess` / `startWindowsProcess` | fork 或 CreateProcess 新进程 |

### BigWorld bwmachined

```cpp
// 文件：programming/bigworld/server/tools/bwmachined/（目录结构）
bwmachined/
  ├── bwmachined.cpp          ← 主入口
  ├── cluster.hpp/cpp         ← 集群发现和协调
  ├── machine_guard.hpp       ← 进程守护
  ├── linux_machine_guard.cpp ← Linux 进程管理
  └── daemon/                  ← 守护进程逻辑
```

BigWorld 的 `bwmachined` 职责类似但更重：

| 职责 | KBEngine Machine | BigWorld bwmachined |
|------|-----------------|-------------------|
| 组件注册 | `onBroadcastInterface` | 类似的广播注册 |
| 组件查找 | `onFindInterfaceAddr` | 类似的查找机制 |
| 远程启停 | `startserver` / `stopserver` | Machine Guard 进程管理 |
| 集群协调 | 无 | `Cluster` 类，机器发现与健康监控 |
| 进程守护 | 无 | `MachineGuard`，SIGCHLD 处理和重启 |
| 资源监控 | 无（Watcher 独立实现） | 内置 CPU/内存/磁盘监控 |
| 跨平台进程创建 | `startLinuxProcess` / `startWindowsProcess` | `linux_machine_guard.cpp` |

### 进程发现的时序

```text
1. Machine/bwmachined 先于所有组件启动
2. 各组件启动时：
   a. 向本机 Machine 广播自己的身份（componentType + componentID + 地址）
   b. 通过 Machine 查找依赖的远程组件地址（如 Baseapp 查找 Dbmgr）
3. Machine 维护本机组件注册表，响应查询请求
4. 跨机器通信时：先查目标组件所在机器的 Machine，再建立直连
```

**关键区别**：KBEngine 的 Machine 是一个独立的 kbe 进程（`kbe/bin/server/machine`），BigWorld 的 bwmachined 是独立的系统守护进程。两者都不做业务逻辑，只做"进程发现和生命周期管理"。

## 8.9 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| EventDispatcher | `kbe/src/lib/network/event_dispatcher.h` |
| EventPoller | `kbe/src/lib/network/event_poller.h` |
| EpollPoller | `kbe/src/lib/network/poller_epoll.h` |
| SelectPoller | `kbe/src/lib/network/poller_select.h` |
| NetworkInterface | `kbe/src/lib/network/network_interface.h` |
| Channel | `kbe/src/lib/network/channel.h` |
| Endpoint | `kbe/src/lib/network/endpoint.h` |
| Bundle | `kbe/src/lib/network/bundle.h` |
| MessageHandler | `kbe/src/lib/network/message_handler.h` |
| Machine（进程发现） | `kbe/src/server/machine/machine.h` |
| Components（组件管理） | `kbe/src/lib/server/components.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| EventDispatcher | `lib/network/event_dispatcher.hpp` |
| EventPoller | `lib/network/event_poller.hpp` |
| NetworkInterface | `lib/network/network_interface.hpp` |
| Channel | `lib/network/channel.hpp` |
| Endpoint | `lib/network/endpoint.hpp` |
| Bundle | `lib/network/bundle.hpp` |
| InterfaceTable | `lib/network/interface_table.hpp` |
| 消息回调 | `lib/network/interfaces.hpp` |
| bwmachined（进程发现） | `server/tools/bwmachined/` |
| Cluster（集群协调） | `server/tools/bwmachined/cluster.hpp` |
| MachineGuard（进程守护） | `server/tools/bwmachined/linux_machine_guard.cpp` |

## 8.10 源码走读路径

### 路径一：理解 Reactor 模式的代码映射

1. `kbe/src/lib/network/event_dispatcher.cpp` — `processOnce()` → `processNetwork()`
2. `kbe/src/lib/network/event_poller.cpp` — `create()` 工厂方法
3. `kbe/src/lib/network/poller_epoll.h` — `processPendingEvents()` 里的 `epoll_wait`
4. 对比 BigWorld: `lib/network/event_dispatcher.cpp` — 多了 `processFrequentTasks()`

### 路径二：理解 Channel 的消息收发

1. `kbe/src/lib/network/endpoint.h` — 底层 socket 操作
2. `kbe/src/lib/network/channel.h` — Channel = Endpoint + PacketReader + Bundle
3. `kbe/src/lib/network/bundle.h` — `newMessage()` / `finiMessage()` 消息边界
4. `kbe/src/lib/network/message_handler.h` — msgID → handler 查找

### 路径三：对比 TCP vs UDP 架构

1. KBEngine: Channel 是具体类，只有 TCP 实现
2. BigWorld: `lib/network/channel.hpp` — 抽象基类，有 `UDPChannel` 子类
3. BigWorld: `lib/network/bundle.hpp` — `ReliableType` 四级可靠性

### 路径四：理解进程发现与协调

1. KBEngine: `kbe/src/server/machine/machine.h` — `onBroadcastInterface` 组件注册、`onFindInterfaceAddr` 组件查找
2. KBEngine: `kbe/src/lib/server/components.h` — `Components` 单例，收集所有已知组件
3. BigWorld: `server/tools/bwmachined/` — `cluster.hpp` 集群发现、`linux_machine_guard.cpp` 进程守护
4. 对比：KBEngine Machine 只做发现和启停；BigWorld bwmachined 额外负责进程守护（崩溃重启）和资源监控

## 8.11 小结

- **Reactor 模式**是两套项目共同的 I/O 模型：EventDispatcher（反应器）→ EventPoller（多路分离器）→ Handler（回调）
- **epoll** 是 Linux 上的最佳选择——O(1) 事件通知，不受连接数影响
- **Channel = Endpoint + PacketReader + Bundle + Handlers**，不只是 socket 封装
- **KBEngine 选 TCP**（简单可靠），**BigWorld 选 UDP + 自建可靠性**（低延迟可控）
- BigWorld 的网络层更完善：Dispatcher 父子分层、消息可靠性分级、Bundle 事件回调、加密接口、请求-回复超时
- BigWorld 用 `ReliableType` 实现"同一条通道内不同消息不同可靠性"——这是 TCP 做不到的
- **进程发现不是 DBMgr 的职责**——两套项目都用独立的 Machine 进程（KBEngine `machine` / BigWorld `bwmachined`）通过 UDP 广播实现组件注册和查找
