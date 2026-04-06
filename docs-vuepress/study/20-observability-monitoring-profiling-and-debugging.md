# Ch20 可观测性：监控、性能分析与调试

> **核心问题**：在一个运行中的 MMO 集群里，你怎么知道系统现在健康不健康？出了问题怎么定位？

## 20.1 可观测性三支柱在游戏服务器中的映射

业界将可观测性（Observability）拆解为三根支柱：**Metrics（指标）**、**Tracing（链路追踪）**、**Logging（日志）**。在 BigWorld / KBEngine 中，这三根支柱的对应关系如下：

| 支柱 | 业界标准 | BigWorld / KBEngine 对应 | 完备度 |
|------|---------|------------------------|--------|
| **Metrics** | Prometheus + Grafana | Watcher + ProfileVal / SendingStats | 有基础指标，无时序存储和告警 |
| **Tracing** | OpenTelemetry Tracing | **无**分布式链路追踪 | 缺失 |
| **Logging** | ELK / Loki | Logger / message_logger | 有集中日志，无结构化查询 |

两套项目的可观测性体系是**面向开发调试**设计的，而非面向生产运维。本章逐一剖析每个子系统的实现，并在最后讨论如何接入现代可观测性栈。

---

## 20.2 Watcher 系统：游戏服务器的"metrics 端点"

### 20.2.1 设计理念

Watcher 的核心思想极其简洁：**任意 C++ 变量或函数返回值，都可以注册到一棵路径式的观察树中，通过远程连接实时查询**。

```
components/
├── cellapp1/
│   ├── entities/            → 实体总数
│   ├── tickProfile/         → tick 耗时统计
│   └── aoIUpdates/          → AOI 更新次数
├── baseapp1/
│   └── ...
└── dbmgr/
    └── ...
```

这与 Prometheus 的 `/metrics` 端点本质上是同一模式——**拉取式**（查询时取值）。但 Watcher 没有时序存储、没有聚合、没有告警——只有当前值快照。

### 20.2.2 KBEngine Watcher 实现

KBEngine 的 Watcher 体系由以下四个模板类构成：

```cpp
// kbe/src/lib/helper/watcher.h

// 基类：路径式标识 + 序列化接口
class WatcherObject {
public:
    WatcherObject(std::string path);
    virtual void addToInitStream(MemoryStream* s) {};
    virtual void addToStream(MemoryStream* s) {};
    const char* path() { return path_.c_str(); }
    const char* name() { return name_.c_str(); }
    virtual WATCHER_VALUE_TYPE getType() { return WATCHER_VALUE_TYPE_UNKNOWN; }
protected:
    std::string path_, name_, strval_;
    WATCHER_ID id_;
    int32 numWitness_;   // 当前观察者数量
};

// 监视一个值（引用绑定）
template <class T>
class WatcherValue : public WatcherObject {
public:
    WatcherValue(std::string path, const T& pVal)
      : WatcherObject(path), watchVal_(pVal) {}
    void addToStream(MemoryStream* s) { (*s) << id_ << watchVal_; }
    T getValue() { return watchVal_; }
protected:
    const T& watchVal_;  // 绑定到被监视变量的引用
};

// 监视一个自由函数的返回值
template <class RETURN_TYPE>
class WatcherFunction : public WatcherObject {
    typedef RETURN_TYPE(*FUNC)();
    void addToStream(MemoryStream* s) { (*s) << id_ << (*func_)(); }
protected:
    FUNC func_;
};

// 监视一个成员函数的返回值
template <class RETURN_TYPE, class OBJ_TYPE>
class WatcherMethod : public WatcherObject {
    typedef RETURN_TYPE(OBJ_TYPE::*FUNC)();
    void addToStream(MemoryStream* s) { (*s) << id_ << (obj_->*func_)(); }
protected:
    FUNC func_;
    OBJ_TYPE* obj_;
};
```

这四层抽象的含义：

| 类 | 用途 | 典型场景 |
|----|------|---------|
| `WatcherObject` | 基类，定义序列化接口 | — |
| `WatcherValue<T>` | 绑定一个 C++ 变量的引用 | 监视实体数量 `numEntities_` |
| `WatcherFunction<R>` | 绑定一个自由函数 | 监视全局统计函数返回值 |
| `WatcherMethod<R, OBJ>` | 绑定一个成员函数 | 监视 `Component::tickTime()` |

所有 Watcher 注册到 `Watchers` 容器中，由 `WatcherPaths` 提供路径式的层次管理：

