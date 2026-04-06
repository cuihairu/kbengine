# 7. 并发模型、线程与内存基础设施

> 这一章回答一个反直觉的问题：为什么游戏服务器不是多线程并行处理请求？然后讲清楚主线程模型、工作线程、对象池、内存分配器和空锁策略。

## 7.1 本章核心问题

- 为什么游戏服务器的主逻辑是单线程的？
- 工作线程（DB / 文件 IO）怎么和主线程协作？
- 对象池为什么是性能关键路径？
- "空锁"策略是什么？为什么需要它？
- BigWorld 的内存基础设施比 KBEngine 做了哪些更多的事？

## 7.2 为什么游戏服务器不是多线程并行处理请求

直觉上，服务器应该"多线程处理多个玩家请求"。但 MMO 服务器做了不同选择：

**主线程单线程运行游戏逻辑。**

原因：

1. **实体状态互相依赖**：玩家 A 的攻击影响玩家 B 的血量，AOI 事件触发跨实体状态变更，加锁代价远高于串行
2. **确定性要求**：同一 tick 内的操作顺序影响结果（谁先攻击、谁先拾取），并行引入不可预测的竞态
3. **tick 模型天然串行**：10Hz tick 内先处理所有网络消息 → 执行 game tick → 属性同步 → 写库，这个流程是原子性的
4. **锁的代价太高**：每个 Entity 有几十个属性、多个定时器、可能跨 Space 的引用——细粒度锁的设计和维护成本超过收益

**结果**：Entity tick 不需要加锁，因为只有主线程在执行它。

## 7.3 主线程模型：EventDispatcher 驱动一切

```text
主线程单线程事件循环：

EventDispatcher::processOnce()
  │
  ├── processTasks()           ← 异步任务回调（DB 结果、文件 IO 完成）
  ├── processTimers()          ← 系统定时器
  │     └── 某些组件把 game tick 挂在这里
  ├── processStats()           ← 空闲统计
  └── processNetwork()         ← epoll/select 网络 I/O

EntityApp::handleGameTick()
  │
  ├── ++g_kbetime
  ├── threadPool_.onMainThreadTick()
  ├── handleTimers()           ← 脚本定时器
  └── networkInterface().processChannels(...)
```

要特别注意顺序边界：`processChannels()` 不是 `EventDispatcher::processOnce()` 统一直接调的，而是 `EntityApp` 在自己的 game tick 里主动处理主消息表。这也是“通用事件循环”和“实体型组件运行节拍”分层的地方。

**瓶颈在哪**：如果 gameTick 执行时间超过 100ms（10Hz 预算），下一帧会被延迟。这就是为什么 C++ 层要做性能关键路径（AOI、序列化、寻路），Python 脚本只做业务回调。

## 7.4 工作线程：只做 I/O 密集型任务

主线程不直接做 DB 查询、文件加载等阻塞操作。这些交给工作线程：

### KBEngine：ThreadPool + TPTask

```cpp
// 文件：kbe/src/lib/thread/threadpool.h（简化）
class ThreadPool
{
    bool addTask(TPTask* tptask);           // 投递任务
    virtual void onMainThreadTick();        // 主线程取回结果

    // 内部管理
    std::queue<TPTask*> bufferedTaskList_;  // 待处理队列
    std::list<TPTask*> finiTaskList_;       // 已完成列表
    std::list<TPThread*> busyThreadList_;   // 繁忙线程
    std::list<TPThread*> freeThreadList_;   // 空闲线程
    uint32 maxThreadCount_;                 // 上限
};
```

任务有三态返回：

```cpp
// 文件：kbe/src/lib/thread/threadtask.h
class TPTask : public Task
{
    enum TPTaskState
    {
        TPTASK_STATE_COMPLETED = 0,               // 任务完成，删除
        TPTASK_STATE_CONTINUE_MAINTHREAD = 1,     // 结果回主线程继续执行
        TPTASK_STATE_CONTINUE_CHILDTHREAD = 2,    // 继续在子线程执行
    };

    virtual TPTaskState presentMainThread() {
        return TPTASK_STATE_COMPLETED;
    }
};
```

