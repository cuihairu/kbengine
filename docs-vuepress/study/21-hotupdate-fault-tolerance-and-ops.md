# Ch21 热更新、容错与运维工具

> **核心问题**：一个线上 MMO 集群怎么做到不停机修 bug？进程挂了怎么办？怎么压测、迁移数据、防止作弊？

## 相关 API 回查

- BaseApp 管理接口：[KBEngine(baseapp)](/api/baseapp/KBEngine.md)、[Proxy(baseapp)](/api/baseapp/Proxy.md)
- Login / DB / Interfaces / Logger：[KBEngine(loginapp)](/api/loginapp/KBEngine.md)、[KBEngine(dbmgr)](/api/dbmgr/KBEngine.md)、[KBEngine(interfaces)](/api/interfaces/KBEngine.md)、[KBEngine(logger)](/api/logger/KBEngine.md)
- Bots 压测接口：[KBEngine(bots)](/api/bots/KBEngine.md)、[PyClientApp(bots)](/api/bots/PyClientApp.md)

## 21.1 脚本热重载：不停服修改 Python 行为的主要入口

### 21.1.1 为什么热重载对 MMO 如此重要

MMO 服务器 24×7 运行，一次全服停机维护意味着几十万玩家同时掉线。Python 脚本层的 bug（逻辑错误、数值配置错误、AI 行为异常）占了线上问题的大多数——这些不需要重编译 C++，只需重新加载 Python 模块。

但热重载有严格的边界：**只能改行为，不能改结构**。

### 21.1.2 KBEngine 热重载实现

KBEngine 的热重载入口是 `KBEngine.reloadScript(fullReload)`，注册在 BaseApp 和 CellApp 的 Python 模块上：

```cpp
// kbe/src/server/baseapp/baseapp.cpp
// 脚本模块注册
APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),
    reloadScript, __py_reloadScript, METH_VARARGS, 0);
```

调用链从 Python 到 C++：

```cpp
// kbe/src/lib/server/entity_app.h

template<class E>
void EntityApp<E>::reloadScript(bool fullReload)
{
    EntityDef::reload(fullReload);     // 1. 重载实体定义
    onReloadScript(fullReload);         // 2. 子类回调

    // 3. 重新调用入口脚本的 onInit(1)
    PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(),
                                        const_cast<char*>("onInit"),
                                        const_cast<char*>("i"),
                                        1);    // 1 = reload mode
    // ...
}
```

三层执行过程：

1. **`EntityDef::reload(fullReload)`**：重载实体定义模块
   - `fullReload = true`：完全重新初始化所有 `ScriptDefModule`（先 `finalise(true)` 再 `initialize()`）
   - `fullReload = false`：仅重新加载 Python 脚本模块（`loadAllEntityScriptModules`）

```cpp
// kbe/src/lib/entitydef/entitydef.cpp

void EntityDef::reload(bool fullReload)
{
    g_isReload = true;
    script::entitydef::reload(fullReload);

    if (fullReload) {
        // 保存旧的脚本模块 UType 映射
        // ...
        bool ret = finalise(true);
        KBE_ASSERT(ret && "EntityDef::reload: finalise error!");
        ret = initialize(EntityDef::__scriptBaseTypes, EntityDef::__loadComponentType);
        KBE_ASSERT(ret && "EntityDef::reload: initialize error!");
    } else {
        loadAllEntityScriptModules(EntityDef::__entitiesPath, EntityDef::__scriptBaseTypes);
    }
}
```

2. **`onReloadScript(fullReload)`**：遍历所有已存在的实体，调用每个实体的 `reload()`

```cpp
// kbe/src/server/cellapp/cellapp.cpp

void Cellapp::onReloadScript(bool fullReload)
{
    Entities<Entity>::ENTITYS_MAP& entities = pEntities_->getEntities();
    Entities<Entity>::ENTITYS_MAP::iterator eiter = entities.begin();
    for(; eiter != entities.end(); ++eiter)
    {
        static_cast<Entity*>(eiter->second.get())->reload(fullReload);
    }
    EntityApp<Entity>::onReloadScript(fullReload);
}
```

3. **入口脚本 `onInit(1)`**：通知 Python 层"重载完成"

### 21.1.3 热重载的边界

| 操作 | 能否热更 | 原因 |
|------|---------|------|
| 修改 Python 方法体 | 能 | `PyImport_ReloadModule` 重新加载 |
| 新增 Python 方法 | 能（部分） | 新方法立即可用，但 .def 声明不会更新 |
| 修改 .def 属性定义 | **不能** | 协议 ID / 持久化结构变更需要重启 |
| 修改 .def 方法签名 | **不能** | 消息 ID / 参数布局变更影响网络协议 |
| 已有实体的 C++ 状态 | **不能** | C++ 侧成员变量不受 Python 重载影响 |
| 全局变量重置 | **灰区** | `fullReload=true` 时模块级变量会被重新初始化 |