```cpp
// kbe/src/lib/helper/watcher.h

class Watchers {
public:
    static Watchers& rootWatchers();
    typedef KBEUnordered_map<std::string, KBEShared_ptr<WatcherObject>> WATCHER_MAP;
    bool addWatcher(const std::string& path, WatcherObject* pwo);
    KBEShared_ptr<WatcherObject> getWatcher(const std::string& name);
    void addToStream(MemoryStream* s);  // 序列化所有 watcher 值
};

class WatcherPaths {
public:
    static WatcherPaths& root();
    typedef KBEUnordered_map<std::string, KBEShared_ptr<WatcherPaths>> WATCHER_PATHS;
    bool addWatcher(std::string path, WatcherObject* pwo);
    void dirPath(std::string path, std::vector<std::string>& vec);
};
```

`WatcherPaths` 构成一棵路径树——`root/components/cellapp1/entities` 这种结构在内部被逐级拆解为嵌套的 `WatcherPaths` 节点。

**值类型系统**：KBEngine 定义了 14 种 watcher 值类型（`WATCHER_VALUE_TYPE_*`），从 `UINT8` 到 `COMPONENT_TYPE`，覆盖了所有基础 C++ 类型。序列化时值类型随数据一起传递，客户端据此解析。

### 20.2.3 BigWorld Watcher 实现（更完整）

BigWorld 的 Watcher 体系更为成熟，增加了**网络接入层**和**分布式转发**两个关键能力。

**WatcherNub——网络接入层**：

```cpp
// BigWorld: lib/network/watcher_nub.hpp

enum WatcherMsg {
    WATCHER_MSG_GET = 16,          // 查询 watcher 值
    WATCHER_MSG_SET = 17,          // 设置 watcher 值
    WATCHER_MSG_TELL = 18,         // 返回 watcher 值
    WATCHER_MSG_GET2 = 26,         // V2 协议
    WATCHER_MSG_SET2 = 27,
    WATCHER_MSG_TELL2 = 28,
    WATCHER_MSG_SET2_TELL2 = 29,
    WATCHER_MSG_EXTENSION_START = 107
};

struct WatcherRegistrationMsg {
    int  version;
    int  uid;
    int  message;     // WATCHER_...
    int  id;          // e.g. 14
    char abrv[32];    // e.g. "cell14"
    char name[64];    // e.g. "Cell 14"
};
```

`WatcherNub` 同时支持 TCP 和 UDP 监听。每个组件启动时通过 `WatcherRegistrationMsg` 向 Mgr 注册自己的 watcher 端口，Mgr 即可通过 GET/SET 消息远程查询和修改任意组件的 watcher 值。

**ForwardingWatcher——分布式转发**：

这是 BigWorld Watcher 最强大的特性：

```cpp
// BigWorld: lib/server/watcher_forwarding.hpp

class ForwardingWatcher : public Watcher {
public:
    enum ExposeHints {
        WITH_ENTITY = 0,     // 组件拥有特定实体
        CELL_APPS,           // 所有 CellApp
        WITH_SPACE,          // 特定 Space 的所有组件
        LEAST_LOADED,        // 负载最低的组件
        LOCAL_ONLY,          // 仅本地
        BASE_APPS,           // 所有 BaseApp
        SERVICE_APPS,        // 所有 ServiceApp
        BASE_SERVICE_APPS,   // 所有 Base + Service App
    };

    virtual ForwardingCollector* newCollector(
        WatcherPathRequestV2& pathRequest,
        const BW::string& destWatcher,
        const BW::string& targetInfo) = 0;
};
```

`ForwardingWatcher` 挂载在 CellAppMgr / BaseAppMgr 上。当 Mgr 收到 watcher 查询请求时：

1. 解析路径中的 `ExposeHints`（如 `CELL_APPS` 表示查所有 CellApp）
2. 创建 `ForwardingCollector`
3. `ForwardingCollector` 通过 Mercury 向所有目标组件广播 GET 请求
4. 收集所有回复，汇总后返回给调用者

```
查询者 ──GET──→ CellAppMgr ──GET──→ CellApp1
                              ├──GET──→ CellApp2
                              ├──GET──→ CellApp3
                ←──TELL──  (汇总)  ←──TELL── ...
```

**这意味着运维人员可以从 CellAppMgr 一站式看到所有 CellApp 的状态，无需逐台登录。** 这是 KBEngine 所不具备的能力。

### 20.2.4 与 Prometheus 的对比

| 维度 | Watcher | Prometheus |
|------|---------|-----------|
| 模式 | 拉取式（查询时取值） | 拉取式（定期 scrape） |
| 时序存储 | 无 | 内置 TSDB |
| 聚合 | 无 | PromQL 聚合查询 |
| 告警 | 无 | AlertManager |
| 分布式 | BigWorld 有转发；KBEngine 无 | Service Discovery + Federation |
| 定位 | 开发调试工具 | 生产监控系统 |

