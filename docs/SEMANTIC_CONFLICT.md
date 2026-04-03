# IOCP 实现的语义冲突问题

## 🚨 核心问题

KBEngine 框架基于**同步 I/O 多路复用（Reactor 模式）**设计，而 IOCP 是**异步 I/O（Proactor 模式）**，两者存在根本性语义冲突。

---

## 📊 两种模式的对比

### Reactor 模式（Select/Epoll）

```
1. 应用告诉内核："关注这个 socket 的可读事件"
2. 应用阻塞在 select/epoll_wait()
3. 内核："socket X 可读了"
4. 应用调用 recv(X, ...) 主动读取数据
5. 应用处理数据
```

**关键特征：**
- **事件通知** - 告诉你"可以读了"
- **主动读取** - 应用必须调用 recv()
- **同步非阻塞** - 调用不阻塞，但需要循环处理

### Proactor 模式（IOCP/io_uring）

```
1. 应用调用 WSARecv(socket, ...) 发起异步读
2. 应用继续做其他事情
3. 内核自动读取数据到缓冲区
4. 内核："socket X 的读操作完成了，数据在这里"
5. 应用直接处理数据（已经读好了）
```

**关键特征：**
- **完成通知** - 告诉你"已经读好了"
- **自动读取** - 内核帮你读完
- **真正异步** - 读取过程不需要应用参与

---

## ⚠️ 语义冲突的具体体现

### KBEngine 的 EventPoller 接口设计

```cpp
// event_poller.cpp:86-98
bool EventPoller::triggerRead(int fd)
{
    FDReadHandlers::iterator iter = fdReadHandlers_.find(fd);
    if (iter == fdReadHandlers_.end())
        return false;

    // ← 只是通知 handler，不做实际读取！
    iter->second->handleInputNotification(fd);
    return true;
}
```

### Handler 的实现期望

```cpp
// handler 期望在通知时主动读取
void Channel::handleInputNotification(int fd)
{
    char buffer[4096];
    int n = ::recv(fd, buffer, sizeof(buffer), 0);  // ← 同步读取
    if (n > 0) {
        processPacket(buffer, n);
    }
}
```

### IOCP 的实现方式

```cpp
// poller_iocp.cpp 中的实现
bool IocpPoller::triggerRead(int fd)
{
    // 创建 Overlapped 结构
    IocpOverlappedData* opData = new IocpOverlappedData(IOCP_OP_READ, fd);

    // ← 直接发起异步读！
    WSARecv((SOCKET)fd, &(opData->dataBuffer), 1, ...);

    return true;
}

// 当 IOCP 完成时
void IocpPoller::processPendingEvents(double maxWait)
{
    GetQueuedCompletionStatus(..., &overlapped, ...);
    IocpOverlappedData* opData = ...;

    // ← 数据已经在 opData->buffer 中了！
    // 但 triggerRead 期望通知 handler 让它调用 recv()
    // 而 recv() 会再次尝试读取，导致冲突！
}
```

### 冲突点

| 阶段 | Select/Epoll | IOCP |
|------|-------------|------|
| **注册** | 注册"可读事件"关注 | 发起异步读操作 |
| **通知内容** | "socket 可读了" | "数据已读完" |
| **数据位置** | 在系统缓冲区 | 在用户缓冲区 |
| **下一步动作** | 应用调用 recv() | 应用处理数据 |
| **triggerRead** | 通知 handler | ❌ 已经读了，怎么通知？ |

---

## 🚫 为什么简单替换不工作

```cpp
// 1. IOCP 模式：已经在 triggerRead 中读了
bool IocpPoller::triggerRead(int fd) {
    WSARecv(fd, ...);  // 数据读到这里了
}

// 2. 完成队列处理
void IocpPoller::processPendingEvents() {
    // 数据已经读好了，在 opData->buffer 中
    IocpOverlappedData* opData = ...;
}

// 3. 通知 handler
triggerRead(fd);  // ← 调用 handler

// 4. Handler 的实现
void Channel::handleInputNotification(int fd) {
    recv(fd, buffer, ...);  // ← ❌ 再次读取！冲突！
}
```

**问题：**
1. IOCP 已经在步骤 1 读取了数据
2. 步骤 4 的 `recv()` 会再次尝试读取
3. 要么读到重复数据，要么阻塞等待