**核心规则**：热更只能改 Python 方法体内的逻辑。任何涉及实体定义结构（属性/方法/组件声明）的变更都必须重启集群。

### 21.1.4 BigWorld 的热重载

BigWorld 的热重载在 `ScriptApp` 层处理，机制与 KBEngine 类似（都是 `PyImport_ReloadModule`），但 BigWorld 还多了一层**资源热重载**：

```cpp
// BigWorld: lib/moo/reload.hpp

class Reloader
{
public:
    void onReloaded(Reloader* pSourceReloader = NULL);
    void onPreReload(Reloader* pSourceReloader = NULL);
    void registerListener(ReloadListener* pListener, bool bothDirection = true);
    void deregisterListener(ReloadListener* pListener, bool bothDirection);

    static void enable(bool enable);
    static bool enable();

private:
    typedef BW::vector<ReloadListener*> Listeners;
    SimpleMutex listenerMutex_;
    Listeners listeners_;
};

class ReloadListener
{
public:
    virtual void onReloaderReloaded(Reloader* pReloader) = 0;
    virtual void onReloaderPreReload(Reloader* pReloader) {}
};
```

BigWorld 的 `Reloader` / `ReloadListener` 是一个**观察者模式**，不仅用于脚本重载，还用于渲染资源（shader、纹理）的热更新。服务端侧主要用脚本重载部分。

---

## 21.2 容错与恢复

### 21.2.1 进程死亡是常态

在 MMO 集群中，进程死亡不是"意外"，而是"必然"。硬件故障、OOM、网络分区都可能导致 CellApp 或 BaseApp 消失。系统必须能：

1. **检测**到进程死亡
2. **恢复**受影响的实体
3. **通知**相关组件

### 21.2.2 BigWorld Reviver：自动进程守护

BigWorld 有专门的 **Reviver** 进程，职责是监控所有组件并在死亡时自动拉起：

```cpp
// BigWorld: server/reviver/reviver.hpp

class Reviver : public ServerApp, public TimerHandler,
    public Singleton< Reviver >
{
public:
    Reviver(Mercury::EventDispatcher & mainDispatcher,
            Mercury::NetworkInterface & interface);

    void shutDown();
    void revive(const char * createComponent);    // 拉起死亡组件
    bool hasEnabledComponents() const;

    virtual void handleTimeout(TimerHandle handle, void * arg);

private:
    virtual bool init(int argc, char * argv[]);
    virtual bool run();

    enum TimeoutType {
        TIMEOUT_REATTACH,
        TIMEOUT_TICK
    };

    TimerHandle      timerHandle_;
    TimerHandle      tickTimer_;
    ComponentRevivers components_;     // 监控的组件列表
    bool             shuttingDown_;
    bool             isDirty_;
};
```

Reviver 的工作机制：

1. **定期心跳**：Reviver 周期性向所有被监控组件发送 ping
2. **被监控端配合**：每个组件内嵌 `ReviverSubject`，响应 ping

```cpp
// BigWorld: lib/server/reviver_subject.hpp

class ReviverSubject : public Mercury::InputMessageHandler
{
public:
    void init(Mercury::NetworkInterface * pInterface,
              const char * componentName);
    void fini();

private:
    virtual void handleMessage(const Mercury::Address & srcAddr,
            Mercury::UnpackedMessageHeader & header,
            BinaryIStream & data);

    Mercury::NetworkInterface *  pInterface_;
    Mercury::Address             reviverAddr_;
    uint64                       lastPingTime_;
    ReviverPriority              priority_;
    int                          msTimeout_;
};
```

3. **超时判定**：如果组件在 `msTimeout_` 内没有响应 ping，Reviver 认为"死亡"
4. **优先级仲裁**：多个 Reviver 可能同时监控同一组件，`ReviverPriority` 用于仲裁由谁负责拉起——优先级最高的 Reviver 获得拉起权

```
Reviver ──ping──→ CellApp1 ──pong──→ Reviver
         ──ping──→ CellApp2 (超时无响应)
         ──revive("cellapp")──→ bwmachined ──→ 启动新 CellApp
```

**关键设计**：Reviver 不是直接 fork 进程，而是通过 `bwmachined` 间接启动——这保证了新进程的注册信息正确。

### 21.2.3 BigWorld Backup/Archive：实体的灾备

Reviver 只负责拉起进程，**实体的恢复**靠 Backup/Archive 机制：