**任务生命周期**：

```
主线程 addTask(task)
  │
  ▼
工作线程 task->process()         ← 在子线程执行（如 DB 查询）
  │
  ▼ finiTaskList_
主线程 onMainThreadTick()
  │
  ▼
task->presentMainThread()        ← 结果回到主线程（如回调 Python 脚本）
```

### BigWorld：TaskManager + BackgroundTask

```cpp
// 文件：lib/cstdmf/bgtask_manager.hpp（简化）
class BackgroundTask : public SafeReferenceCount
{
    virtual void doBackgroundTask(TaskManager& mgr,
            BackgroundTaskThread* pThread) = 0;  // 子线程执行
    virtual void doMainThreadTask(TaskManager& mgr) {} // 主线程回调
};

class TaskManager
{
    void addBackgroundTask(BackgroundTaskPtr pTask, int priority);
    void tick();  // 主线程 tick，执行 fgTaskList_

    // 带信号量的优先级任务队列
    class BackgroundTaskList {
        BW::list<std::pair<int, BackgroundTaskPtr>> list_;
        SimpleSemaphore semaphore_;
    };
};
```

BigWorld 多了**优先级支持**（`MIN=0, LOW=32, MEDIUM=64, HIGH=96, MAX=128`）和**信号量唤醒**（任务入队时信号量 push，工作线程 pull 等待）。

还有**线程私有数据**：`BackgroundThreadDataPtr pData_`，工作线程可以持有独立的 DB 连接等资源。

### 两者对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 线程池 | `ThreadPool` | `TaskManager` + `BgTaskManager` 单例 |
| 任务基类 | `TPTask` | `BackgroundTask` |
| 子线程方法 | `process()` | `doBackgroundTask()` |
| 主线程回调 | `presentMainThread()` | `doMainThreadTask()` |
| 优先级 | 无 | 有（0-128） |
| 信号量唤醒 | 条件变量 | `SimpleSemaphore` |
| 线程私有数据 | 无 | `BackgroundThreadDataPtr` |
| 文件 IO 专用 | 无 | `FileIOTaskManager`（独立线程池） |

## 7.5 线程安全策略：空锁模式

### 为什么需要空锁

对象池的模板参数可以是 `ThreadMutex`（线程安全）或 `ThreadMutexNull`（不安全但更快）：

- 如果对象池只在主线程使用 → 用 `ThreadMutexNull`，零开销
- 如果对象池可能在多线程间共享 → 用 `ThreadMutex`

```cpp
// 文件：kbe/src/lib/common/objectpool.h（简化）
template<typename T, typename THREADMUTEX = KBEngine::thread::ThreadMutexNull>
class ObjectPool { /* ... */ };

// 主线程使用（默认空锁）
ObjectPool<MemoryStream> streamPool;              // ThreadMutexNull

// 多线程使用（传入真实锁）
ObjectPool<MemoryStream, ThreadMutex> sharedPool;  // ThreadMutex
```

### KBEngine 空锁

```cpp
// 文件：kbe/src/lib/thread/threadmutex.h
class ThreadMutexNull
{
public:
    virtual void lockMutex(void) {}
    virtual void unlockMutex(void) {}
};

class ThreadMutex : public ThreadMutexNull
{
    virtual void lockMutex(void) { THREAD_MUTEX_LOCK(mutex_); }
    virtual void unlockMutex(void) { THREAD_MUTEX_UNLOCK(mutex_); }
};
```

### BigWorld 空锁

```cpp
// 文件：lib/cstdmf/concurrency.hpp
class DummyMutex
{
public:
    void grab() {}
    bool grabTry() { return true; }
    void give() {}
};

class SimpleMutex
{
    void grab() { pthread_mutex_lock(&mutex_); }
    void give() { pthread_mutex_unlock(&mutex_); }
};
```

