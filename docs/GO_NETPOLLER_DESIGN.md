# 参考 Go Netpoller 的网络层重构方案

## 🎯 核心思想

**问题：** KBEngine 基于同步 I/O 多路复用语义（Select/Epoll），IOCP/io_uring 是异步完成模型

**Go 的解决方案：** 用调度器桥接异步 I/O 和同步代码

---

## 📊 设计对比

### 当前 KBEngine（同步 I/O 多路复用）

```
┌─────────────────────────────────────────┐
│  Channel::processRead()                 │
│    ↓                                    │
│  EventPoller::waitForEvents()           │ ← select/epoll_wait
│    ↓                                    │
│  "socket 可读"                          │
│    ↓                                    │
│  triggerRead(fd)                        │
│    ↓                                    │
│  handler->handleInputNotification()     │
│    ↓                                    │
│  recv(fd, buf)  ← 同步读取              │
│    ↓                                    │
│  processPacket(buf)                     │
└─────────────────────────────────────────┘
```

**问题：**
- Windows 上使用 Select 性能差（~1024 连接限制）
- 无法利用 IOCP 的异步能力
- 需要手动管理读写状态

### Go Netpoller（异步 I/O + 调度器）

```
┌─────────────────────────────────────────┐
│  goroutine 代码                          │
│    ↓                                    │
│  conn.Read(buf)  ← 看起来是同步阻塞     │
│    ↓                                    │
│  【goroutine 被挂起】                   │
│    ↓                                    │
│  runtime: WSARecv()  ← 发起异步 I/O     │
│    ↓                                    │
│  IOCP 完成队列                           │
│    ↓                                    │
│  【goroutine 被唤醒】                   │
│    ↓                                    │
│  继续执行 processPacket(buf)            │
└─────────────────────────────────────────┘
```

**关键：**
- **用户代码保持同步风格**
- **底层使用异步 I/O**
- **调度器自动切换上下文**

---

## 🚀 KBEngine 改造方案

### 方案 A：轻量级协程（推荐）

类似于 Go 的 goroutine，但针对 C++:

```cpp
// 1. 创建可挂起的协程上下文
class NetworkCoroutine {
public:
    void yield();      // 挂起，等待 I/O
    void resume();     // I/O 完成后恢复
};

// 2. 修改 Channel 使用协程
class Channel {
    void processRead() {
        // 看起来是同步的
        int n = recv(fd_, buffer_, size);  // 可能挂起
        // IOCP 完成后自动恢复到这里
        processPacket(buffer_, n);
    }

private:
    NetworkCoroutine* coroutine_;
};

// 3. 底层 IOCP 实现
int recv(int fd, void* buf, size_t size) {
    if (!hasDataInBuffer()) {
        // 发起异步读，挂起协程
        WSARecv(fd, ...);
        coroutine_->yield();  // ← 挂起
    }
    // 协程恢复后，数据已经在缓冲区了
    return bufferedDataSize_;
}
```

**优点：**
- ✅ 用户代码保持同步风格
- ✅ 底层充分利用 IOCP/io_uring
- ✅ 改动相对较小
- ✅ 性能接近 Go

**缺点：**
- ⚠️ 需要实现协程调度器
- ⚠️ 增加复杂度

### 方案 B：Promise/Future 模式

```cpp
// 1. 定义异步结果
template<typename T>
class Future {
public:
    T get();              // 阻塞等待结果
    bool isReady();
};

// 2. Channel 使用 Future
class Channel {
    void processRead() {
        Future<int> future = asyncRecv(fd_, buffer_, size);

        // 处理其他事情...

        int n = future.get();  // 等待完成
        processPacket(buffer_, n);
    }
};

// 3. IOCP 实现
Future<int> asyncRecv(int fd, void* buf, size_t size) {
    auto promise = std::make_shared<Promise<int>>();

    // 发起异步读
    WSARecv(fd, overlapped, [](DWORD bytes) {
        promise->setValue(bytes);  // 完成时设置值
    });

    return promise->getFuture();
}
```

**优点：**
- ✅ 使用标准 C++ 特性
- ✅ 不需要协程

**缺点：**
- ⚠️ 代码风格改变较大
- ⚠️ 需要大量使用回调

### 方案 C：混合模式（过渡方案）

保持现有语义，但优化底层：