```cpp
// BigWorld: server/baseapp/backup_sender.hpp

class BackupSender
{
public:
    void tick(const Bases & bases,
              Mercury::NetworkInterface & networkInterface);

    bool autoBackupBase(Base & base,
                        Mercury::BundleSendingMap & bundles,
                        Mercury::ReplyMessageHandler * pHandler = NULL);
    bool backupBase(Base & base,
                    Mercury::BundleSendingMap & bundles,
                    Mercury::ReplyMessageHandler * pHandler = NULL);

    void handleBaseAppDeath(const Mercury::Address & addr);

private:
    typedef BW::vector<EntityID> BasesToBackUp;
    BasesToBackUp basesToBackUp_;

    BackupHash entityToAppHash_;       // 实体 → 备份 BaseApp 映射
    BackupHash newEntityToAppHash_;
    bool       isOffloading_;
    BaseApp &  baseApp_;
};
```

```cpp
// BigWorld: server/baseapp/archiver.hpp

class Archiver
{
public:
    void tick(DBApp & dbApp, BaseAppMgrGateway & baseAppMgr,
              Bases & bases, SqliteDatabase * pSecondaryDB);

    void handleBaseAppDeath(const Mercury::Address & addr,
                             Bases & bases, SqliteDatabase * pSecondaryDB);

private:
    void restartArchiveCycle(Bases & bases);

    int                       archiveIndex_;
    BW::vector<EntityID>      basesToArchive_;
    Mercury::Address          deadBaseAppAddr_;
};
```

两级保护机制：

| 机制 | Backup | Archive |
|------|--------|---------|
| 目标 | 将 Base 实体备份到另一个 BaseApp | 将 Base 实体写入数据库 |
| 频率 | 每个 tick 备份一部分实体 | 周期性轮转归档 |
| 用途 | BaseApp 死亡后，备份 BaseApp 可恢复实体 | 持久化保障 |
| 存储位置 | 另一个 BaseApp 的内存 | MySQL / SecondaryDB |

**BaseApp 死亡时的恢复流程**：

```
1. CellAppMgr / BaseAppMgr 检测到 BaseApp 死亡
2. BackupSender::handleBaseAppDeath() 被调用
3. 从备份 BaseApp 恢复 Base 实体
4. Archiver::handleBaseAppDeath() 触发紧急归档
5. 新 BaseApp 启动后，从数据库加载实体
```

### 21.2.4 KBEngine 的容错处理

KBEngine 没有 BigWorld 那种独立的 Reviver 进程，也没有 BigWorld 风格的跨 BaseApp 备份恢复链。源码里能看到的相关机制主要有三类：

**1. EntityLog 检出机制**

DBMgr 维护一张 `EntityLog` 表，记录每个在线实体的检出状态：

```cpp
// kbe/src/server/dbmgr/dbtasks.h

class DBTaskEraseBaseappEntityLog : public DBTask
{
public:
    DBTaskEraseBaseappEntityLog(COMPONENT_ID componentID);
    virtual bool db_thread_process();
    virtual thread::TPTask::TPTaskState presentMainThread();

    virtual std::string name() const {
        return "DBTaskEraseBaseappEntityLog";
    }

protected:
    COMPONENT_ID componentID_;
    bool success_;
};
```

`DBTaskEraseBaseappEntityLog` 本身做的事情很直接：按 `componentID` 清掉这台 BaseApp 的 `_entitylog` 检出记录。它不是“标记可恢复”的复杂状态机，恢复能力主要来自旧检出记录被擦除后，后续登录能重新走分配与加载链路。

**2. Proxy 重连机制**

每个 Proxy 创建时生成一个 `rndUUID`，用于重连时的身份识别：

```cpp
// kbe/src/server/baseapp/proxy.h

class Proxy : public Entity
{
public:
    Proxy(ENTITY_ID id, const ScriptDefModule* pScriptModule);

    // 每个proxy创建之后都会由系统产生一个uuid，
    // 提供前端重登陆时用作身份识别
    INLINE uint64 rndUUID() const;
    INLINE void rndUUID(uint64 uid);

    // 将其自身所关联的客户端转给另一个proxy去关联
    void giveClientTo(Proxy* proxy);
    void onGiveClientTo(Network::Channel* lpChannel);

protected:
    uint64 rndUUID_;
    // ...
};
```