如果要将 Watcher 接入 Prometheus，只需写一个 exporter 定期拉取 Watcher 值推给 pushgateway，或者在组件上暴露 HTTP `/metrics` 端点。

---

## 20.3 Profiler：函数级性能剖析

### 20.3.1 KBEngine Profile / ProfileVal

KBEngine 的 Profiler 采用**栈式嵌套**设计，能精确测量函数间的耗时关系：

```cpp
// kbe/src/lib/helper/profile.h

class ProfileVal {
public:
    void start() {
        TimeStamp now = timestamp();
        if (inProgress_++ == 0)
            lastTime_ = now;          // 记录第一次进入的时间

        ProfileGroup::PROFILEVALS& stack = pProfileGroup_->stack();

        // 如果栈中有对象，说明从上一个函数进入了本函数
        // 取上一个函数到本函数之间耗时的"内部时间"
        if (!stack.empty()) {
            ProfileVal& profile = *stack.back();
            profile.lastIntTime_ = now - profile.lastIntTime_;
            profile.sumIntTime_ += profile.lastIntTime_;
        }
        stack.push_back(this);
        lastIntTime_ = now;
    }

    void stop(uint32 qty = 0) {
        TimeStamp now = timestamp();
        // inProgress_ 为 0 时，计算本函数的总耗时
        if (--inProgress_ == 0) {
            lastTime_ = now - lastTime_;
            sumTime_ += lastTime_;
        }
        lastQuantity_ = qty;
        sumQuantity_ += qty;
        ++count_;

        ProfileGroup::PROFILEVALS& stack = pProfileGroup_->stack();
        stack.pop_back();
        lastIntTime_ = now - lastIntTime_;
        sumIntTime_ += lastIntTime_;

        // 恢复上层函数的内部时间起点
        if (!stack.empty())
            stack.back()->lastIntTime_ = now;
    }

    // 关键字段
    std::string  name_;
    TimeStamp    lastTime_;       // 最近一次总耗时
    TimeStamp    sumTime_;        // 累计总耗时
    TimeStamp    lastIntTime_;    // 最近一次内部耗时
    TimeStamp    sumIntTime_;     // 累计内部耗时
    uint32       count_;          // 调用次数
    int          inProgress_;     // 重入计数
};
```

这里有两组时间度量：

- **总耗时**（`lastTime_` / `sumTime_`）：从 `start()` 到 `stop()` 的完整时间，包含所有子调用
- **内部耗时**（`lastIntTime_` / `sumIntTime_`）：仅本函数自身消耗的时间，**排除**子 ProfileVal 的耗时

这个区分非常重要——它能告诉你一个函数本身花了多少时间 vs 它调用的子函数花了多少时间。

**ScopedProfile RAII 宏**：

```cpp
// kbe/src/lib/helper/profile.h

class ScopedProfile {
public:
    ScopedProfile(ProfileVal& profile, const char* filename, int lineNum)
      : profile_(profile), filename_(filename), lineNum_(lineNum) {
        profile_.start();
    }
    ~ScopedProfile() { profile_.stop(filename_, lineNum_); }
};

// 使用方式
#define AUTO_SCOPED_PROFILE(NAME)                   \
    static ProfileVal _localProfile(NAME);          \
    ScopedProfile _autoScopedProfile(_localProfile, __FILE__, __LINE__);

#define START_PROFILE(PROFILE)  PROFILE.start();
#define STOP_PROFILE(PROFILE)   PROFILE.stop(__FILE__, __LINE__);
```

`AUTO_SCOPED_PROFILE` 在函数开头声明即可，离开作用域时自动 stop。`static` 保证 `ProfileVal` 只初始化一次，后续调用复用同一对象累积统计。

**CellApp 组件级 Profile 组**：

```cpp
// kbe/src/server/cellapp/profile.h（示例，各组件独立定义）

// cellapp 中定义的关键 ProfileVal：
extern ProfileVal ON_MOVE_PROFILE;
extern ProfileVal ON_NAVIGATE_PROFILE;
extern ProfileVal CLIENT_UPDATE_PROFILE;
extern ProfileVal AOI_UPDATE_PROFILE;
// ... 更多
```

每个组件在 `profile.h` 中声明一组全局 `ProfileVal`，在关键路径上用 `START_PROFILE` / `STOP_PROFILE` 包裹，运行时通过 Watcher 查询。

