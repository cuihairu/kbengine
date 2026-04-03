# 成熟网络库的 Reactor/Proactor 兼容性设计分析

## 📚 研究目标

研究成熟的跨平台网络库如何同时兼容 Reactor（Select/Epoll）和 Proactor（IOCP）两种模式。

---

## 🔍 各库的设计方案

### 1. Boost.Asio（C++）

**核心策略：统一 Proactor 模式**

```cpp
// 用户代码看起来是异步的
async_read(socket, buffer, [](error_code ec, size_t n) {
    // 处理数据
});

// 底层实现：
// Windows: 真正的 IOCP（Proactor）
// Linux:   epoll（Reactor）+ 模拟层 → 变成 Proactor 语义
// macOS:   kqueue（Reactor）+ 模拟层
```

**关键设计：**

1. **统一接口 - Proactor 语义**
   ```cpp
   // 所有平台都用异步操作
   async_read/sync_read
   async_write/sync_write
   ```

2. **Linux 上的 Reactor→Proactor 适配**
   ```
   用户发起 async_read
   → 内部在 epoll 注册 EPOLLIN
   → epoll_wait 返回
   → 内部调用 read()
   → 调用用户 handler（模拟"异步完成"）
   ```

3. **Windows 上使用原生 IOCP**
   ```
   用户发起 async_read
   → 直接调用 WSARecv（真异步）
   → GetQueuedCompletionStatus
   → 调用用户 handler
   ```

**优点：**
- ✅ 用户代码统一，都是异步风格
- ✅ 充分利用平台能力（Windows 真异步，Linux 模拟）
- ✅ API 一致性好

**缺点：**
- ⚠️ Linux 上不是真异步（仍然需要 read()）
- ⚠️ 需要复杂的适配层

---

### 2. libuv（Node.js 核心）

**核心策略：统一的异步 API + 平台特定实现**

```javascript
// 用户代码（JavaScript）
fs.readFile('path', (err, data) => {
    // 处理数据
});
```

**底层实现（C）：**

```c
// Linux
uv__io_start(loop, poll, UV__POLLIN);
epoll_wait(loop->ep, events, timeout);
read();  // 同步读取，然后调用 callback

// Windows
CreateIoCompletionPort();
WSARecv();  // 真异步
GetQueuedCompletionStatus();

// macOS
kqueue();
read();  // 同步读取
```

**关键设计：**

1. **事件循环统一**
   ```c
   uv_run(loop, UV_RUN_DEFAULT);
   ```

2. **平台抽象层**
   ```
   +-------------------+
   |   libuv API       |  ← 统一接口
   +-------------------+
   | Platform Layer    |
   +-------------------+
   | epoll | kqueue | IOCP |
   +-------------------+
   ```

3. **I/O 模型差异处理**
   - **Windows**: 真异步（IOCP）
   - **Linux/macOS**: 非阻塞 I/O + 事件通知

**优点：**
- ✅ API 极其简洁
- ✅ 跨平台一致性最好
- ✅ 成熟稳定（支撑 Node.js）

**缺点：**
- ⚠️ 非 Windows 平台不是真异步
- ⚠️ C API，回调地狱

---

### 3. Netty（Java）

**核心策略：坚持 Reactor 模式**

**Netty 的选择：**
- **主要使用 Reactor 模式**（所有平台）
- **放弃 Windows IOCP 的 Proactor 优势**

**原因（官方 Issue #2515）：**

1. Linux AIO 实现不完整
2. AIO 相比 NIO 性能提升不明显
3. Windows 很少用作服务器

**架构设计：**

```
┌─────────────────────────────────┐
│   ChannelPipeline (统一)         │
├─────────────────────────────────┤
│   EventLoop (Reactor)            │
├─────────────────────────────────┤
│   Linux: NIO (epoll)            │
│   Windows: NIO (select)         │  ← Windows 上也用 Reactor
│   macOS: NIO (kqueue)           │
└─────────────────────────────────┘
```

**平台差异：**

| 平台 | 传输层 | 性能 |
|------|--------|------|
| **Linux** | Native epoll (JNI) | ⭐⭐⭐⭐⭐ |
| **Windows** | NIO Selector | ⭐⭐⭐ |
| **macOS** | NIO kqueue | ⭐⭐⭐⭐ |

**关键发现：**
- Netty 曾经尝试过 Windows AIO/Proactor
- **后来删除了**（版本 4.0.0.CR3）
- 理由是"不值得增加复杂度"

**优点：**
- ✅ 架构简单一致
- ✅ Reactor 模式容易理解
- ✅ 跨平台行为可预测

**缺点：**
- ❌ Windows 上无法利用 IOCP 优势
- ❌ Windows 性能相对较差

---

### 4. ACE Framework（C++）

**核心策略：同时提供两套 API**

**Reactor API：**
```cpp
class MyHandler : public ACE_Event_Handler {
public:
    int handle_input(ACE_HANDLE fd) {
        char buffer[1024];
        ssize_t n = ACE_OS::read(fd, buffer, sizeof(buffer));
        // 处理数据
        return 0;
    }
};

ACE_Reactor::instance()->register_handler(handler, ACE_Event_Handler::READ_MASK);
```

**Proactor API：**
```cpp
class MyHandler : public ACE_Handler {
public:
    void handle_read(const ACE_Asynch_Read_Stream::Result &result) {
        // 数据已经读好了
        process_data(result.message_block());
    }
};

ACE_Asynch_Read_Stream reader;
reader.read(*message_block, ...);
```

**关键设计：**

1. **两套独立的 API**
   - `ACE_Reactor` / `ACE_Event_Handler`（Reactor）
   - `ACE_Proactor` / `ACE_Handler`（Proactor）