BigWorld 的 `PoolAllocator` 同样使用模板参数选择锁策略：

```cpp
// 文件：lib/cstdmf/pool_allocator.hpp
template <class MUTEX = DummyMutex>
class PoolAllocator { /* ... */ MUTEX mutex_; };
```

**空锁模式 = 编译期多态**：不需要运行时判断是否加锁，模板实例化时直接生成有锁/无锁版本。

## 7.6 对象池：MemoryStream / Packet 的分配频率为什么是性能关键

每秒百万级消息 = 百万级 MemoryStream / Packet 分配/释放。如果每次都 `new/delete`，内存碎片和分配延迟会成为瓶颈。

### KBEngine ObjectPool

```cpp
// 文件：kbe/src/lib/common/objectpool.h（简化）
template<typename T, typename THREADMUTEX = ThreadMutexNull>
class ObjectPool
{
    T* createObject(const std::string& logPoint)
    {
        pMutex_->lockMutex();
        while (true) {
            if (obj_count_ > 0) {
                T* t = static_cast<T*>(*objects_.begin());
                objects_.pop_front();
                --obj_count_;
                t->isEnabledPoolObject(true);
                pMutex_->unlockMutex();
                return t;
            }
            assignObjs();  // 池空时批量预分配 16 个
        }
    }

    void reclaimObject(T* obj)
    {
        obj->onReclaimObject();  // 重置状态
        if (size() >= max_ || isDestroyed_)
            delete obj;          // 池满则真正释放
        else
            objects_.push_back(obj);  // 回收到池
    }

    // 瘦身：每 300 秒检查，如果池大小 > OBJECT_POOL_INIT_SIZE 则缩减
};
```

池对象基类：

```cpp
class PoolObject {
    virtual void onReclaimObject() = 0;  // 回收时重置
    bool isEnabledPoolObject_;
    std::string poolObjectCreatePoint_;  // 创建位置追踪（防泄漏）
};

// RAII 包装
template<typename T>
class SmartPoolObject {
    ~SmartPoolObject() { objectPool_.reclaimObject(pPoolObject_); }
};
```

MemoryStream 继承 `PoolObject`，可直接被对象池管理。

### BigWorld PackedObjectPool

```cpp
// 文件：lib/cstdmf/object_pool.hpp（简化）
// Handle：索引 + 世代号，防止悬垂引用
template<size_t indexBits, size_t generationBits>
struct Handle {
    baseType index_ : indexBits;
    baseType generation_ : generationBits;
};

// PackedObjectPool：紧凑排列 + swap-and-pop 删除
template<typename ObjectType, typename HandleType = DefaultHandle>
class PackedObjectPool
{
    HandleType create();           // 尾部追加
    void release(HandleType h);   // swap-and-pop（O(1)，保持紧凑）
    ObjectType& lookup(HandleType h);  // handle → 内部索引 → 对象

    // 双向映射
    HandleTable<HandleType> handles_;
    BW::LookUpTable<size_t> outerLookup_;  // handle → 内部索引
    BW::vector<size_t> innerToOuter_;      // 内部索引 → handle
    ObjectContainer innerLookup_;          // 紧凑存储的对象数组
};
```

**世代号（generation）机制**：Handle 包含 index + generation。释放时 generation+1。旧 Handle 的 generation 不匹配，查找时返回无效——**防止悬垂引用**。

### 对比

| 维度 | KBEngine ObjectPool | BigWorld PackedObjectPool |
|------|-------------------|--------------------------|
| 存储 | `std::list<T*>` 空闲链表 | 紧凑数组 + swap-and-pop |
| 查找 | 直接返回指针 | Handle（index + 世代号） |
| 悬垂保护 | 无 | 世代号校验 |
| 缓存友好性 | 差（链表分散） | 好（连续内存） |
| 瘦身 | 每 300 秒检查 | 无（紧凑排列天然高效） |
| 泄露追踪 | 有（创建位置记录） | 无 |