```
重连流程:
Client ──reloginBaseapp(name, password, rndUUID, entityID)──→ BaseApp
  → BaseApp 校验 Proxy 是否存在且 rndUUID 匹配
  → createClientProxies(proxy, true)
  → Proxy::onGetWitness() 重建客户端控制与视野

**3. 周期性写库 / 备份采样**

KBEngine 虽然没有 BigWorld 那种“另一台 BaseApp 保存你的可恢复副本”的 BackupSender，但 BaseApp 里确实有 `Archiver` 和 `Backuper`：

- `Archiver::tick()` 会按 `baseapp/archivePeriod` 分批调用 `entity.writeToDB(NULL, NULL, NULL)`
- `Backuper::tick()` 会按 `baseapp/backupPeriod` 分批调用 `entity.writeBackupData(&s)`

这两者更多是“周期性收束实体状态”和“向 Base 收集备份流”的内部机制，不等同于 BigWorld 完整的跨进程灾备恢复方案。
```

**3. 与 BigWorld 的对比**

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 进程守护 | 无内建 Reviver，通常依赖外部进程管理 | Reviver 进程自动拉起 |
| 实体备份 | 有 `Backuper` / `writeBackupData`，但不是 BigWorld 式跨 BaseApp 灾备 | BackupSender 跨 BaseApp 备份 |
| 实体归档 | `writeToDB` + `Archiver` 周期触发 | Archiver 周期归档 + SecondaryDB |
| 重连识别 | rndUUID | 类似机制 |
| EntityLog | DBMgr 维护在线检出记录 | 主要靠 Backup / checkout 链 |

**KBEngine 的弱点**：没有内建 Reviver，进程死亡后的拉起通常要交给 systemd、supervisor 或自研运维脚本；同时它缺少 BigWorld 那种成熟的跨 BaseApp 热备恢复路径，所以 BaseApp 非正常死亡时，更依赖最近一次写库/归档结果。

---

## 21.3 Bots 压测系统

### 21.3.1 为什么需要 Bots

MMO 服务器性能测试不能用真人——需要自动化工具模拟大量玩家行为，包括登录、移动、战斗、交互。两套项目都有内置的 Bots 系统。

### 21.3.2 KBEngine Bots

KBEngine 的 Bots 是一个独立的客户端进程，内部嵌入了完整的客户端 SDK：

```cpp
// kbe/src/server/tools/bots/bots.h

class Bots : public ClientApp
{
public:
    Bots(Network::EventDispatcher& dispatcher,
         Network::NetworkInterface& ninterface,
         COMPONENT_TYPE componentType,
         COMPONENT_ID componentID);

    virtual bool initialize();
    virtual void handleGameTick();

    // 添加bots（由 guiconsole 或 telnet 触发）
    virtual void addBots(Network::Channel * pChannel, MemoryStream& s);

    // 完整的客户端协议处理
    virtual void onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s);
    virtual void onEntityEnterWorld(Network::Channel * pChannel, MemoryStream& s);
    virtual void onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s);
    virtual void onUpdatePropertys(Network::Channel* pChannel, MemoryStream& s);
    // ... 更多客户端接口

    static PyObject* __py_addBots(PyObject* self, PyObject* args);

protected:
    PyBots* pPyBots_;                         // Python 控制接口
    CLIENTS clients_;                          // 所有 bot 客户端
    uint32 reqCreateAndLoginTotalCount_;       // 总创建数
    uint32 reqCreateAndLoginTickCount_;        // 每 tick 创建数
    float  reqCreateAndLoginTickTime_;         // tick 时间间隔
    CreateAndLoginHandler* pCreateAndLoginHandler_;  // 创建/登录调度器
};
```

Bots 的创建由 Python 驱动：

```cpp
// kbe/src/server/tools/bots/bots.cpp

void Bots::addBots(Network::Channel * pChannel, MemoryStream& s)
{
    uint32 reqCreateAndLoginTotalCount;
    uint32 reqCreateAndLoginTickCount = 0;
    float reqCreateAndLoginTickTime = 0;

    s >> reqCreateAndLoginTotalCount;
    reqCreateAndLoginTotalCount_ += reqCreateAndLoginTotalCount;
    // ...
}
```

`PyBots` 提供了 Python 字典接口，可以查询所有 bot 的状态：

```cpp
// kbe/src/server/tools/bots/pybots.h

class PyBots : public script::ScriptObject
{
public:
    DECLARE_PY_MOTHOD_ARG1(pyHas_key, ENTITY_ID);
    DECLARE_PY_MOTHOD_ARG0(pyKeys);
    DECLARE_PY_MOTHOD_ARG0(pyValues);
    DECLARE_PY_MOTHOD_ARG0(pyItems);

    static PyObject* __py_pyGet(PyObject * self, PyObject * args, PyObject* kwds);
    static PyMappingMethods mappingMethods;
};
```

KBEngine Bots 的特点：
- **继承 ClientApp**：复用了完整的客户端网络栈
- **Python 可编程**：通过 `PyBots` 和 Python 脚本控制 bot 行为
- **渐进式创建**：`CreateAndLoginHandler` 每个 tick 创建一定数量的 bot，避免瞬间冲击