2. **用户选择使用哪套**
   - 需要性能 → Proactor（Windows）
   - 需要兼容性 → Reactor（所有平台）

3. **平台实现**
   - Windows: Proactor 用 IOCP
   - Linux: Proactor 用 AIO（或者 O_SYNC）
   - 其他: 只支持 Reactor

**优点：**
- ✅ 给用户完全的控制权
- ✅ Windows 上可以真正利用 IOCP

**缺点：**
- ❌ 两套 API，学习成本高
- ❌ 代码不兼容（选了 Reactor 不能切换到 Proactor）

---

## 📊 方案对比总结

| 库 | 核心策略 | 统一性 | 性能 | 复杂度 |
|-----|---------|--------|------|--------|
| **Boost.Asio** | 统一 Proactor | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | 高 |
| **libuv** | 统一异步 API | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | 中 |
| **Netty** | 统一 Reactor | ⭐⭐⭐⭐ | ⭐⭐⭐ | 低 |
| **ACE** | 双 API | ⭐⭐ | ⭐⭐⭐⭐⭐ | 高 |

---

## 💡 对 KBEngine 的启示

### 方案选择分析

#### 方案 1：统一 Proactor（Boost.Asio 路线）

```cpp
// 修改 EventPoller 接口
class EventPoller {
public:
    // 旧接口（Reactor）
    virtual bool triggerRead(int fd) = 0;

    // 新接口（Proactor）
    virtual void asyncRead(int fd, Buffer* buf, Callback cb) = 0;
};
```

**优点：**
- ✅ Windows 真异步，高性能
- ✅ API 统一

**缺点：**
- ❌ 破坏性改动，需要修改所有 Channel 代码
- ❌ Linux/macOS 模拟异步，增加复杂度

#### 方案 2：统一 Reactor（Netty 路线）

```cpp
// 保持现有接口，Windows 上用 Reactor 模拟
class IocpPoller : public EventPoller {
    bool doRegisterForRead(int fd) override {
        // 预读数据到缓存
        WSARecv(fd, cachedBuffer[fd], ...);
        return true;
    }

    int processPendingEvents(double maxWait) override {
        GetQueuedCompletionStatus(...);
        // 触发 handler
        triggerRead(fd);
    }
};

// 修改 recv 从缓存返回
int recv(int fd, void* buf, size_t size) {
    if (hasCachedData(fd)) {
        return getCachedData(fd, buf, size);
    }
    return ::recv(fd, buf, size);
}
```

**优点：**
- ✅ 最小改动
- ✅ 保持现有架构
- ✅ 兼容性好

**缺点：**
- ❌ Windows 不是真异步（预读模式）
- ❌ 性能提升有限

#### 方案 3：双 API（ACE 路线）

```cpp
// 保留现有 Reactor API
class ReactorEventPoller { ... };

// 新增 Proactor API
class ProactorEventPoller {
public:
    virtual void asyncRead(int fd, Buffer* buf, Callback cb) = 0;
};

// 用户选择使用哪个
```

**优点：**
- ✅ 不破坏现有代码
- ✅ Windows 可以真异步

**缺点：**
- ❌ 两套 API，维护成本高
- ❌ 用户需要选择和学习

---

## 🎯 推荐方案

基于 KBEngine 的实际情况，**推荐方案 2（统一 Reactor）**：

### 理由

1. **Netty 的经验**：Netty 曾经尝试 Proactor，后来放弃了
2. **最小改动**：不需要修改大量现有代码
3. **足够好**：对于大多数应用，预读模式性能已足够

### 实现路径

```
阶段 1：预读模式（2-4 周）
├─ Windows 上 IOCP 预读
├─ recv() 从缓存返回
└─ 验证性能提升

阶段 2：性能优化（4-6 周）
├─ 批量操作
├─ 零拷贝优化
└─ 压力测试

阶段 3（可选）：Proactor API（8-12 周）
├─ 如果确实需要更高性能
├─ 添加新的 Proactor API
└─ 保持 Reactor API 兼容
```

---

## 🔗 参考资料

### Boost.Asio
- [Boost.Asio Documentation](https://www.boost.org/doc/libs/release/libs/asio/)
- [Asio Design Overview](https://think-async.com/Asio/AsioDesign)

### libuv
- [libuv Design Documentation](https://docs.libuv.org/en/v1.x/design.html)
- [libuv GitHub Repository](https://github.com/libuv/libuv)
- [How libuv bridges Node.js and OS](https://shoaibamin.com/blog/from-javascript-to-c-how-libuv-bridges-nodejs-and-the-operating-system)

### Netty
- [Netty Native Transports](https://netty.io/wiki/native-transports.html)
- [Netty NIO.2/AIO Discussion (Issue #2515)](https://github.com/netty/netty/issues/2515)
- [聊聊 Netty 的 Reactor 实现](https://www.cnblogs.com/binlovetech/p/16440319.html)

### ACE
- [ACE Framework Official Site](https://www.dre.vanderbilt.edu/~schmidt/ACE.html)
- [ACE Reactor/Proactor Tutorial](https://www.dre.vanderbilt.edu/~schmidt/ACE.html)

### Comparison Articles
- [epoll, kqueue and IOCP explained](https://medium.com/@sachinklocham/the-os-level-magic-behind-of-millions-of-connections-epoll-kqueue-and-iocp-explained-ce7889d31580)
- [Reactor vs Proactor Pattern](https://www.artima.com/articles/io-design-pattern-reactor-vs-proactor)

---

**创建日期**: 2026-04-03
**状态**: 📐 方案调研完成