## 7.7 内存分配器

### KBEngine

KBEngine **没有自定义内存分配器**。直接使用标准 `new/delete` 和 `malloc/free`。

ObjectPool 是唯一的内存复用机制。高频对象（MemoryStream、Packet、Bundle）通过对象池避免频繁分配。

### BigWorld

BigWorld 有完整的内存分配体系：

```cpp
// 文件：lib/cstdmf/allocator.hpp（简化）
namespace BW::Allocator {
    void* allocate(size_t size);
    void  deallocate(void* ptr);
    void* reallocate(void* ptr, size_t size);
    // 调试：setReportOnExit, setCrashOnLeak, debugReport
    // 填充模式：CleanLandFill(0xCD), DeadLandFill(0xDD), NoMansLandFill(0xFE)
}
```

**FixedSizedAllocator**（固定大小池）：

```cpp
// 文件：lib/cstdmf/fixed_sized_allocator.hpp（简化）
class FixedSizedAllocator
{
    static const int MaxPools = 16;
    size_t allocSizes_[MaxPools];     // 每个池的分配大小
    PoolHeader* topLevelPools_[MaxPools]; // 池链表头

    void* allocate(size_t size) {
        // 找到最匹配的池，从空闲链表取
    }
};
```

**PoolAllocator**（不释放的内存池）：

```cpp
// 文件：lib/cstdmf/pool_allocator.hpp（简化）
template <class MUTEX = DummyMutex>
class PoolAllocator
{
    void* allocate(size_t size) {
        if (pHead_) { ret = pHead_; pHead_ = pHead_->next; }  // 从空闲链表取
        else { ret = new char[size]; }                          // 或新分配
    }
    void deallocate(void* p) {
        // 不真正释放，放回空闲链表
    }
};
```

**StlAllocator** — 所有 `BW::vector` 的内存都经过 `bw_new/bw_delete` 路由：

```cpp
// 文件：lib/cstdmf/bw_vector.hpp
namespace BW {
template<class T, class Allocator = StlAllocator<T>>
class vector : public std::vector<T, Allocator> { /* ... */ };
}
```

这意味着 BigWorld 可以在整个引擎层面追踪和优化内存分配。

## 7.8 智能指针与引用计数

### KBEngine

```cpp
// 文件：kbe/src/lib/common/refcountable.h
// 非线程安全
class RefCountable {
    volatile mutable long refCount_;
    void incRef() const { ++refCount_; }
    void decRef() const { if (--refCount_ <= 0) onRefOver(); /* delete this */ }
};

// 线程安全（原子操作）
class SafeRefCountable {
    // Windows: InterlockedIncrement / InterlockedDecrement
    // Linux: __sync_add_and_fetch / __sync_sub_and_fetch
};
```

### BigWorld

```cpp
// 文件：lib/cstdmf/smartpointer.hpp
class ReferenceCount { /* 非线程安全 ++count_ */ };
class SafeReferenceCount {
    // BW_ATOMIC32_INC_AND_FETCH / BW_ATOMIC32_DEC_AND_FETCH
    void decRef() const {
        int32 count = BW_ATOMIC32_DEC_AND_FETCH(&count_);
        if (count == 0) this->destroy();
    }
};
```

BigWorld 的 `BW_ATOMIC32_*` 宏封装了 Windows（`InterlockedCompareExchange`）和 GCC（`__sync_*`）的原子操作，跨平台统一。

## 7.9 同步原语工具箱

### KBEngine

| 原语 | 类 | 用途 |
|------|-----|------|
| 互斥体 | `ThreadMutex` / `ThreadMutexNull` | 空锁策略 |
| RAII 守卫 | `ThreadGuard` | 自动加解锁 |
| 引用计数 | `RefCountable` / `SafeRefCountable` | 智能指针 |

### BigWorld

