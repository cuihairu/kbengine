# KBEngine 2.0 重设计方案

> 这是一份人工整理的重构草案，不来自 `docs/api/kbengine/kbengine_api(cn).chm` 或 `docs/WebConsole_Guide(cn).pdf` 原文。
> 
> 文中内容属于设计提案与工程判断，不应视为 KBEngine 官方文档结论。

## 一、进程模型

保留 BigWorld 的多进程分布式架构：

```
┌─────────────────────────────────────────────────────┐
│                    Machine (机器管理)                  │
│                  进程守护、自动重启、资源上报            │
└─────────────────────────────────────────────────────┘

┌──────────┐     ┌──────────────┐     ┌──────────────┐
│ LoginApp │────▶│    BaseApp    │◀───▶│    CellApp   │
│ 登录认证  │     │  实体持久化    │     │  空间/物理    │
│ 账号管理  │     │  客户端代理    │     │  视野/Witness │
│          │     │  非空间逻辑   │     │  移动/导航    │
└──────────┘     └──────┬───────┘     └──────────────┘
                        │
                 ┌──────┴───────┐
                 │    DBMgr     │
                 │  数据库抽象   │
                 │  Entity 持久化│
                 └──────────────┘

┌──────────┐     ┌──────────────┐
│BaseAppMgr│     │  CellAppMgr  │
│ 负载均衡  │     │   负载均衡    │
│ App 管理  │     │   App 管理   │
└──────────┘     └──────────────┘

辅助：Logger(日志) / Interfaces(第三方) / Bots(测试)
```

**保留**：每个进程职责清晰，LoginApp/BaseApp/CellApp/DBMgr 四大件不变。

**改进**：
- 进程间通信从自定义 Bundle 打包改为 Schema 驱动的二进制协议 + 共享内存通道
- 组件发现从 UDP 广播改为 gossip 协议 + Machine 协调，支持跨网段部署

---

## 二、Def 系统（声明式实体定义）

这是简化开发的核心机制，保留并增强。

### 文件结构

```
entity_defs/
├── entities.yaml          # 实体注册表
├── types.yaml             # 自定义类型
├── interfaces/            # 接口定义
│   └── Combat.iface.yaml
├── components/            # 组件定义
│   └── Inventory.comp.yaml
└── entities/
    ├── Avatar.def.yaml
    ├── NPC.def.yaml
    └── Space.def.yaml
```

### Def 示例

```yaml
# Avatar.def.yaml
entity:
  hasBase: true
  hasCell: true
  hasClient: true

properties:
  name:
    type: UNICODE
    flags: BASE | CLIENT
    persistent: true
    sync: ON_CHANGE
    default: ""

  level:
    type: UINT32
    flags: BASE | CELL | CLIENT
    persistent: true
    default: 1

  position:
    type: VECTOR3
    flags: CELL | CLIENT
    persistent: false
    sync: REALTIME
    detail: OWN_ONLY

  hp:
    type: INT32
    flags: BASE | CELL | CLIENT
    persistent: true
    default: 100

clientMethods:
  onHPChanged:
    args:
      old: INT32
      new: INT32

baseMethods:
  addExp:
    args:
      amount: UINT32
    exposed: false

cellMethods:
  useSkill:
    args:
      skillID: UINT32
      targetID: ENTITY_ID
    exposed: true
```

### Def 构建时自动生成

```
Avatar.def.yaml
    │
    ├──▶ C++ AvatarBase / AvatarCell / AvatarClient 类骨架
    │       属性访问器、序列化、delta同步
    │
    ├──▶ Python AvatarBase.py / AvatarCell.py 基类绑定
    │       属性自动变成 py property
    │
    ├──▶ 网络消息注册
    │       MessageID 分配、Handler 注册
    │
    ├──▶ 数据库表结构
    │       MySQL/Redis schema 自动映射
    │
    └──▶ 客户端 SDK
            JS/C#/Java 的 Entity 类定义
```

### 对比当前 KBEngine 的改进

| 项目 | 当前 KBEngine | 重设计 |
|------|--------------|--------|
| 格式 | XML | YAML |
| 同步策略 | VolatileInfo 复杂机制 | `sync: REALTIME / ON_CHANGE` |
| 细节级别 | DETAIL_LEVEL 枚举 | `detail: OWN_ONLY / ALL / IN_RANGE` |
| 客户端调用 | ExposedMessageInfo 机制 | `exposed: true` 直接标记 |
| 代码生成 | 运行时 XML 解析 | 构建时代码生成 |