### 21.3.3 BigWorld Bots

BigWorld 的 Bots 系统更丰富，包含多种行为控制器：

```cpp
// BigWorld: server/tools/bots/bot_entity.cpp

class BotEntity : public Entity
{
public:
    BotEntity(const ClientApp & clientApp, const EntityType & type);
    // ...
};
```

**直线移动控制器**（从 A 点到 B 点）：

```cpp
// BigWorld: server/tools/bots/beeline_controller.cpp

class BeelineController
{
public:
    BeelineController(const Vector3 &destinationPos) :
        destinationPos_(destinationPos) {}

    bool nextStep(float &speed, float dTime,
                  Vector3 &pos, Direction3D &dir)
    {
        float distance = speed * dTime;
        Vector3 destVec = destinationPos_ - pos;

        if (destVec.length() > distance)
        {
            pos += destVec * (distance / destVec.length());
            dir.yaw = destVec.yaw();
        }
        return true;
    }

private:
    Vector3 destinationPos_;
};
```

**锯齿巡逻控制器**（在区域内随机巡逻）：

```cpp
// BigWorld: server/tools/bots/zigzag_patrol_graph.cpp

namespace ZigzagPatrol
{
const int ZigzagGraphTraverser::DEFAULT_CORRIDOR_WIDTH = 50;

ZigzagGraphTraverser::ZigzagGraphTraverser(const Graph &graph,
                                           float &speed,
                                           Vector3 &startPosition,
                                           float corridorWidth) :
    GraphTraverser(graph, speed, startPosition, true, true),
    corridorWidth_(corridorWidth)
{
    sourcePos_ = startPosition;
}
}
```

BigWorld Bots 的特点：
- **多种行为模式**：直线移动、锯齿巡逻、跟随导航图
- **可插拔控制器**：不同的 `Controller` 类实现不同的行为
- **更接近真实玩家**：随机移动路径 + 巡逻区域模拟

### 21.3.4 两套项目的对比

| 维度 | KBEngine Bots | BigWorld Bots |
|------|--------------|---------------|
| 基础架构 | 继承 ClientApp，复用客户端 SDK | 独立 bot_entity |
| 行为控制 | Python 脚本驱动 | 内置多种 Controller |
| 移动模式 | 脚本自定义 | 直线 / 锯齿巡逻 / 导航图 |
| 创建方式 | `addBots(total, tickCount, tickTime)` | 类似 |
| Python 接口 | PyBots 字典操作 | py_bots |

---

## 21.4 安全机制

### 21.4.1 Exposed 方法的信任边界

在 Ch11 中我们提到 EntityCall 有 Exposed 方法。这里从安全角度审视：

- **Exposed 方法**可以被客户端直接调用（通过 Client → BaseApp / CellApp 路径）
- **非 Exposed 方法**只能被服务端组件调用

```
信任边界:

Client ──只能调用──→ Exposed 方法（需验证参数合法性）
Server ──可调用全部──→ 所有方法（内部可信）
```

设计原则：**永远不要信任客户端传来的数据**。Exposed 方法内部必须做参数校验。

### 21.4.2 登录挑战（Login Challenge）：反自动登录

BigWorld 实现了**Proof-of-Work 登录挑战**机制，防止自动化脚本暴力登录：

```cpp
// BigWorld: lib/connection/login_challenge.hpp

class LoginChallenge : public SafeReferenceCount
{
public:
    virtual bool writeChallengeToStream(BinaryOStream & data) = 0;
    virtual bool readChallengeFromStream(BinaryIStream & data) = 0;
    virtual bool writeResponseToStream(BinaryOStream & data) = 0;
    virtual bool readResponseFromStream(BinaryIStream & data) = 0;

protected:
    LoginChallenge() : SafeReferenceCount() {}
};
```

这是一个**挑战-响应**协议：

```
Server                          Client
  │                               │
  │─── challenge data ───────────→│  服务端发送计算难题
  │                               │  客户端执行耗时计算
  │←── response data ────────────│  客户端提交计算结果
  │                               │
  │─── 验证通过，允许登录 ────────→│
```

BigWorld 实现了 **Cuckoo Cycle** 作为具体的 PoW 算法：

```cpp
// BigWorld: lib/connection/cuckoo_cycle_login_challenge_factory.hpp

class CuckooCycleLoginChallengeFactory : public LoginChallengeFactory
{
public:
    bool configure(const LoginChallengeConfig & config) /* override */;
    LoginChallengePtr create() /* override */;

    void easiness(double value) {
        easiness_ = std::max(0.0, std::min(100.0, value));
    }
    double easiness() const { return easiness_; }

private:
    double easiness_;    // 难度参数，0-100
    static const double DEFAULT_EASINESS;
};
```