| 原语 | 类 | 用途 |
|------|-----|------|
| 互斥体 | `SimpleMutex` / `DummyMutex` | 空锁策略 |
| 递归互斥体 | `RecursiveMutex` | 可重入锁 |
| 读写锁 | `ReadWriteLock` + `ReadGuard`/`WriteGuard` | 读多写少场景 |
| 信号量 | `SimpleSemaphore` | 任务队列唤醒 |
| 事件 | `SimpleEvent` | 线程间通知 |
| RAII 守卫 | `SimpleMutexHolder` | 自动加解锁 |
| 线程本地存储 | `ThreadLocal<T>` | 线程私有数据 |
| 原子操作 | `BW_ATOMIC32_*` 宏 | 跨平台原子操作 |

BigWorld 的同步原语更完整，特别是 `ReadWriteLock` 和 `ThreadLocal` 在 KBEngine 中缺失。

## 7.10 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| 线程池 | `kbe/src/lib/thread/threadpool.h` |
| 任务基类 | `kbe/src/lib/thread/threadtask.h` |
| 互斥体/空锁 | `kbe/src/lib/thread/threadmutex.h` |
| RAII 守卫 | `kbe/src/lib/thread/threadguard.h` |
| 对象池 | `kbe/src/lib/common/objectpool.h` |
| 智能指针 | `kbe/src/lib/common/smartpointer.h` |
| 引用计数 | `kbe/src/lib/common/refcountable.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| 并发原语全家桶 | `lib/cstdmf/concurrency.hpp` |
| 任务管理器 | `lib/cstdmf/bgtask_manager.hpp` |
| 对象池 | `lib/cstdmf/object_pool.hpp` |
| 内存分配器 | `lib/cstdmf/allocator.hpp` |
| 固定大小分配器 | `lib/cstdmf/fixed_sized_allocator.hpp` |
| 池分配器 | `lib/cstdmf/pool_allocator.hpp` |
| 智能指针 | `lib/cstdmf/smartpointer.hpp` |
| STL 分配器 | `lib/cstdmf/stl_fixed_sized_allocator.hpp` |

## 7.11 源码走读路径

### 路径一：理解空锁策略

1. KBEngine: `kbe/src/lib/thread/threadmutex.h` — `ThreadMutexNull` vs `ThreadMutex`
2. BigWorld: `lib/cstdmf/concurrency.hpp` — `DummyMutex` vs `SimpleMutex`
3. KBEngine: `kbe/src/lib/common/objectpool.h` — 模板参数 `THREADMUTEX` 的默认值

### 路径二：理解工作线程与主线程的协作

1. KBEngine: `kbe/src/lib/thread/threadpool.h` — `addTask()` / `onMainThreadTick()`
2. KBEngine: `kbe/src/lib/thread/threadtask.h` — `TPTaskState` 三态
3. BigWorld: `lib/cstdmf/bgtask_manager.hpp` — `BackgroundTask` 双阶段 + 优先级

### 路径三：对比对象池设计

1. KBEngine: `kbe/src/lib/common/objectpool.h` — list 空闲链表 + 瘦身
2. BigWorld: `lib/cstdmf/object_pool.hpp` — Handle 世代号 + swap-and-pop
3. BigWorld: `lib/cstdmf/pool_allocator.hpp` — 不释放的内存池

## 7.12 小结

- **主线程单线程运行游戏逻辑**，不是设计缺陷而是正确的架构选择——实体状态强依赖，加锁代价超过串行
- **工作线程只做 I/O 密集型任务**（DB 查询、文件加载），结果通过 `presentMainThread` / `doMainThreadTask` 回到主线程
- **空锁模式**通过模板参数在编译期选择有锁/无锁版本，零运行时开销
- **对象池是性能关键**：高频分配的 MemoryStream / Packet 必须池化，否则 `new/delete` 成为瓶颈
- **BigWorld 基础设施更完善**：世代号 Handle、紧凑数组对象池、FixedSizedAllocator、读写锁、线程本地存储、STL 自定义分配器
- **KBEngine 更简单直接**：无自定义内存分配器，依赖对象池 + 标准 malloc