---

## 三、Entity 系统

### 类层次

```
                    ┌─────────┐
                    │  Entity  │  ← 轻量基类，ID + Def引用 + 属性块
                    └────┬────┘
                         │
              ┌──────────┼──────────┐
              │          │          │
        ┌─────┴────┐ ┌──┴───┐ ┌───┴────┐
        │CellEntity│ │Base  │ │ Proxy  │
        │ 空间实体  │ │Entity│ │ 客户端  │
        └──────────┘ │持久化 │ │ 代理   │
                     └──────┘ └────────┘
```

### Entity 核心结构（C++20）

```cpp
class Entity {
public:
    EntityID id() const;
    ScriptDefModule& def() const;

    // def 驱动的属性访问 — 类型安全，编译期生成
    // 由 def 代码生成器自动产生 set_xxx / get_xxx

    // 远程调用
    EntityCall* baseCall() const;
    EntityCall* cellCall() const;

    // Python 脚本对象
    py::object scriptObject() const;

    // 生命周期
    void destroy(bool deleteFromDB = false);
    void writeToDB();

protected:
    EntityID id_;
    ScriptDefModule* def_;
    PropertyBlock props_;      // 连续内存布局，由 def 定义驱动
    py::object pyObject_;      // Python 脚本实例
};
```

### PropertyBlock — 连续内存，def 驱动布局

```cpp
class PropertyBlock {
    std::vector<uint8_t> data_;       // 连续内存
    std::vector<uint16_t> offsets_;   // 每个属性的偏移（def 编译期确定）
    std::vector<uint64_t> dirtyMask_; // 脏标记位图

    template<typename T>
    T get(size_t propIndex) const {
        return *reinterpret_cast<const T*>(data_.data() + offsets_[propIndex]);
    }

    template<typename T>
    void set(size_t propIndex, const T& val) {
        *reinterpret_cast<T*>(data_.data() + offsets_[propIndex]) = val;
        dirtyMask_[propIndex / 64] |= (1ULL << (propIndex % 64));
    }

    // 只序列化脏属性 → 精确 delta 同步
    void serializeDirty(BinaryWriter& writer) const;
    void serializeAll(BinaryWriter& writer) const;
};
```

对比当前 KBEngine 每个属性都是 `PyObject*` 散落在 map 里，改为 def 定义的属性紧凑排列在连续内存中，cache 友好、序列化高效、delta 同步精确。

---

## 四、View/Witness 系统

保留 BigWorld 核心设计：

```
Space (空间)
├── Cell_1 (格子)
├── Cell_2
└── Cell_3
    ├── CoordinateSystem (九宫格索引)
    │   ├── Entity_A (玩家, 拥有 Witness)
    │   ├── Entity_B (NPC)
    │   └── Entity_C (怪物)
    │
    └── View (Entity_A 的视野)
        ├── ViewRange: 500m (AOI半径)
        ├── 进入视野 → 通知 BaseApp → 转发 Client 创建实体
        ├── 离开视野 → 通知 Client 销毁实体
        └── 属性变化(REALTIME) → 只同步给 Witness 在视野内的实体
```

**改进点**：
- CoordinateSystem 当前用链表节点(CoordinateNode)，改为四叉树/九宫格双索引，视查询模式切换
- View 触发从回调链改为事件总线，解耦 Cell/View/Witness 的循环依赖
- Ghost 实体（跨 Cell 边界的实体副本）用增量快照替代当前的全量同步

---

## 五、网络层

参考 nng/aeron/caf，三层架构：

```
┌─────────────────────────────────┐
│  Transport Layer (传输层)         │  ← 参考 aeron: 零拷贝、批量收发
│  io_uring / epoll / kqueue       │
│  TCP / UDP / KCP / WebSocket     │
├─────────────────────────────────┤
│  Messaging Layer (消息层)         │  ← 参考 nng: pipeline/bus/pub-sub 模式
│  二进制协议 + Schema 驱动编解码     │
│  可靠/不可靠通道分离               │
├─────────────────────────────────┤
│  Actor Layer (Actor 层)          │  ← 参考 caf: 邮箱 + 异步消息传递
│  Entity 作为 Actor               │
│  EntityCall = Actor 地址          │
│  C++20 协程 async/await          │
└─────────────────────────────────┘
```

### 传输层