**Cuckoo Cycle** 是一种内存硬的 PoW 算法——计算需要大量内存访问，GPU 加速效果有限。`easiness` 参数控制难度：值越低，计算越耗时。

**KBEngine 没有内置 PoW 登录挑战**，这部分安全逻辑需要脚本层自行实现。

### 21.4.3 加密通道（EncryptionFilter）

两套项目都实现了网络层加密过滤器。

**KBEngine Blowfish 加密**：

```cpp
// kbe/src/lib/network/encryption_filter.h

class EncryptionFilter : public PacketFilter
{
public:
    virtual void encrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
    virtual void decrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
};

class BlowfishFilter : public EncryptionFilter, public KBEBlowfish
{
public:
    virtual Reason send(Channel * pChannel, PacketSender& sender,
                        Packet * pPacket, int userarg);
    virtual Reason recv(Channel * pChannel, PacketReceiver & receiver,
                        Packet * pPacket);

    void encrypt(Packet * pInPacket, Packet * pOutPacket);
    void decrypt(Packet * pInPacket, Packet * pOutPacket);
};
```

**BigWorld EncryptionFilter**：

```cpp
// BigWorld: lib/network/encryption_filter.hpp

class EncryptionFilter : public PacketFilter
{
public:
    static EncryptionFilterPtr create(BlockCipherPtr pCipher);

    virtual Reason send(PacketSender & packetSender,
                        const Address & addr, Packet * pPacket);
    virtual Reason recv(PacketReceiver & receiver,
                        const Address & addr, Packet * pPacket,
                        ProcessSocketStatsHelper * pStatsHelper);

    void encryptStream(MemoryOStream & clearStream,
                       BinaryOStream & cipherStream);
    bool decryptStream(BinaryIStream & cipherStream,
                       BinaryOStream & clearStream);

private:
    BlockCipherPtr pBlockCipher_;    // 可插拔的加密算法
};
```

两者差异：

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 加密算法 | Blowfish（固定） | BlockCipher（可插拔） |
| 架构 | 直接继承加密类 | 组合模式，BlockCipherPtr 注入 |
| 应用场景 | LoginApp → Client 通道 | Mercury 任意 Channel |

加密通道的典型应用场景：

1. **登录阶段**：保护用户名/密码传输
2. **密钥交换**：LoginApp 将 BaseApp 的加密密钥安全传给客户端
3. **敏感数据传输**：充值、交易等操作

### 21.4.4 rndUUID：会话身份识别

KBEngine 的 `Proxy` 使用 `rndUUID` 做重连时的身份识别：

```cpp
// kbe/src/server/baseapp/proxy.h

class Proxy : public Entity
{
    // 每个proxy创建之后都会由系统产生一个uuid，
    // 提供前端重登陆时用作身份识别
    INLINE uint64 rndUUID() const;
    INLINE void rndUUID(uint64 uid);

    void giveClientTo(Proxy* proxy);    // 控制权转移

protected:
    uint64 rndUUID_;                     // 64位随机UUID
};
```

`rndUUID` 的作用：

1. **重连验证**：客户端断线后携带 `rndUUID` 重连，BaseApp 据此找到对应的 Proxy
2. **防伪造**：64 位随机值，暴力猜中概率极低
3. **一次性**：重连成功后 `rndUUID` 会刷新，防止重放攻击

**giveClientTo** 允许将一个 Proxy 的客户端连接转给另一个 Proxy——这是实现"挤号"（同一账号踢掉旧连接）的基础。

---

## 21.5 数据迁移工具

### 21.5.1 BigWorld 数据迁移

BigWorld 提供了三个数据库迁移工具：

**consolidate_dbs**：合并数据库

```
// BigWorld: server/tools/consolidate_dbs/main.cpp
// 将多个 SecondaryDatabase 的数据合并到 PrimaryDatabase
```

典型场景：多台从库数据汇总到主库。

**transfer_db**：数据库迁移

```
// BigWorld: server/tools/transfer_db/transfer_db.cpp
// 将数据从一台数据库服务器迁移到另一台
```

典型场景：硬件升级、机房迁移。

**sync_db**：数据库同步

```
// BigWorld: server/tools/sync_db/main.cpp
// 在主库和从库之间同步数据
```

典型场景：从库数据修复、灾备同步。

这三个工具构成了 BigWorld 的数据库运维工具链，配合 `PrimaryDatabase / SecondaryDatabase` 架构使用。

### 21.5.2 KBEngine 数据迁移