### 20.3.2 BigWorld Profiler（更细致）

BigWorld 的 Profiler 提供了**多层粒度**的性能追踪：

**核心 Profiler（内存环形缓冲区）**：

```cpp
// BigWorld: lib/cstdmf/profiler.hpp

class Profiler {
public:
    enum ProfileMode {
        PROFILE_OFF = 0,
        HIERARCHICAL,       // 层次化显示
        SORT_BY_TIME,       // 按耗时排序
        SORT_BY_NUMCALLS,   // 按调用次数排序
        SORT_BY_NAME,       // 按名称排序
        CPU_GPU,            // CPU/GPU 分离显示
        GRAPHS,             // 图形化
        CORES,              // 按核心显示
    };

    enum EventType {
        EVENT_START,        // 计时开始
        EVENT_START_IDLE,   // 空闲计时开始
        EVENT_END,          // 计时结束
        EVENT_COUNTER       // 计数器事件
    };

    enum EventCategory {
        CATEGORY_CPP    = (1 << 0),
        CATEGORY_GPU    = (1 << 1),
        CATEGORY_PYTHON = (1 << 2),
        CATEGORY_ALL    = (1 << 3) - 1
    };

    void addEntry(const char* text, EventType e,
                  int32 value, EventCategory category);
    void tick();
    void setProfileMode(ProfileMode mode, bool inclusive);
    void getStatistics(BW::vector<ProfileLine>* profileOutput);
};
```

BigWorld 的 Profiler 使用**内存环形缓冲区**记录事件，支持按 C++ / GPU / Python 分类，有层次化、按耗时排序等多种输出模式。

**EntityProfiler——单个实体的性能追踪**：

```cpp
// BigWorld: lib/server/entity_profiler.hpp

class EntityProfiler {
public:
    void start() const;
    void stop() const;
    void tick(uint64 tickDtInStamps,
              float smoothingFactor,
              EntityTypeProfiler& typeProfiler);

    float load() const { return currAdjustedLoad_; }
    float rawLoad() const { return currRawLoad_; }
    float maxRawLoad() const { return maxRawLoad_; }

    // RAII 自动作用域追踪
    template <class ENTITY>
    class AutoScopedHelper {
    public:
        AutoScopedHelper(const ENTITY* pEntity) : pEntity_() {
            if (pEntity) {
                pEntity_ = pEntity;
                pEntity->profiler().start();
            }
        }
        ~AutoScopedHelper() {
            if (pEntity_) pEntity_->profiler().stop();
        }
    private:
        ConstSmartPointer<ENTITY> pEntity_;
    };

private:
    mutable uint64 elapsedTickTime_;
    float currAdjustedLoad_;   // 指数平滑后的负载
    float currRawLoad_;        // 原始负载
    float maxRawLoad_;         // 最大负载
};

// 使用宏
#define AUTO_SCOPED_THIS_ENTITY_PROFILE  \
    EntityProfiler::AutoScopedHelper<BaseOrEntity> _autoEntityProfile(this)
```

`EntityProfiler` 是**实体粒度**的负载追踪。每个 tick 开始时 `start()`，实体处理完成时 `stop()`，`tick()` 方法将本 tick 的耗时转化为负载值并进行**指数平滑**（smoothing），然后上报给 `EntityTypeProfiler`。

**EntityTypeProfiler——按实体类型聚合**：

```cpp
// BigWorld: lib/server/entity_type_profiler.hpp

class EntityTypeProfiler {
public:
    void addEntityLoad(float entitySmoothedLoad,
                       float entityRawLoad,
                       float entityAddedLoad);
    void tick();

    float load() const { return currSmoothedLoad_; }
    float rawLoad() const { return currRawLoad_; }
    int numEntities() const { return numEntities_; }

private:
    float currSmoothedLoad_;  // 该类型的平均平滑负载
    float currRawLoad_;
    float currAddedLoad_;
    int numEntities_;         // 该类型的实体数量
};
```

**CellProfiler——Cell 级负载**：

```cpp
// BigWorld: server/cellapp/cell_profiler.hpp

class CellProfiler {
public:
    void addEntityLoad(float entitySmoothedLoad, float entityRawLoad);
    void tick();
    float load() const { return currSmoothedLoad_; }
    float rawLoad() const { return currRawLoad_; }
    float maxRawLoad() const { return maxRawLoad_; }

private:
    float currSmoothedLoad_;
    float currRawLoad_;
    float maxRawLoad_;
    float accSmoothedLoad_;  // 一个 tick 内的累积值
    float accRawLoad_;
};
```

**三级 Profiler 的数据流向**：