```cpp
// 替代 EventPoller + EventDispatcher
class IoContext {
    // 底层: io_uring (Linux) / kqueue (macOS) / IOCP (Windows)
    Task<int> accept(Socket& listener);
    Task<size_t> recv(Socket& sock, Buffer& buf);
    Task<size_t> send(Socket& sock, const Buffer& buf);
    Task<void> poll(Duration timeout);
};
```

### 消息层

```cpp
// 消息定义由 def 自动生成
struct Msg_Avatar_onHPChanged {
    static constexpr MsgID id = 0x1023;
    int32 old_value;
    int32 new_value;

    void serialize(BinaryWriter& w) const { w << old_value << new_value; }
    void deserialize(BinaryReader& r) { r >> old_value >> new_value; }
};

// 发送
co_await channel.send(Msg_Avatar_onHPChanged{.old_value = 80, .new_value = 60});

// 接收
channel.on<Msg_Avatar_onHPChanged>([](const auto& msg, Channel& from) {
    // 类型安全处理
});
```

### Actor 层

```cpp
// EntityCall 变成类型安全的 Actor 地址，由 def 自动生成

// 当前: pEntityCall->pyCall("onMoveTo", args...);  // 字符串调用，无类型检查
// 改为:
auto result = co_await avatar.base<AvatarBase>().addExp(100);
//                    ↑ EntityCall    ↑ def生成的方法  ↑ 参数类型检查
```

### 对比当前 KBEngine 网络层

| 项目 | 当前 | 重设计 |
|------|------|--------|
| I/O 多路复用 | epoll/select | io_uring/kqueue/IOCP |
| 发送阻塞 | `sleep(10)` + while循环 | C++20 协程 co_await |
| 消息编解码 | Bundle << operator 手写 | Schema 驱动自动生成 |
| Channel 职责 | 300+行，管一切 | 拆分为 Transport/Channel/Actor |
| 宏 | NETWORK_SEND_TO_ENDPOINT 等 | 消灭全部宏 |
| EntityCall | 函数指针 + MemoryStream | 类型安全 Actor 调用 |

---

## 六、服务端基础设施

替代 ServerApp → EntityApp 的深度继承链，改为组合模式：

```cpp
class Server {
    IoContext io;                          // 网络
    TimerWheel timers;                     // 定时器
    EntityRegistry entities;               // 实体管理
    ScriptEngine python;                   // Python
    ComponentRegistry components;          // 组件发现
    ConfigStore config;                    // 配置

    void run() {
        while (running_) {
            io.poll(16ms);                 // 网络 I/O
            timers.tick();                 // 定时器
            processMessages();             // 消息处理
            gameTick();                    // 游戏帧
        }
    }
};

// 各 App 只是配置不同的 Server 实例
class BaseApp {
    Server server_;
public:
    BaseApp() {
        server_.components.registerRole(Role::Base);
        server_.entities.setEntityClass<BaseEntity, Proxy>();
    }
};
```

---

## 七、Python 集成

```python
# AvatarBase.py — 开发者只写业务逻辑
class AvatarBase(KBEngine.EntityBase):
    def onExpChanged(self, old, new):
        self.level = calc_level(new)       # 属性赋值自动触发同步
        self.client.onLevelUp(self.level)  # client 方法由 def 自动生成

    async def useItem(self, itemID):
        item = await self.getItem(itemID)  # C++20 协程 → Python await
        if item:
            item.use()
```

**改进**：
- pybind11 替代手写 C API
- Python 属性赋值自动触发脏标记和同步（通过 def 生成的 descriptor）
- `await` 支持跨进程 EntityCall 的异步返回

---

## 八、构建流程

```
                    .def.yaml 文件
                         │
                    ┌────┴────┐
                    │  defc   │  ← 代码生成器（类似 protoc）
                    └────┬────┘
                         │
        ┌────────────────┼────────────────┐
        │                │                │
   C++ Entity 类    Python 绑定      网络消息注册
   属性序列化        客户端 SDK       数据库映射
        │                │                │
        └────────────────┼────────────────┘
                         │
                    CMake 构建
                         │
                    编译链接
```

---

## 核心设计原则总结

1. **Def 系统是灵魂** — 声明式定义驱动一切自动生成
2. **BigWorld 架构是骨架** — Base/Cell 分离、View/Witness、Space/Cell 不变
3. **网络层彻底重做** — 协程 + Schema 驱动消息 + Actor 模式
4. **基础设施现代化** — 组合替代继承、消除全局状态、pybind11
5. **技术栈** — C++20/23 + Python 3.12+ + CMake + vcpkg