KBEngine 没有独立的数据迁移工具。数据库操作通过以下方式管理：

- **EntityDef 变更**：修改 `.def` 文件中的属性定义，重启时 DBMgr 自动更新表结构
- **脚本层迁移**：通过 `executeRawDatabaseCommand` 执行 SQL
- **备份/恢复**：依赖外部工具（mysqldump / redis-cli）

---

## 21.6 日常运维

### 21.6.1 配置管理

两套项目都使用 XML 配置文件：

- **KBEngine**：`kbengine.xml` + `kbengine_defaults.xml`，分层覆盖
- **BigWorld**：`bw.xml` + `bw.local.xml`，类似分层

关键配置项：

| 类别 | 配置 | 说明 |
|------|------|------|
| 网络端口 | `telnet_port` / `externalTcpPort` | 各组件监听端口 |
| 性能 | `tick_interval` / `io_tick_interval` | tick 频率 |
| 安全 | `telnet_passwd` / `encryption` | Telnet 密码、加密设置 |
| 容错 | `reviver` 配置 | Reviver 心跳超时 |
| 数据库 | `dbMgr` / `database` | 数据库连接参数 |

### 21.6.2 日志轮转

- **KBEngine**：Logger 组件集中收集日志，`buffered_logs_` 缓冲后写入文件
- **BigWorld**：message_logger 双格式存储（binary + text），按组件维度隔离

### 21.6.3 性能基线建立

利用 Ch20 中的可观测性工具：

1. **Watcher 快照**：定期采集关键指标（实体数、tick 耗时、带宽）作为基线
2. **Profile 报告**：在正常负载下运行 Profiler，记录各函数耗时的基准值
3. **Bots 压测**：用 Bots 模拟目标在线人数，验证性能是否达标
4. **回归对比**：每次版本更新后重新跑 Bots 压测，对比性能基线

---

## 21.7 源码入口表

### KBEngine

| 模块 | 文件路径 | 关键类/函数 |
|------|---------|-----------|
| 热重载入口 | `kbe/src/lib/server/entity_app.h` | `EntityApp::reloadScript()`, `EntityApp::onReloadScript()` |
| EntityDef 重载 | `kbe/src/lib/entitydef/entitydef.cpp` | `EntityDef::reload()` |
| CellApp 重载 | `kbe/src/server/cellapp/cellapp.cpp` | `Cellapp::reloadScript()`, `Cellapp::onReloadScript()` |
| BaseApp 重载 | `kbe/src/server/baseapp/baseapp.cpp` | `Baseapp::reloadScript()`, `Baseapp::onReloadScript()` |
| EntityLog | `kbe/src/server/dbmgr/dbtasks.h` | `DBTaskEraseBaseappEntityLog` |
| DBMgr 容错 | `kbe/src/server/dbmgr/dbmgr.cpp` | BaseApp 死亡检测与 EntityLog 清理 |
| Proxy 重连 | `kbe/src/server/baseapp/proxy.h` | `Proxy::rndUUID()`, `Proxy::giveClientTo()` |
| 加密过滤器 | `kbe/src/lib/network/encryption_filter.h` | `EncryptionFilter`, `BlowfishFilter` |
| Bots 主类 | `kbe/src/server/tools/bots/bots.h` | `Bots` (继承 `ClientApp`) |
| Bots Python 接口 | `kbe/src/server/tools/bots/pybots.h` | `PyBots` |
| Bots 创建调度 | `kbe/src/server/tools/bots/create_and_login_handler.h` | `CreateAndLoginHandler` |
| Bots 客户端对象 | `kbe/src/server/tools/bots/clientobject.h` | `ClientObject` |

### BigWorld

| 模块 | 文件路径 | 关键类/函数 |
|------|---------|-----------|
| Reviver 主进程 | `server/reviver/reviver.hpp` | `Reviver` |
| Reviver 被监控端 | `lib/server/reviver_subject.hpp` | `ReviverSubject` |
| Reviver 通用 | `lib/server/reviver_common.hpp` | `ReviverPriority` |
| Backup 发送 | `server/baseapp/backup_sender.hpp` | `BackupSender` |
| Archive 归档 | `server/baseapp/archiver.hpp` | `Archiver` |
| 资源热重载 | `lib/moo/reload.hpp` | `Reloader`, `ReloadListener` |
| 登录挑战接口 | `lib/connection/login_challenge.hpp` | `LoginChallenge` |
| Cuckoo Cycle PoW | `lib/connection/cuckoo_cycle_login_challenge_factory.hpp` | `CuckooCycleLoginChallengeFactory` |
| 登录挑战工厂 | `lib/connection/login_challenge_factory.hpp` | `LoginChallengeFactory` |
| 加密过滤器 | `lib/network/encryption_filter.hpp` | `EncryptionFilter`, `BlockCipherPtr` |
| Bot 实体 | `server/tools/bots/bot_entity.cpp` | `BotEntity` |
| 直线控制器 | `server/tools/bots/beeline_controller.cpp` | `BeelineController` |
| 锯齿巡逻 | `server/tools/bots/zigzag_patrol_graph.cpp` | `ZigzagGraphTraverser` |
| 数据库合并 | `server/tools/consolidate_dbs/main.cpp` | `consolidate_dbs` |
| 数据库迁移 | `server/tools/transfer_db/transfer_db.cpp` | `transfer_db` |
| 数据库同步 | `server/tools/sync_db/main.cpp` | `sync_db` |