```
EntityProfiler (per-entity)
    │ tick() → addEntityLoad()
    ↓
EntityTypeProfiler (per-type)     ← 聚合同类型的实体负载
    │
    ↓
CellProfiler (per-cell)           ← 汇总所有类型 → Cell 负载
    │
    ↓
doBalance()                       ← 基于 Cell 负载做动态均衡
```

这个三级体系让运维可以精确定位性能瓶颈在**哪个实体的哪个类型**上——是 NPC 太多了？还是 AOI 触发器太密了？ entityTypeProfiler 直接告诉你答案。

### 20.3.3 与 pprof / perf 的关系

| 维度 | 引擎内置 Profiler | pprof / perf |
|------|-------------------|-------------|
| 粒度 | 逻辑级（哪个函数/实体类型慢） | 系统级（CPU 火焰图） |
| 语言感知 | 知道实体类型、Watcher 路径 | 只知道 C++ 函数符号 |
| 开销 | 极低（纳秒级时间戳） | 采样开销可控 |
| 输出 | Watcher 树 + ProfileVal 值 | 火焰图 / top / callgraph |

两者互补：引擎 Profiler 知道"哪个实体类型 tick 慢"，pprof 知道"哪个系统调用慢"。

---

## 20.4 NetworkStats：网络层统计

### 20.4.1 KBEngine NetworkStats

```cpp
// kbe/src/lib/network/network_stats.h

class NetworkStats : public Singleton<NetworkStats> {
public:
    enum S_OP { SEND, RECV };

    struct Stats {
        std::string name;
        uint32 send_size;    // 发送总字节数
        uint32 send_count;   // 发送次数
        uint32 recv_size;    // 接收总字节数
        uint32 recv_count;   // 接收次数
    };

    typedef KBEUnordered_map<std::string, Stats> STATS;

    void trackMessage(S_OP op, const MessageHandler& msgHandler, uint32 size);
    STATS& stats() { return stats_; }

private:
    STATS stats_;
};
```

每条网络消息经过 `trackMessage()` 时，按消息名聚合 `send_count` / `send_size` / `recv_count` / `recv_size`。这是一个**消息级**的统计，能告诉你哪种消息占了多少带宽。

### 20.4.2 BigWorld SendingStats

```cpp
// BigWorld: lib/network/sending_stats.hpp

class SendingStats : public TimerHandler {
public:
    double bitsPerSecond() const;
    double packetsPerSecond() const;
    double messagesPerSecond() const;

    uint numPacketsSent() const;
    uint numBundlesSent() const;
    uint numPacketsResent() const;     // 重传统计
    uint numPiggybacks() const;        // 捎带统计
    uint numFailedBundleSend() const;  // 发送失败统计
    uint numFailedPacketSend() const;

private:
    Stat numBytesSent_;
    Stat numBytesResent_;
    Stat numPacketsSent_;
    Stat numPacketsResent_;
    Stat numPiggybacks_;
    Stat numBytesPiggybacked_;
    Stat numBundlesSent_;
    Stat numMessagesSent_;
    Stat numReliableMessagesSent_;
    Stat numFailedPacketSend_;
    Stat numFailedBundleSend_;
};
```

BigWorld 的 `SendingStats` 提供了更丰富的网络指标，包括**每秒比特率**（bitsPerSecond）、**每秒包数**（packetsPerSecond）、**每秒消息数**（messagesPerSecond），以及可靠传输的重传统计和 piggyback 统计。这些是定位网络瓶颈的关键指标。

---

## 20.5 Telnet 调试控制台

### 20.5.1 KBEngine Telnet 实现

```cpp
// kbe/src/lib/server/telnet_handler.h

class TelnetHandler : public Network::InputNotificationHandler {
public:
    enum TELNET_STATE {
        TELNET_STATE_PASSWD,    // 密码认证
        TELNET_STATE_ROOT,      // 根命令模式
        TELNET_STATE_PYTHON,    // Python 解释器模式
        TELNET_STATE_READONLY,  // 只读模式
        TELNET_STATE_QUIT       // 退出
    };

    TelnetHandler(Network::EndPoint* pEndPoint,
                  TelnetServer* pTelnetServer,
                  Network::NetworkInterface* pNetworkInterface,
                  TELNET_STATE defstate = TELNET_STATE_ROOT);
};
```

Telnet 状态机的工作流程：

1. **TELNET_STATE_PASSWD**：输入密码认证
2. **TELNET_STATE_ROOT**：可执行 C++ 层命令（查看 watcher、查看 profile 等）
3. **TELNET_STATE_PYTHON**：进入 Python 解释器，可直接操作实体、调用脚本方法
4. **TELNET_STATE_READONLY**：会话中可切换的只读模式，仅能查询不能修改