---

## 💡 解决方案

### 方案 1：修改 Handler 接口（破坏性）

```cpp
// 修改 handler 接口，接收已读好的数据
class InputNotificationHandler {
public:
    // 旧接口
    virtual void handleInputNotification(int fd) = 0;

    // 新接口：IOCP 传递数据
    virtual void handleInputNotification(int fd, const char* data, size_t size) = 0;
};
```

**问题：**
- ❌ 需要修改所有 handler 实现
- ❌ Select/Epoll 也需要适配（不适用）
- ❌ 破坏性太大

### 方案 2：IOCP 预读 + recv 从缓存返回

```cpp
class IocpPoller {
private:
    std::map<int, std::queue<char*>> readBuffers_;  // per-fd 缓存
};

int recv(int fd, void* buf, size_t size) {
    // 如果是 IOCP 模式且缓存有数据
    if (hasCachedData(fd)) {
        return getCachedData(fd, buf, size);  // ← 从缓存返回
    }
    // 否则调用原生 recv
    return ::recv(fd, buf, size);
}
```

**优点：**
- ✅ 不需要修改 handler
- ✅ 兼容现有代码

**缺点：**
- ⚠️ `recv()` 需要全局 hook
- ⚠️ 不是真正的异步
- ⚠️ 增加复杂度

### 方案 3：协程桥接（推荐，参考 Go）

```cpp
class Channel {
    NetworkCoroutine* coroutine_;

    void processRead() {
        // 看起来是同步的，实际是异步
        int n = co_recv(fd_, buffer_, size_);  // 可能挂起
        // IOCP 完成后自动恢复
        processPacket(buffer_, n);
    }
};

int co_recv(int fd, void* buf, size_t size) {
    if (!hasDataInBuffer()) {
        // 发起异步读，挂起协程
        WSARecv(fd, ...);
        coroutine_->yield();  // ← 挂起
    }
    // 协程恢复，数据已就绪
    return getCachedData(fd, buf, size);
}
```

**优点：**
- ✅ 用户代码保持同步风格
- ✅ 真正利用异步 I/O
- ✅ 性能最优

**缺点：**
- ⚠️ 需要实现协程调度器
- ⚠️ 增加复杂度

---

## 📚 参考资源

### Go Netpoller（最佳参考）

Go 完美解决了这个问题：

```go
// 用户代码看起来是同步的
func (c *conn) Read(b []byte) (int, error) {
    n, err := c.fd.Read(b)  // ← 阻塞读取
    return n, err
}

// 但底层是异步的
func (fd *netFD) Read(p []byte) (int, error) {
    // 发起异步读
    n, err := syscall.Read(fd.pfd.Sysfd, p)

    // 如果会阻塞，挂起 goroutine
    if err == syscall.EAGAIN {
        poller.Wait(fd, 'r')  // ← 挂起
        // IOCP 完成后恢复到这里
        // 重试读取
        n, err = syscall.Read(fd.pfd.Sysfd, p)
    }

    return n, err
}
```

**关键：goroutine 提供可挂起的执行上下文！**

### 相关资料

- [Go runtime/netpoll.go](https://github.com/golang/go/blob/master/src/runtime/netpoll.go) - 平台抽象
- [Go netpoll_windows.go](https://go.dev/src/runtime/netpoll_windows.go) - IOCP 实现
- [Proactor vs Reactor](https://www.artima.com/articles/io-design-pattern-reactor-vs-proactor)

---

## 🎯 结论

**简单添加 IOCP poller 无法工作，因为：**

1. **语义不兼容** - Reactor vs Proactor
2. **接口冲突** - "可读通知" vs "完成通知"
3. **数据流不同** - 主动读取 vs 自动推送

**正确的解决方案是参考 Go Netpoller：**

- 用调度器/协程桥接异步 I/O 和同步代码
- 保持用户代码的同步风格
- 底层充分利用平台异步能力

**详见：** [GO_NETPOLLER_DESIGN.md](GO_NETPOLLER_DESIGN.md)

---

**创建日期**: 2026-04-03
**问题**: 架构不兼容
**解决方案**: 参考 Go Netpoller