---

## 21.8 源码漫游路径

### 路径 A：从 reloadScript 到实体重载

```
1. Python 层调用
   → KBEngine.reloadScript(fullReload)

2. kbe/src/server/baseapp/baseapp.cpp
   → Baseapp::__py_reloadScript() → Baseapp::reloadScript()

3. kbe/src/lib/server/entity_app.h
   → EntityApp::reloadScript()
   → EntityDef::reload(fullReload)
     → entitydef.cpp: finalise(true) → initialize()（完全重载）
     → 或 loadAllEntityScriptModules()（仅脚本重载）

4. kbe/src/server/baseapp/baseapp.cpp
   → Baseapp::onReloadScript()
   → 遍历所有 Entity，调用 entity->reload()

5. 入口脚本
   → onInit(1) 被 Python 调用，通知重载完成
```

### 路径 B：从 Reviver ping 到进程拉起

```
1. server/reviver/reviver.hpp
   → Reviver::handleTimeout() → 定期 ping 所有组件

2. lib/server/reviver_subject.hpp
   → ReviverSubject::handleMessage() → 响应 ping
   → 如果超时无响应 → 判定死亡

3. server/reviver/reviver.hpp
   → Reviver::revive("cellapp") → 通过 bwmachined 启动新组件

4. 新组件启动后
   → 向 bwmachined 注册 → 向 Mgr 组件注册 → 开始工作

5. server/baseapp/backup_sender.hpp
   → BackupSender::handleBaseAppDeath() → 从备份恢复实体
```

### 路径 C：从客户端加密到安全登录

```
1. kbe/src/lib/network/encryption_filter.h
   → BlowfishFilter 在 Channel 上安装

2. kbe/src/server/loginapp/loginapp.cpp
   → 登录请求经过加密通道传输

3. kbe/src/server/baseapp/proxy.h
   → Proxy 创建 → rndUUID_ = genUUID64()
   → rndUUID 通过加密通道传给客户端

4. 客户端断线重连
   → 携带 rndUUID → LoginApp → BaseApp
   → BaseApp 查找匹配的 Proxy → 恢复会话
```

---

## 21.9 小结

| 子系统 | KBEngine | BigWorld | 业界对比 |
|--------|---------|---------|---------|
| **脚本热重载** | `reloadScript(fullReload)` + `EntityDef::reload()` | 类似 + Reloader/ReloadListener | Kubernetes rolling update（更重） |
| **进程守护** | 无（外部脚本） | Reviver 自动拉起 | systemd / supervisor |
| **实体备份** | 无 | BackupSender 跨 BaseApp 备份 | Raft / Paxos 日志复制 |
| **实体归档** | writeToDB 直写 | Archiver 周期归档 + SecondaryDB | WAL + 定期 checkpoint |
| **EntityLog** | DBMgr 维护检出状态 | 无（靠 Backup 恢复） | 分布式锁 / lease |
| **Bots 压测** | ClientApp 继承 + Python 控制 | 多种行为 Controller | Locust / JMeter |
| **登录挑战** | 无 | Cuckoo Cycle PoW | CAPTCHA / reCAPTCHA |
| **加密通道** | BlowfishFilter（固定算法） | EncryptionFilter（可插拔 BlockCipher） | TLS / mTLS |
| **数据迁移** | 无独立工具 | consolidate_dbs / transfer_db / sync_db | mysqldump / pg_dump |
| **重连识别** | rndUUID（64位随机） | 类似 | JWT / Session Token |

**核心差异**：BigWorld 的容错体系更完整——Reviver 自动拉起 + BackupSender 跨进程备份 + Archiver 周期归档形成三级保障。KBEngine 依赖 DBMgr 的 EntityLog 做检出恢复，粒度更粗。

**共同短板**：两套项目都没有现代的配置中心（如 Consul / etcd）和自动化运维流水线（如 CI/CD + 滚动更新），运维仍以手动操作 + 脚本为主。