每个组件（CellApp、BaseApp、DBMgr 等）启动时都会创建 `TelnetServer`：

```cpp
// kbe/src/server/cellapp/cellapp.cpp

pTelnetServer_ = new TelnetServer(&this->dispatcher(), &this->networkInterface());
pTelnetServer_->pScript(&this->getScript());
pTelnetServer_->start(g_kbeSrvConfig.getCellApp().telnet_passwd,
                       g_kbeSrvConfig.getCellApp().telnet_deflayer,
                       g_kbeSrvConfig.getCellApp().telnet_port);
```

`telnet_deflayer` 在 `TelnetServer::start()` 里实际只识别 `"python"` 和默认的 `root`。`readonly`/`quit` 是会话内状态，不是启动默认层。它的价值主要是在线排查和临时 profile，不是对外的长期运维 API。

---

## 20.6 Message Logger：集中日志

### 20.6.1 KBEngine Logger

```cpp
// kbe/src/server/tools/logger/logger.h（简化）

struct LOG_ITEM {
    int uid;
    int logtype;
    int componentType;
    int componentID;
    double t;
    KBEngine::KBETime kbetime;
    std::string logstream;
    bool persistent;           // 是否持久化
};

class Logger {
public:
    void writeLog(Network::Channel* pChannel, KBEngine::MemoryStream& s);
    void registerLogWatcher(Network::Channel* pChannel, KBEngine::MemoryStream& s);
    void sendInitLogs(LogWatcher& logWatcher);
    size_t bufferedLogsSize();

private:
    std::deque<LOG_ITEM*> buffered_logs_;  // 缓冲最近一批日志指针
};
```

所有组件的日志通过 `LoggerInterface::writeLog` 汇聚到 Logger 组件。`writeLog()` 会把日志格式化为统一字符串，先回调入口脚本的 `onLogWrote()`，再按需持久化，并广播给已注册的 `LogWatcher`。

### 20.6.2 BigWorld message_logger（更完整）

BigWorld 的 message_logger 是一个完整的日志聚合系统：

```cpp
// BigWorld: server/tools/message_logger/logger.hpp

class Logger : public WatcherRequestHandler, public TimerHandler {
public:
    bool init(int argc, char* argv[]);
    bool handleNextMessage();
    bool shouldLogPriority(MessageLogger::NetworkMessagePriority priority);
protected:
    virtual void processExtensionMessage(int messageID,
            char* data, int dataLen, WatcherEndpoint& watcherEndpoint);
};

// 日志条目
struct LogEntry {
    LogEntry(const struct timeval& tv,
             MessageLogger::UserComponentID componentID, ...);
    const struct timeval& time() const;
    int categoryID() const;
    int messagePriority() const;
    int stringOffset() const;
};
```

**双格式存储**：`binary_file_handler`（二进制，高效写入）和 `text_file_handler`（文本，可读性好）。

**按组件维度隔离**：`user_components.hpp` 管理每个进程的注册信息（名称、PID、地址），日志按组件维度隔离存储。

**Python 可查询**：`py_bwlog` / `py_query` / `py_query_result` 提供 Python 接口，支持按组件、级别、时间、主机等维度查询历史日志。

**结构化日志**：`categories.hpp` / `log_string_interpolator.hpp` 提供日志分类和字符串插值，比 KBEngine 的纯文本日志更结构化。

---

## 20.7 ServerInfo：硬件与系统信息

```cpp
// BigWorld: lib/server/server_info.hpp

class ServerInfo {
public:
    const BW::string& serverName() const { return serverName_; }
    const BW::string& cpuInfo() const { return cpuInfo_; }
    const CpuSpeeds& cpuSpeeds() const { return cpuSpeeds_; }
    const BW::string& memInfo() const { return memInfo_; }
    uint64 memTotal() const { return memTotal_; }
    uint64 memUsed() const { return memUsed_; }
    void updateMem();    // 运行时刷新内存使用

private:
    BW::string serverName_;
    BW::string cpuInfo_;
    BW::vector<float> cpuSpeeds_;
    BW::string memInfo_;
    uint64 memTotal_;
    uint64 memUsed_;
};
```

运维时需要知道"这个 CellApp 跑在什么机器上、CPU 什么型号、内存用了多少"。`ServerInfo` 提供了这些硬件层面的上下文信息，与 Watcher 中的逻辑指标互补。

---

## 20.8 分布式链路追踪：两套项目共同的缺失

### 20.8.1 问题