```cpp
class IocpPoller : public EventPoller {
protected:
    bool doRegisterForRead(int fd) override {
        // 1. 预读：发起异步 WSARecv
        OverlappedData* data = new OverlappedData(fd);
        WSARecv(fd, data->buffer, ...);

        // 2. 注册到完成端口
        CreateIoCompletionPort((HANDLE)fd, hPort_, fd, 0);
        return true;
    }

    int processPendingEvents(double maxWait) override {
        // 3. 等待完成
        GetQueuedCompletionStatus(..., &overlapped, ...);

        // 4. 数据已经在缓冲区了
        //    但保持 triggerRead 语义，handler 看不到差异
        OverlappedData* data = (OverlappedData*)overlapped;
        data->fd;  // 保存数据到 fd 的缓存

        // 5. 触发 handler（handler 仍然调用 recv，但会从缓存返回）
        triggerRead(data->fd);
    }
};

// 6. 修改 recv 从缓存读取
int recv(int fd, void* buf, size_t size) {
    if (hasCachedData(fd)) {
        return getCachedData(fd, buf, size);  // ← 从 IOCP 缓存返回
    }
    return ::recv(fd, buf, size);  // ← 回退到同步
}
```

**优点：**
- ✅ 最小改动
- ✅ 保持现有接口
- ✅ 兼容性好

**缺点：**
- ⚠️ 仍然不是真正的异步
- ⚠️ 预读增加延迟

---

## 📋 平台支持对比

| 平台 | Select | Epoll | IOCP | io_uring |
|------|--------|-------|------|----------|
| **当前 KBEngine** | ✅ | ✅ | ❌ | ❌ |
| **Go** | ❌ | ✅ | ✅ | 🔄 讨论 |
| **方案 A（协程）** | ❌ | ✅ | ✅ | ✅ |
| **方案 B（Future）** | ❌ | ✅ | ✅ | ✅ |
| **方案 C（混合）** | ✅ | ✅ | ✅ | ⚠️ 困难 |

---

## 🛠️ io_uring 支持路径

### io_uring 与 IOCP 的相似性

```c
// IOCP (Windows)
WSARecv(socket, wsabuf, &overlapped, completion_routine);
GetQueuedCompletionStatus(port, &bytes, &key, &overlapped, timeout);

// io_uring (Linux)
io_uring_prep_read(sqe, fd, buf, size, 0);
io_uring_submit(ring);
io_uring_wait_cqe(ring, &cqe);
```

**两者都是：**
1. 提交队列：发起操作
2. 完成队列：获取结果
3. 异步完成：系统自动完成

### 统一抽象

```cpp
class AsyncPoller {
public:
    // 提交异步读
    virtual void submitRead(int fd, Buffer* buf) = 0;

    // 等待完成
    virtual int waitForCompletion(double timeout) = 0;

protected:
#if KBE_PLATFORM == PLATFORM_WIN32
    HANDLE port_;
#else
    struct io_uring ring_;
#endif
};
```

---

## 📊 实现难度评估

| 方案 | 工作量 | 复杂度 | 性能 | 兼容性 |
|------|--------|--------|------|--------|
| **A. 协程** | 8-12 周 | 高 | ⭐⭐⭐⭐⭐ | 中 |
| **B. Future** | 6-8 周 | 中 | ⭐⭐⭐⭐ | 中 |
| **C. 混合** | 2-4 周 | 低 | ⭐⭐⭐ | 高 |

---

## 🎯 推荐实施路径

### 阶段 1：混合模式（短期）
- 实现方案 C
- 支持 IOCP，保持兼容
- 时间：2-4 周

### 阶段 2：协程支持（中期）
- 实现轻量级协程库
- 迁移到方案 A
- 支持 io_uring
- 时间：8-12 周

### 阶段 3：性能优化（长期）
- 批量操作（io_uring 注册文件）
- 零拷贝优化
- 性能测试和调优
- 时间：持续

---

## 🔗 参考资料

- [Go runtime/netpoll.go](https://github.com/golang/go/blob/master/src/runtime/netpoll.go) - 平台无关抽象
- [Go netpoll_windows.go](https://go.dev/src/runtime/netpoll_windows.go) - IOCP 实现
- [Go netpoll_epoll.go](https://github.com/golang/go/blob/master/src/runtime/netpoll_epoll.go) - Epoll 实现
- [liburing](https://github.com/axboe/liburing) - io_uring 封装库
- [cloudwego/netpoll](https://github.com/cloudwego/netpoll/issues/407) - 讨论 io_uring 支持

---

**创建日期**: 2026-04-03
**状态**: 📐 设计方案