一次 EntityCall 的完整链路可能跨越多个进程：

```
BaseApp (脚本调用 EntityCall)
  → CellApp (处理 RPC)
    → 另一个 CellApp (ghost 转发)
      → BaseApp (回调脚本)
```

但整个链路**没有 traceID**。运维只能靠日志时间戳 + 经验手动拼链路。

### 20.8.2 OpenTelemetry Tracing 能做什么

1. 给每次 EntityCall 分配 `traceID` / `spanID`
2. 跨进程传递 trace context（通过 Bundle 头部注入 W3C TraceContext）
3. 在 Jaeger / Zipkin 中可视化完整调用链

### 20.8.3 接入难点

1. **高频调用**：每秒百万级 EntityCall → sampling 策略必须精心设计（头部一致性采样 / 尾部自适应采样）
2. **同步 tick 内的串行调用**：span 嵌套关系比微服务更复杂——一个 tick 内可能有数百个 EntityCall
3. **改动较深**：需要在 `MemoryStream` / `Bundle` 层注入 trace context，侵入消息协议

---

## 20.9 OTel Metrics 接入的可能性

更准确地说，Watcher 已经提供了可拉取的指标树，但它不是 Prometheus / OTel 原生协议，所以通常需要一个自定义 exporter 或 collector 适配层去查询 watcher tree，再转换成 Metrics。关键指标清单：

| 指标 | 来源 | 告警阈值 |
|------|------|---------|
| 实体数量（按类型/按组件） | Watcher | 突增/突降 |
| tick 耗时（P50/P99） | ProfileVal | > 100ms → tick 堆积 |
| AOI 更新耗时 | ProfileVal | 持续增长 |
| 网络带宽（收/发 bytes/s） | NetworkStats / SendingStats | 接近带宽上限 |
| DB 写入延迟 | ProfileVal | > 500ms → 数据库瓶颈 |
| Channel 数量 | Watcher | 突降 → 大量断线 |
| 对象池使用率 | ObjectPool | > 80% → 内存压力 |

---

## 20.10 常见调试场景

### 场景 1：定位属性同步延迟

```
Watcher → 查 tickProfile
  → Profile → 查 Witness::update 耗时
    → NetworkStats → 查带宽是否饱和
```

### 场景 2：定位 RPC 链路瓶颈

```
Profiler → 查 EntityCall 处理耗时
  → NetworkStats → 查消息队列深度
    → EntityTypeProfiler → 查哪个实体类型最耗时
```

### 场景 3：定位 AOI 性能问题

```
Profile → 查 RangeTrigger 耗时
  → Watcher → 查实体数量是否异常增长
    → EntityTypeProfiler → 查实体类型分布
```

### 场景 4：定位数据库写库延迟

```
Profile → 查 writeToDB 耗时
  → Watcher → 查 ThreadPool 队列长度
    → ServerInfo → 查内存是否不足
```

---

## 20.11 源码入口表

### KBEngine

| 模块 | 文件路径 | 关键类/函数 |
|------|---------|-----------|
| Watcher 基类 | `kbe/src/lib/helper/watcher.h` | `WatcherObject`, `WatcherValue<T>`, `WatcherFunction<R>`, `WatcherMethod<R,OBJ>` |
| Watcher 容器 | `kbe/src/lib/helper/watcher.h` | `Watchers`, `WatcherPaths` |
| Profile 核心 | `kbe/src/lib/helper/profile.h` | `ProfileVal`, `ProfileGroup`, `ScopedProfile` |
| Profile 宏 | `kbe/src/lib/helper/profile.h` | `AUTO_SCOPED_PROFILE`, `START_PROFILE`, `STOP_PROFILE` |
| 网络统计 | `kbe/src/lib/network/network_stats.h` | `NetworkStats`, `Stats` |
| Telnet 服务 | `kbe/src/lib/server/telnet_server.h` | `TelnetServer` |
| Telnet 处理 | `kbe/src/lib/server/telnet_handler.h` | `TelnetHandler`, `TELNET_STATE` |
| Logger | `kbe/src/server/tools/logger/logger.h` | `Logger`, `LOG_ITEM` |
| 对象池 | `kbe/src/lib/common/objectpool.h` | `ObjectPool<T>` |

### BigWorld

| 模块 | 文件路径 | 关键类/函数 |
|------|---------|-----------|
| Watcher 网络 | `lib/network/watcher_nub.hpp` | `WatcherNub`, `WatcherMsg` 枚举 |
| 分布式转发 | `lib/server/watcher_forwarding.hpp` | `ForwardingWatcher`, `ExposeHints` |
| 转发收集器 | `lib/server/watcher_forwarding_collector.hpp` | `ForwardingCollector` |
| 核心 Profiler | `lib/cstdmf/profiler.hpp` | `Profiler`, `ProfileMode`, `HitchDetector` |
| 实体 Profiler | `lib/server/entity_profiler.hpp` | `EntityProfiler`, `AutoScopedHelper` |
| 类型 Profiler | `lib/server/entity_type_profiler.hpp` | `EntityTypeProfiler` |
| Cell Profiler | `server/cellapp/cell_profiler.hpp` | `CellProfiler` |
| CellApp Profile | `server/cellapp/profile.hpp` | `CellProfileGroup`, 各种 ProfileVal |
| 网络统计 | `lib/network/sending_stats.hpp` | `SendingStats` |
| 服务器信息 | `lib/server/server_info.hpp` | `ServerInfo` |
| Message Logger | `server/tools/message_logger/logger.hpp` | `Logger`, `Component` |
| 日志条目 | `server/tools/message_logger/log_entry.hpp` | `LogEntry` |
| 二进制处理 | `server/tools/message_logger/binary_file_handler.hpp` | `BinaryFileHandler` |
| 文本处理 | `server/tools/message_logger/text_file_handler.hpp` | `TextFileHandler` |
| 组件管理 | `server/tools/message_logger/user_components.hpp` | `UserComponents` |
| Python 查询 | `server/tools/message_logger/py_bwlog.hpp` | `PyBWLog` |

---

## 20.12 源码漫游路径

### 路径 A：从 Watcher 注册到远程查询

```
1. kbe/src/lib/helper/watcher.h
   → WatcherValue<T> 构造，绑定变量引用

2. kbe/src/lib/helper/watcher.h
   → WatcherPaths::addWatcher() 注册到路径树

3. kbe/src/server/cellapp/cellapp.cpp
   → initializeWatcher() 中注册组件级 watcher

4. guiconsole / telnet
   → 发起查询 → addToStream() 序列化 → 网络 → 客户端显示
```

### 路径 B：从 Profiler 宏到负载均衡决策

```
1. 函数入口
   → AUTO_SCOPED_PROFILE("onTick") → ScopedProfile 构造 → ProfileVal::start()

2. 函数出口
   → ScopedProfile 析构 → ProfileVal::stop()

3. tick 结束
   → ProfileGroup::runningTime() 汇总

4. BigWorld: EntityProfiler::tick()
   → 计算平滑负载 → 上报 EntityTypeProfiler → CellProfiler → doBalance()
```

### 路径 C：从日志产生到集中存储

```
1. 组件中 DEBUG_MSG / INFO_MSG 宏
   → 格式化日志字符串

2. kbe/src/server/tools/logger/logger.h
   → writeLog() 接收 → buffered_logs_ 缓冲

3. BigWorld: message_logger/
   → Logger::handleNextMessage() → LogEntry 创建 → UserComponents 路由
   → binary_file_handler / text_file_handler 双写
```

---

## 20.13 小结

| 子系统 | KBEngine | BigWorld | 业界对比 |
|--------|---------|---------|---------|
| **Watcher（指标）** | 单机路径树，guiconsole/telnet 查询 | + WatcherNub 网络层 + ForwardingWatcher 分布式转发 | Prometheus 有 TSDB + 告警 + PromQL |
| **Profiler（剖析）** | ProfileVal 栈式嵌套，函数级 | + EntityProfiler / EntityTypeProfiler / CellProfiler 三级体系 | pprof 提供系统级火焰图，互补 |
| **NetworkStats** | 消息级收发统计 | + bps/pps/mps + 重传 + piggyback | Prometheus node_exporter 网络指标 |
| **Telnet** | 4 状态机，Python 解释器 | 类似 | kubectl exec 远程 shell |
| **Logger** | 集中收集 + LogWatcher 过滤 | + 双格式存储 + 组件隔离 + Python 查询 | ELK 有全文索引 + 聚合 |
| **Tracing** | 缺失 | 缺失 | OpenTelemetry + Jaeger |

**核心差异**：BigWorld 的 `ForwardingWatcher` 实现了从 Mgr 组件一站式查询所有下属组件的能力，这在 KBEngine 中没有对应实现。BigWorld 的三级 Profiler 体系（Entity → EntityType → Cell）也比 KBEngine 的单一 `ProfileVal` 层次更丰富。

**共同缺失**：两套项目都没有分布式链路追踪，无法自动关联一次 EntityCall 跨越多个进程的完整链路。这是接入现代可观测性栈时优先级最高的改进方向。
