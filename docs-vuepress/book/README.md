# KBEngine / BigWorld 源码学习

> 这是一条独立于 API 手册的结构化学习路径，目标不是罗列接口，而是把 BigWorld 的问题背景、KBEngine 的系统取舍，以及关键源码链路组织成一套能持续扩写的"书"。

## 这本书关注什么

- BigWorld 试图解决的核心技术约束，以及这些约束怎样塑造出 `Login / Base / Cell / DB` 模型。
- KBEngine 如何继承这套模型，并在开源工程环境中做出自己的实现取舍。
- 从基础设施（I/O 模型、并发、内存管理）到核心机制（RPC、AOI、广播、持久化）到运维调试的完整技术栈。
- 每个关键设计决策背后的"为什么"，以及与其他方案（gRPC、nng、Aeron 等）的对比。
- 一个玩家从登录到下线的完整运行链，以及可以直接拿 IDE 跟进去的函数级走读路径。

## 这本书不做什么

- 不按函数逐个解释 API。
- 不把背景资料拆成低密度铺垫。
- 不把源码分析和接口手册混在同一条叙事线上。

---

## 推荐阅读顺序

1. 先读 Part I，建立问题背景和系统地图。
2. 再读 Part II，搞清组件怎样启动、实体定义怎样变成运行对象。
3. 接着读 Part III，理解基础设施层（并发、网络、分布式）。
4. 然后按 Part IV 的顺序读核心机制：序列化 → RPC → 广播 → AOI → 持久化。
5. 再读 Part V，看空间、运动、Ghost 这些空间层高级机制。
6. 然后读 Part VI，理解脚本层行为。
7. 接着读 Part VII，从前后端交互角度理解客户端视角。
8. 最后用 Part VIII 把运维调试掌握，再用 Part IX 串联实战。

---

## Part I — 为什么长这样

建立心智模型。读完这部分，你应该知道这套架构为什么长成这个样子，而不是别的样子。

### [Ch1 导读与阅读方法](01-introduction-and-method.md)

- 这本书的组织方式
- 每章统一结构：核心问题 → BigWorld 背景 → KBEngine 实现 → 关键源码入口 → 关键调用链 → 小结
- 建议的阅读方式（通读 → IDE 跟读 → 专题深挖）
- 源码引用地址
  - BigWorld: https://github.com/cuihairu/BigWorld-Engine-14.4.1
  - KBEngine: 本书工作目录下的 `kbe/`

### [Ch2 BigWorld：问题、模型与核心概念](02-bigworld-problems-model-concepts.md)

> 合并原第 2-4 章。把 BigWorld 背景压成一章，一次读完建立完整心智模型。

- 大规模多人在线场景的技术约束
- 为什么传统单进程模型不行
- Login / Base / Cell / DB 分工是如何被需求推出来的
- Entity / Proxy / Base / Cell / Space / Witness / Ghost 各是什么
- 每个概念的职责、所在进程、与其他概念的关系
- BigWorld 模型是"被问题塑造出来的"

### [Ch3 KBEngine 系统全景](03-kbengine-system-panorama.md)

> 合并原第 5+6 章。定位和架构放一起。

- 项目定位与对 BigWorld 的取舍
- 五类进程的职责边界（接入 / 逻辑 / 空间 / 持久化 / 管理）
- 四条系统边界（接入边界 / 逻辑边界 / 持久化边界 / 管理边界）
- 系统记忆图：`Client → LoginApp → BaseApp → CellApp`，侧面 DBMgr / *Mgr / Machine

---

## Part II — 运行骨架

理解组件怎么跑起来，实体怎么从定义变成运行对象。

### [Ch4 启动流程与进程模型](04-startup-and-process-model.md)

- `main → kbeMainT → ServerApp → EntityApp → 组件特化 → 集群注册 → InitProgress`
- 公共骨架 vs 实体骨架 vs 组件特化 vs 集群注册的四层分层
- 组件启动完成 ≠ 准备好对外服务（InitProgressHandler）
- 主循环：`EventDispatcher::processUntilBreak`
- BaseApp / CellApp / DBMgr 各自的启动差异

### [Ch5 EntityDef 与实体定义系统](05-entitydef-and-entity-definition.md)

> 从原位置前移。RPC、属性同步、持久化都依赖实体定义，必须先讲。

- `.def` 不是配置，而是运行时骨架（脚本描述 + 网络协议描述 + 持久化描述 三合一）
- `ScriptDefModule`：属性描述表 / 方法描述表 / 组件描述表 / alias / volatileInfo
- 属性的四层身份：Python 名 / 所属侧 / 协议字段 ID / 是否持久化-是否客户端可见
- alias 机制：定义系统直接决定协议压缩方式
- `EntityApp::createEntity`：定义世界 → 运行世界
- `initializeEntity`：定义驱动的数据装配过程
- 持久化与定义系统的衔接
- 组件系统：实体定义是对象树，不是平面表

### [Ch6 Python 运行时与脚本桥接](06-python-runtime-and-script-bridge.md)

> 从原位置前移，紧跟 EntityDef。两个项目都用 Python 做业务脚本，但集成深度不同。

- **为什么游戏服务器选 Python 做业务语言**
  - C++ 引擎做性能关键路径，Python 做业务逻辑快速迭代
  - 热更新需求：线上修复 bug 不能重启整个集群
  - 脚本驱动的开发方式：策划/脚本程序员不需要编译 C++
- **Python 在两套项目里的核心位置**
  - 实体行为全部由 Python 脚本定义（onTick / onEnterSpace / onWitness 等）
  - EntityDef 的 .def 文件描述结构，Python 文件定义行为
  - 脚本层是引擎和游戏业务之间唯一的扩展接口
- **应用继承链的差异**
  - KBEngine: `ServerApp → EntityApp`（两层）
  - BigWorld: `ServerApp → EntityApp → ScriptApp`（三层）
    - ScriptApp 多了一层 Python 运行时初始化
    - `lib/server/script_app.cpp`：在 EntityApp 基础上安装 Python 解释器、脚本模块、事件系统
    - `lib/server/python_server.cpp` / `python_server.hpp`：PythonServer 类封装 CPython 初始化/销毁
  - BigWorld 的 ScriptApp 多做了什么：
    - Python 路径初始化（PyImport_AppendInittab）
    - 脚本模块注册（entity_type / base / cell 各自的 Python 类型）
    - 脚本定时器系统（app_script_timers）
    - 脚本频繁任务（script_frequent_tasks）
- **C++ 与 Python 的桥接**
  - PyTypeObject / tp_call / PyObject 持有
  - KBEngine: `kbe/src/lib/pyscript/` — Python 桥接模块
  - BigWorld: `lib/pyscript/`（50+ 文件）+ `lib/script/`（40+ 文件）— 更完整的绑定层
  - 两者对比：
    - BigWorld 多了 pyobject_plus / script_events / script_data_sink/source 等高级抽象
    - BigWorld 的类型绑定更系统化（data_types/ 下有 30+ 种 DataType 的 Python 映射）
- **脚本对象创建与 Entity 对象的统一构造**
  - ScriptDefModule::createObject()：分配 Python 对象
  - 在同一构造流程里把 Python 对象和 C++ Entity 对象绑定
  - 实体不是"先有脚本对象再外挂 C++ 句柄"，而是统一构造
- **BigWorld 的 Twisted Deferred：脚本层的异步编程能力**
  - `lib/entitydef/py_deferred.hpp/cpp`：PyDeferred 类，基于 Twisted Deferred 模式
  - 为什么 BigWorld 引入 Twisted：
    - 异步操作在脚本层也需要优雅处理（DB 查询 / 远程调用 / 定时器链）
    - Deferred 提供了 callback/errback 链式处理
  - KBEngine 为什么没有：
    - KBE 选择更简单的 CallbackMgr——callbackID 映射，结果回来直接执行
    - 牺牲了组合能力（不能链式 then），但更简单直接
  - 这也是 BigWorld RPC TwoWay 的基础设施（Ch11 会再讲）
- **热重载机制**
  - 基本原理：PyImport_ReloadModule 重新加载 .py 模块
  - KBEngine: reloadScript 接口
  - BigWorld: script_bigworld.cpp 中的重载逻辑
  - **边界：哪些能热更，哪些不能**
    - 能热更：实体行为逻辑（Python 方法体）
    - 不能热更：EntityDef 定义（.def 文件的属性/方法签名变更需要重启）
    - 不能热更：已存在的实体对象的 C++ 侧状态
    - 灰区：全局变量的重置语义
  - **热更新的完整流程**
    - 服务器接收 reload 命令
    - Python 解释器重新 import 模块
    - 已有实体对象引用的旧类 → 需要处理类指针更新
    - 新创建的实体使用新类定义
  - **与钩子章节的关系**
    - 本章关注"运行时怎么建立 + 怎么重载"
    - 具体 onInit / onReadyForLogin 等钩子的语义和调用时机放 Ch18

- **设计决策：为什么选 Python 而不是 Lua？**
  - **前提：Lua 确实更快**（LuaJIT 比 CPython 快 30-50x，内存也更小）
  - **但脚本层性能不是瓶颈**
    - MMO 服务器热路径全在 C++（网络 I/O、AOI、序列化、寻路）
    - Python 脚本只处理实体行为回调，tick 频率 10Hz（100ms 预算）
    - 从 2ms 优化到 0.1ms 在 100ms 预算里无意义
    - 真正的性能约束：网络带宽、AOI 计算量、DB 写入延迟——都不是脚本层的事
  - **Python 的表现力是生产力乘数**
    - MMO 脚本是几十万行业务代码，不是几百行嵌入式脚本
    - Python 内置 class / 继承 / 多重继承 / 装饰器 / 生成器 / with / abc / property
    - Lua 没有内置 class（需手写元表）、没有装饰器、没有 with、1-based 数组
    - Lua 的哲学"提供最少机制让你自己构建"——在大型工程里"自己构建"的成本很高
  - **标准库：Python 自带电池**
    - json / xml / re / logging / unittest / hashlib / configparser 全内置
    - Lua 标准库极小，以上全需第三方库或 C++ 绑定
    - BigWorld 脚本要做的不仅是实体行为，还有 billing 对接、配置读取、日志查询、运维工具
  - **Twisted Deferred：BigWorld 的异步基础设施绑定 Python**
    - BigWorld 的 PyDeferred 直接基于 Twisted Deferred 模式
    - TwoWay RPC、DB 查询、定时器链全部依赖 Deferred
    - Lua 没有 Twisted，这个模式要么自己实现，要么放弃
    - KBEngine 虽然没用 Twisted，但也受益于 Python 的闭包/生成器做异步编排
  - **热重载：Python 模块系统天然支持**
    - CPython 内置 `PyImport_ReloadModule`
    - Lua 的 `require` 缓存已加载模块，热重载需自己清 `package.loaded` 并处理状态保持
    - 在复杂实体系统中，Python 的热重载更可控
  - **团队门槛**
    - MMO 团队：少数 C++ 引擎程序员 + 多数脚本逻辑程序员 + 策划
    - Python 可读性高，非专业程序员也能写
    - Lua 元表魔法、全局变量污染、1-based 数组对非程序员是障碍
  - **历史时机**
    - BigWorld 架构设计于 2002-2004 年
    - Python 2.x 已成熟，C API 稳定
    - LuaJIT 还不存在（2005 首发示，2008 才稳定）
    - 即使 LuaJIT 存在，以上因素仍然主导决策
  - **一句话总结**：Lua 是给 C++ 程序员嵌进去写小脚本的，Python 是给团队写大工程的。BigWorld/KBEngine 的脚本层代码量是工程级别，不是脚本级别。
  - **客户端为什么常用 Lua 而服务端不用**
    - 客户端：UI 脚本、输入处理，代码量小，每帧调用频繁，需要极低延迟
    - 服务端：业务逻辑，代码量大，需要标准库/异步框架/热重载，tick 频率低
    - WoW 等游戏：客户端 UI 用 Lua，服务端逻辑用 C++ 或 Python——同一个人不写两端

---

## Part III — 基础设施层

理解系统跑在什么样的基础设施之上：并发、网络、分布式。

### [Ch7 并发模型、线程与内存基础设施](07-concurrency-threads-and-memory.md)

- **为什么游戏服务器不是多线程并行处理请求**
  - 主线程单线程运行游戏逻辑
  - 工作线程只做 I/O 密集型任务（DB / 加载）
- **主线程模型：EventDispatcher 驱动一切**
  - 单线程事件循环：网络回调 + 定时器 + game tick
  - 为什么 Entity tick 不需要加锁
  - 主线程的瓶颈在哪
- **工作线程**
  - KBEngine: ThreadPool / TPThread / TPTask
  - BigWorld: WorkerThread / WorkerJob / BgTaskManager
  - 任务投递 → 执行 → presentMainThread 回主线程
- **线程安全策略：空锁模式**
  - ThreadMutexNull (KBE) / DummyMutex (BW)
  - 对象池的线程安全参数选择
- **对象池**
  - MemoryStream / Packet 的分配频率为什么是性能关键
  - KBEngine: ObjectPool\<T\> + SmartPoolObject\<T\>
  - BigWorld: PackedObjectPool + Handle\<T\> + 世代号复用
- **内存分配器**
  - 为什么不用系统默认 malloc
  - KBEngine: kbemalloc（nedmalloc / jemalloc）
  - BigWorld: ned_allocator / fixed_sized_allocator / pool_allocator
- **同步原语工具箱**
  - 两套项目的锁类型对比
  - ReadWriteLock / SafeFIFO / Atomic

### [Ch8 网络基础设施：I/O 模型与进程间通信](08-network-infrastructure.md)

- **I/O 多路复用**
  - select / poll / epoll / kqueue 各自特点
  - 为什么游戏服务器选 epoll
  - KBEngine: poller_epoll.h vs poller_select.h
  - BigWorld: event_poller.cpp（同一文件实现 select/poll/epoll 三种）
- **Reactor 模式**
  - 四个参与者：Handle / SyncDemultiplexer / Event Handler / Reactor
  - 代码映射：EventPoller / InputNotificationHandler / EventDispatcher
  - 为什么不是 Proactor
- **Channel 与 Endpoint**
  - Endpoint：对 socket fd 的封装
  - Channel：Endpoint + PacketReader + PacketSender + MessageHandlers
  - Channel 生命周期
  - DelayedChannels
- **TCP vs UDP：不同路径的不同选择**
  - KBEngine: 内部 TCP，外部 TCP/UDP/KCP
  - BigWorld: 内部 UDP 为主 + 自建可靠性，TCP 补充
  - 与 Aeron 的设计对比（Aeron 也是 UDP + 自建可靠性）
- **InterfaceTable / 消息路由表**
  - 组件启动时注册接口集
  - 消息 ID → handler 的静态绑定
  - 内部 Channel 建立过程
- **两套项目的网络层架构对比**

### [Ch9 分布式基础：ID、发现、注册与一致性](09-distributed-foundation.md)

> BigWorld/KBEngine 都是分布式系统，这一章讲它们如何成为一个"集群"。

- **分布式 ID 生成**
  - componentID：进程身份标识
  - entityID：实体全局唯一标识
  - ID 分配策略：区间分配 vs 集中分配
  - componentID 是否具有位置识别能力（能否判断是否同一机器）
  - 与 Snowflake / UUID 方案的对比
- **Machine：游戏服务器的"注册中心"**
  - 为什么需要 Machine：10+ 个进程组件，没有注册中心就无法互相发现
  - Machine 不是业务组件，而是基础设施——所有组件启动第一步就是找 Machine
  - **KBEngine Machine 的设计**
    - UDP 广播机制：组件启动后通过广播宣告自己的存在
    - onBroadcastInterface()：组件注册自身地址（内网/外网）
    - onFindInterfaceAddr()：组件查询其他组件地址
    - onQueryAllInterfaceInfos()：获取全局组件视图
    - cidMap_ / pidMD5Map_：组件 ID → 地址的映射表
    - MachineInterface：注册/查询/心跳的完整接口定义
    - Components 类：每个进程维护的已知组件表
    - 完整注册流程：新组件启动 → 广播 → Machine 收录 → 其他组件查询到
  - **BigWorld bwmachined 的设计**
    - 比 KBEngine Machine 复杂得多：完整的进程生命周期管理
    - BWMachined 核心类：handleCreateMessage / handleMessage / broadcastToListeners
    - Listeners 系统：birth_listeners_ / death_listeners_——组件上线/下线的通知
    - Cluster 类：跨机器集群管理
      - chooseBuddy()：伙伴节点选择
      - FloodTriggerHandler / FloodReplyHandler：洪流发现算法（非简单广播）
      - BirthReplyHandler：新节点加入的确认机制
    - machine_guard：与 bwmachined 配合的机器守护进程
    - save()/load()：注册表持久化——Machine 重启后可恢复集群视图
  - **两者对比**
    - KBEngine：轻量级中心化注册，UDP 广播 + 查询
    - BigWorld：分布式集群管理，洪流发现 + 监听器 + 持久化
    - BigWorld 有 machine_guard 做机器级守护，KBEngine 没有
    - BigWorld 的 birth/death 监听器让组件能感知其他组件的上线/下线事件
  - **与现代服务注册中心的对比**
    - Machine vs Consul / etcd / ZooKeeper：游戏服务器为什么不用这些
    - Machine vs nng 的 survey 模式：类似的查询-发现语义
    - Machine 的局限：单点问题、无健康检查、无自动摘除
    - BigWorld 的 Reviver 弥补了部分局限
  - **onRegisterNewApp：注册完成后的第二层收束**
    - 组件在 Machine 注册后，还要向目标组件发送 onRegisterNewApp
    - 目标组件（BaseAppMgr / CellAppMgr / DBMgr）维护自己的组件表
    - 从"Machine 知道你存在"到"业务组件愿意和你协作"
- **进程间通信模型**
  - 与 Aeron 的通信模型对比（Aeron: 单播/多播 + 背压感知）
  - 与 nng 的通信模式对比（pair/req-rep/pub-sub/push-pull/survey/bus）
  - BigWorld/KBEngine 选择了什么模式，为什么
- **一致性保证**
  - 在线状态一致性：KBEEntityLogTable / LogOnRecord
  - 实体状态一致性：real vs ghost 的权威模型
  - 最终一致性 vs 强一致性：为什么 MMO 选最终一致性
- **稳定性保证**
  - 进程死亡检测与恢复
  - BigWorld Reviver vs KBEngine 的处理方式
  - 心跳与超时机制

---

## Part IV — 通信与协作

理解实体间如何通信、数据如何同步。

### [Ch10 序列化、Bundle 与网络消息](10-serialization-bundle-and-messages.md)

- **三层职责：MemoryStream / Bundle / MessageHandler**
- **为什么用手工流编码而不是通用结构体序列化**
  - 高吞吐、小包体、可控布局
- **Bundle 真正在做什么**
  - 一个 bundle 可以拼多条消息
  - newMessage / finiMessage
- **MessageHandler：消息如何落地**
  - 组件级消息表
  - msgID → handler 查找
- **PacketReader：拆包与防御**
  - 未知消息 ID condemn 通道
  - 半包 fragment 合并
- **MemoryStream 为什么贯穿三条主线**
  - RPC 参数 / 属性同步 / Base-Cell 交接 / 持久化 / DB 查询全部复用
- **FixedMessages：高频字段的固定消息优化**
- **优势与包袱**

### [Ch11 RPC、EntityCall 与通信模式](11-rpc-entitycall-and-communication-patterns.md)

> 从"只讲实现"扩展为"先讲模式选择，再讲实现"。

- **RPC 的四种通信模式**
  - Fire-and-forget：发出不管
  - Request-Response：gRPC/HTTP 默认
  - Deferred/Callback：发请求注册回调
  - Streaming：双向持续流
  - 为什么 MMO 不能选 req-resp 作为默认
- **BigWorld 的选择：Mailbox + 单向为主 + TwoWay 补充**
  - Mailbox 是远端实体的"邮箱地址"
  - BWTwoWay + PyDeferred（Twisted Deferred 模式）
  - ReturnValuesHandler 处理回复
- **KBEngine 的选择：EntityCall = 纯单向 + CallbackMgr**
  - EntityCall 与 BigWorld Mailbox 的直接对应
  - 为什么砍掉 TwoWay
  - CallbackMgr vs TwoWay+Deferred 的本质区别
- **EntityCall 详解**
  - 不是一次调用请求，而是长期可持有的远端实体引用
  - 方法定义固化进实体定义（Utype / MethodDescription）
  - tp_call 完整链路
  - newCall_ 路由逻辑
- **Base/Cell/Client 三侧的专用方法类**
  - Base 侧 EntityRemoteMethod
  - Cell 侧 ClientEntityMethod（受 Witness/AOI 约束）
  - Ghost 上的 RealEntityMethod（转接到 real entity）
- **Exposed 方法的信任边界**
- **与 gRPC / Protobuf / HTTP / nng 的设计对比**
  - 服务发现 vs 实体引用持有
  - .proto 代码生成 vs .def 引擎内元数据
  - 无状态 vs 有状态实体
  - 为什么通用 RPC 框架不适合 MMO 实体系统

### [Ch12 属性同步与数据包广播](12-property-sync-and-broadcast.md)

> 面试高频章节。回答"一个实体属性变更，怎么高效同步给所有看到的客户端"。

- **心智模型：不是"改了就发"，而是"tick 内收集，tick 末批量发"**
  - 每个 tick 内属性可能变化多次，只发最终值
  - Witness 的 update() 在 tick 末统一执行
  - batch 广播模型
- **一条属性更新的完整链路**
  - 脚本 setattr → 标记脏 → Cell 确定 authoritative 侧 → Witness 收集 → Bundle 构造 → 发送
- **Bundle 的构造与复用**
  - payload 复用：同一实体的属性变更数据只序列化一次
  - header 不同：每个客户端的 entityID alias 不同
  - "一份 payload + N 份 header"
- **alias 机制：为什么属性同步包这么小**
  - usePropertyDescrAlias 的条件
  - alias 分配算法（onLoaded）
  - 短 ID vs 长 Utype
- **detailLevel：不是所有属性都实时同步**
  - 远处实体只同步位置/朝向
  - 近处实体同步全部
  - VolatileInfo 与 detailLevel 分级设计
- **BigWorld 的 aoi_update_schemes**
  - 可插拔的 AOI 更新策略抽象层
  - KBEngine 为什么没做这层抽象
- **广播的效率边界**
  - 1000 人同屏时的带宽瓶颈
  - N 客户端 × M 实体 × K 属性
  - 为什么 MMO 有"最大同屏人数"约束

### [Ch13 数据库、DBMgr 与持久化](13-database-dbmgr-and-persistence.md)

> 这一章不只是"怎么写库"，而是回答：在线实体的运行态和数据库的持久态之间，引擎到底做了哪些抽象和保障？

- **DBMgr 不是 SQL 代理，是在线/离线边界管理者**
- **为什么不让 Base 或 Cell 直接写库**
- **一次 writeToDB 的完整链路（Base → Cell → DBMgr 三段式）**
- **Base / Cell / DBMgr 各自的职责**
- **DBTaskWriteEntity：线程池异步执行 + 主线程回调**

- **数据操作的封装层：EntityDef → 存储结构的映射**
  - **KBEngine: EntityTables + EntityTable**
    - EntityDef 定义中哪些属性标记了 `persistent = true`
    - EntityTables 按实体模块名找对应表对象
    - EntityTable 负责具体的 CRUD SQL 构造
    - `addPersistentsDataToStream()`：按 persistentPropertyDescr_ 统一编码
    - `createDictDataFromPersistentStream()`：从持久化流恢复 Python 字典
    - 改 .def 文件 → 影响表结构、写库流、恢复流、客户端同步——这就是为什么定义系统是三合一的
  - **BigWorld: EntityMapping + PropertyMapping（更完善）**
    - `lib/db_storage_mysql/mappings/` 下 15+ 种映射类
    - EntityMapping：整个实体到数据库行的映射
    - PropertyMapping：单个属性到列的映射（支持 blob/sequence/string/class/composite/user_type 等）
    - EntityTypeMapping / EntityTypeMappings：按实体类型管理映射集
    - ColumnMapping / CommaSepColumnNamesBuilder：列级的精细控制
    - StreamToQueryHelper：流式数据 → SQL 查询的转换器
    - PythonMapping：Python 对象到数据库列的序列化
    - 比 KBEngine 多了"属性级映射策略"——每个属性可以有不同的存储策略

- **存储后端：MySQL vs Redis vs XML**
  - **KBEngine: MySQL + Redis 双后端**
    - `lib/db_mysql/`：MySQL 完整实现
      - db_interface_mysql.h/cpp：MySQL 连接、执行、事务管理
      - entity_table_mysql.h/cpp：MySQL 表结构管理
      - entity_sqlstatement_mapping.h/cpp：SQL 语句映射
    - `lib/db_redis/`：Redis 完整实现
      - db_interface_redis.h/cpp：Redis 连接与命令执行
      - entity_table_redis.h/cpp：Redis 的实体表结构（用 Hash/Set/Sorted Set 模拟表）
      - redis_helper.h：Redis 命令辅助
      - redis_watcher.h/cpp：键空间通知
      - db_transaction.h/cpp：Redis 事务（MULTI/EXEC）
    - **MySQL vs Redis 的使用场景**
      - MySQL：实体持久化主存储（关系型，事务，复杂查询）
      - Redis：缓存层 + 会话数据 + globalData/baseAppData/cellAppData（内存级，高速读写）
      - 两者通过 DBInterface 统一接口，脚本层透明
    - **Redis 的"表"是怎么做的**
      - 没有 SQL 表，用 Redis Hash 存实体属性
      - 用 Sorted Set 做索引
      - Key 设计：`dbname:entityType:entityID:field`
  - **BigWorld: MySQL + XML 双后端（无 Redis）**
    - `lib/db_storage_mysql/`：MySQL 完整实现
      - mysql_database.h/cpp：连接池、查询执行
      - mysql_billing_system.h/cpp：计费系统
      - tasks/ 下 15+ 种异步任务（get/put/del/write_space_data/set_game_time 等）
      - buffered_entity_tasks.h/cpp：批量实体操作
      - thread_data.h/cpp：线程级连接管理
    - `lib/db_storage_xml/`：XML 存储
      - xml_database.h/cpp：开发/测试用的轻量存储
      - xml_billing_system.h/cpp：XML 版计费
      - 不适合生产环境，但方便本地开发调试
    - **PrimaryDatabase / SecondaryDatabase**
      - BigWorld 支持主库 + 从库分离
      - SecondaryDatabase：读操作可以走从库
      - transfer_db / consolidate_dbs：主从迁移工具
  - **为什么 BigWorld 不用 Redis**
    - BigWorld 用 SharedData（globalData/baseAppData/cellAppData）做内存级数据共享
    - 这些数据存在 BaseAppMgr / CellAppMgr 进程内存中，不走数据库
    - 架构选择不同：BigWorld 把"高速数据"放在管理进程内存，KBEngine 放在 Redis
  - **设计含义对比**
    | 维度 | KBEngine | BigWorld |
    |------|----------|----------|
    | 主存储 | MySQL | MySQL |
    | 缓存 | Redis | 无（内存共享替代） |
    | 开发存储 | 无 | XML |
    | 从库 | 无原生支持 | Primary/Secondary |
    | 属性映射 | EntityTable（表级） | PropertyMapping（属性级） |

- **KBEEntityLogTable / EntityLogTable：在线实体检出与恢复**
  - 不是普通业务数据，而是"实体在线日志"
  - 记录：dbid / componentID / entityID / flags / deadline
  - 用于判断：这是纯离线实体还是已有在线上
  - 没有这层，createEntityFromDBID / 账号恢复 / 重检出都会变得脆弱

- **Entity Auto-Loading：服务器启动时自动恢复**
  - DBMgr 启动时扫描需要自动加载的实体类型
  - BigWorld: entity_auto_loader.cpp
  - KBEngine: dbmgr 中的 auto-load 逻辑

- **Backup / Archive 机制**
  - BigWorld: archiver.cpp / backup_sender.cpp / auto_backup_and_archive.cpp
  - KBEngine: Backuper / Archiver
  - 定时归档 vs 即时写库的区别
  - Backup：Cell 状态备份到 Base（容灾）
  - Archive：Base 状态写入数据库（持久化）

- **globalData / baseAppData / cellAppData：内存级数据共享**
  - 跨组件的全局数据，不需要走数据库
  - KBEngine：SharedDataManager（可能底层用 Redis）
  - BigWorld：SharedData，存在 BaseAppMgr / CellAppMgr 内存
  - 使用场景：全服公告、在线人数统计、跨 Base 的共享状态
  - 与 Redis 的关系：KBEngine 可以用 Redis 实现，BigWorld 不需要 Redis 因为数据在 Mgr 进程内存

- **BillingSystem：计费系统**
  - BigWorld 有完整的计费抽象层
    - billing_system.h/cpp：计费接口
    - mysql_billing_system.h/cpp / xml_billing_system.h/cpp
    - custom_billing_system.h/cpp / bwauth_billing_system.h/cpp：可扩展的计费后端
    - py_billing_system.h/cpp：Python 层可调用
  - KBEngine：计费逻辑在脚本层自行实现，引擎不内置

- **BigWorld DBApp + DBAppMgr vs KBEngine DBMgr**
  - BigWorld：DBApp（实际操作）+ DBAppMgr（集群协调）
  - KBEngine：合为单个 DBMgr
  - 拆分的好处：多 DBApp 可以水平扩展数据库操作

---

## Part V — 空间、运动与拓扑

理解世界如何被组织、实体如何移动、空间如何动态分配。

### [Ch14 Space、AOI 与视野系统](14-space-aoi-and-vision-system.md)

> 面试高频章节。详细讲解 AOI 算法。

- **AOI 解决什么问题**
  - 不是"谁在附近"，而是"谁的状态需要同步给谁"
- **常见空间索引方案对比**
  - 九宫格：简单但不均匀
  - 四叉树：动态划分但更新成本高
  - 十字链表（有序链表）：两个维度分别排序的链表
  - 为什么 BigWorld 和 KBEngine 都选了十字链表
- **十字链表的具体实现**
  - CoordinateSystem / CellRangeList：两套有序链表
  - EntityCoordinateNode / EntityRangeListNode
  - 插入 / 移动 / 删除的时间复杂度分析
  - 缓存友好性
- **RangeTrigger 算法**
  - 触发器挂载位置
  - 进入/离开检测机制
  - 时间复杂度
- **ViewTrigger：视野特化**
  - 两个半径（view + hysteresis）
  - Hysteresis 防抖的必要性
- **Witness：从 AOI 事件到客户端同步**
  - onEnterView / onLeaveView 状态机（ENTER_PENDING / LEAVE_PENDING）
  - 为什么不是立即发消息
  - update() 把 pending 变成客户端消息
- **实体移动触发 AOI 更新的完整链路**
- **与四叉树/九宫格的性能对比**

### [Ch15 空间拓扑与动态扩容](15-space-topology-and-dynamic-scaling.md)

> BigWorld 核心功能。KBEngine 在这方面简化很多。

- **BigWorld 的空间拓扑：BSP 树**
  - BSPNode / InternalNode / LeafNode
  - BSP_NODE_HORIZONTAL / VERTICAL / LEAF
  - SpaceBranch：BSP 内部节点，对应分割平面
  - EntityBoundLevels：实体在各层级的边界数据
- **动态负载均衡**
  - InternalNode::balance / doBalance / BalanceDirection
  - grow / shrink：Cell 边界动态移动
  - CellAppGroup：CellApp 分组管理
  - metaLoadBalanceScheme / metaLoadBalanceTolerance
  - Offload：实体跨 Cell 迁移（offload_checker）
- **KBEngine 的简化方案**
  - 没有 BSP 树
  - 空间管理更简单（SpaceMemory）
  - 为什么简化，取舍是什么
- **Cell 死亡处理**
  - BigWorld: dead_cell_apps / ack_cell_app_death_helper
  - 实体迁移与恢复

### [Ch16 移动、寻路与导航](16-movement-pathfinding-and-navigation.md)

- **地图数据格式**
  - BigWorld: Chunk 格式（.chunk / .cdata），PhysicalChunkSpace
  - KBEngine: NavMesh 文件格式（Recast 导出），SpaceData
  - 支持哪些软件导出的数据
  - Recast / Detour 集成
- **NavMesh 基本原理**
  - 为什么不用 Grid / Waypoint
  - 导航网格生成流程（Recast 流水线）
  - A* 在导航多边形上的变体
- **移动控制器架构**
  - BigWorld: move_controller / navigation_controller / turn_controller / face_entity_controller / accelerate_* / proximity_controller / vision_controller / timer_controller
  - KBEngine: MoveController / NavigateHandler
  - Controller 模式：目标点 → 路径 → 逐帧移动 → 到达回调
- **寻路与 AOI 的关系**
  - 移动触发 AOI 事件
  - 寻路结果影响同步频率
- **射线检测**
  - 基于 Chunk / NavMesh 的射线投射
  - 碰撞检测与空间查询
  - 在服务器端和客户端的不同用途

### [Ch17 Ghost 系统](17-ghost-system.md)

- **real / ghost 区分**
  - Cell 世界不是单副本
  - isReal() 检查遍布 Cell 代码
- **GhostManager**
  - ghost 消息缓冲机制
  - buffered_ghost_message / buffered_ghost_message_queue / buffered_ghost_message_factory
- **RealEntityMethod**
  - ghost 不直接执行逻辑，转接到 real entity
  - 同一份脚本接口在 real / ghost 上对应不同执行语义
- **controlledBy：谁在控制这个 cell 实体**
  - 与 Witness / client 关系恢复的关系
  - 重连 / 挤号 / 控制权转移
- **跨 Cell 边界实体的控制权分布**

---

## Part VI — 脚本层行为

理解引擎怎样通过钩子、回调、定时器驱动脚本行为。

### [Ch18 钩子、回调、定时器与事件](18-hooks-callbacks-timers-and-events.md)

- **四大类严格区分**
  - 生命周期钩子：引擎主动调用（onInit / onBaseAppReady / onReadyForLogin / onGetWitness / onWriteToDB）
  - 定时器回调：脚本注册，引擎未来触发（addTimer）
  - 异步结果回调：CallbackMgr 托管（writeToDB 回调 / createEntityFromDBID 回调）
  - 事件注册/恢复：运行时注册的事件响应关系（registerEvent）
- **生命周期钩子如何从 C++ 进入 Python**
  - onBaseAppReady / onReadyForLogin 的触发点
  - onReadyForLogin 是轮询直到返回完成
- **CallbackMgr 模型**
  - 发起时保存 callbackID → 结果回来时 take(callbackID) → 执行回调
- **ScriptTimers**
  - timerID vs 底层 TimerHandle
  - 定时器的恢复语义（addTimersToStream / createEventsFromStream）
- **事件注册的恢复语义**
  - 事件名 + 回调名可以被序列化/反序列化
  - 事件是实体运行态的一部分，不是外挂观察者

---

## Part VII — 前后端交互

理解客户端视角的协议和交互。

### [Ch19 客户端协议与前后端交互](19-client-protocol-and-interaction.md)

- **客户端 SDK 的协议层**
  - 客户端怎样收发消息
  - ClientInterface 消息定义
  - onRemoteMethodCall / onUpdateProperties
- **客户端视角的实体生命周期**
  - 实体创建 / 销毁协议
  - 进入空间 / 离开空间
  - onEntityEnterSpace / onEntityLeaveSpace
- **属性同步的客户端侧**
  - 收到 onUpdateProperties 后客户端做什么
  - 位置/朝向的插值与平滑
  - detailLevel 在客户端的表现
- **断线重连的客户端侧**
  - onReloginBaseappSuccessfully
  - 客户端如何重建世界状态
- **消息的顺序性与可靠性**
  - TCP 保证顺序 vs UDP 不保证
  - KCP 的可靠性机制
- **客户端 EntityCall**
  - 客户端调服务端方法（Exposed 方法）
  - 信任边界与安全考虑

---

## Part VIII — 运维、调试与稳定性

理解如何运维、调试和保障稳定性。

### [Ch20 可观测性：监控、性能分析与调试](20-observability-monitoring-profiling-and-debugging.md)

> 这一章不只是"工具列表"，而是回答：在运行中的 MMO 集群里，你怎么知道系统现在健康不健康？出了问题怎么定位？

- **可观测性三支柱在游戏服务器中的映射**

  | 支柱 | 业界标准 | BigWorld/KBEngine 对应 | 完备度 |
  |------|---------|----------------------|--------|
  | **Metrics（指标）** | Prometheus + Grafana | Watcher + ProfileVal / SendingStats | 有基础指标，但无时序存储和告警 |
  | **Tracing（追踪）** | OpenTelemetry Tracing | 无分布式链路追踪 | **缺失** |
  | **Logging（日志）** | ELK / Loki | Logger / message_logger | 有集中日志，但无结构化查询 |

- **Watcher 系统：游戏服务器的"metrics 端点"**
  - 设计理念：任意 C++ 变量/函数返回值可以注册到观察树
  - **KBEngine Watcher 实现**
    - WatcherObject / WatcherValue\<T\> / WatcherFunction\<R\> / WatcherMethod\<R, OBJ\>
    - 路径式层次组织（"app/.../tickTime"）
    - 通过 guiconsole 或 telnet 远程查询
    - WatchObjectPool：对象池级别的运行时统计
  - **BigWorld Watcher 实现（更完整）**
    - watcher_nub：Watcher 的网络接入层（TCP/UDP）
    - watcher_connection：远程连接管理
    - watcher_forwarding：**分布式转发**——从 Mgr 组件汇聚所有业务组件的 watcher 值
    - watcher_forwarding_baseapp / watcher_forwarding_cellapp：按组件类型收集
    - watcher_forwarding_collector：汇总器
    - 这意味着 BigWorld 可以从 BaseAppMgr / CellAppMgr 一站式看到所有下属组件的状态
  - **与 Prometheus 的本质差异**
    - Watcher 是拉取式（查询时取值），Prometheus 也是拉取式——模式相同
    - 但 Watcher 没有时序存储、没有聚合、没有告警——只有当前值快照
    - Watcher 是"开发调试工具"，Prometheus 是"生产监控系统"
    - 如果要接入 Prometheus，只需写一个 exporter 定期拉 Watcher 值推给 pushgateway

- **Profiler：函数级性能剖析**
  - **KBEngine Profile / ProfileVal**
    - 函数级耗时统计
    - PROFILE_FUNC / PROFILE_SCOPE 宏
    - baseapp/profile.h / cellapp/profile.h：组件级性能摘要
  - **BigWorld Profiler（更细致）**
    - lib/cstdmf/profiler.hpp：核心 profiler
    - EntityProfiler：**单个实体**的性能追踪
    - CellProfiler：Cell 级负载追踪
    - EntityTypeProfiler：按实体类型聚合的耗时统计
    - AutoScopedHelper：RAII 自动作用域计时
    - 指数平滑负载计算
  - **与 pprof / perf 的关系**
    - 引擎内置 Profiler 是逻辑级（哪些函数慢），pprof/perf 是系统级（CPU 火焰图）
    - 两者互补：引擎 Profiler 知道"哪个实体类型 tick 慢"，pprof 知道"哪个系统调用慢"
  - **与 OTel Span 的差异**
    - 引擎 Profiler 没有 Span 概念（无 traceID / spanID / parentID）
    - 无法跨进程追踪一次调用的完整链路
    - 这就是为什么需要 Tracing（见下）

- **NetworkStats：网络层统计**
  - KBEngine: NetworkStats / NetworkStats::Stats——消息级收发追踪
  - BigWorld: SendingStats / NetworkStats——bits/bytes/packets/messages per second，Bundle 重传统计
  - 这些是**系统级指标**，不是业务指标，但定位网络瓶颈时关键

- **Telnet 调试控制台**
  - 远程连接执行 Python 命令
  - 在线排查脚本问题、查看实体状态、调用脚本方法
  - KBEngine: telnet_server.h / telnet_handler.h
  - BigWorld: 类似

- **GUI Console**
  - KBEngine: guiconsole（WatcherWindow / ProfileWindow / DebugWindow）
  - BigWorld: cellappmgr_viewer_server / bwp 工具
  - 与 Web Console 的区别：直接连进程，不经过 HTTP

- **Message Logger：集中日志**
  - KBEngine: Logger 组件
  - BigWorld: message_logger——完整系统
    - binary_file_handler / text_file_handler：双格式存储
    - categories / log_string_interpolator：结构化日志
    - py_bwlog / py_query / py_query_result：Python 可查询
    - user_components：按组件维度隔离
    - 支持按组件/级别/时间/主机查询

- **ServerInfo：硬件与系统信息**
  - BigWorld: server_info.hpp——CPU、内存、机器名
  - 运维时需要知道"这个 CellApp 跑在什么机器上、负载多少"

- **分布式链路追踪：两套项目共同的缺失**
  - **问题**：一次 EntityCall 从 Base → Cell → 回 Base → 回调脚本，跨越多个进程，但没有 traceID
  - **现状**：只能靠日志时间戳 + 经验拼链路
  - **OpenTelemetry Tracing 能做什么**
    - 给每次 EntityCall 分配 traceID / spanID
    - 跨进程传递 trace context（通过 Bundle 头部注入 W3C TraceContext）
    - 在 Jaeger / Zipkin 中可视化完整调用链
  - **接入难点**
    - 高频调用（每秒百万级 EntityCall）→ sampling 策略必须设计好
    - 同步 tick 内的调用是串行的，span 嵌套关系比微服务更复杂
    - 需要在 MemoryStream / Bundle 层注入 trace context，改动较深
  - **与 nng survey 模式的类比**
    - nng 的 survey 可以"一次询问，所有节点回复"——类似从 BaseAppMgr 广播查询所有 BaseApp 状态
    - 但 nng 不提供 tracing，只提供通信模式

- **OTel Metrics 接入的可能性**
  - Watcher 已经暴露了指标端点，写一个 OTel Collector receiver 即可
  - 关键指标清单：
    - 实体数量（按类型 / 按组件）
    - tick 耗时（P50 / P99）
    - AOI 更新耗时
    - 网络带宽（收/发 bytes/s）
    - DB 写入延迟
    - Channel 数量
    - 对象池使用率
  - 告警规则示例：
    - tick 耗时 > 100ms → tick 堆积
    - Channel 数量突降 → 大量断线
    - DB 写入延迟 > 500ms → 数据库瓶颈

- **常见调试场景**
  - 定位属性同步延迟：Watcher 查 tickProfile → Profile 查 Witness::update 耗时 → NetworkStats 查带宽
  - 定位 RPC 链路瓶颈：Profiler 查 EntityCall 处理耗时 → NetworkStats 查消息队列深度
  - 定位 AOI 性能问题：Profile 查 RangeTrigger 耗时 → Watcher 查实体数量 → EntityTypeProfiler 查分布
  - 定位数据库写库延迟：Profile 查 writeToDB 耗时 → Watcher 查 ThreadPool 队列长度

### [Ch21 热更新、容错与运维工具](21-hotupdate-fault-tolerance-and-ops.md)

- **脚本热重载**
  - reloadScript 的边界
  - 哪些能热更，哪些必须重启
  - 热更时的实体状态保持
- **容错与恢复**
  - BigWorld Reviver：自动检测和拉起死亡进程
  - KBEngine 的容错处理
  - 进程死亡后的实体恢复
- **Bots 压测**
  - BigWorld: bot_entity / py_bots / beeline_controller / zigzag_patrol_graph
  - KBEngine: bots 工具
  - 如何模拟真实玩家行为
- **数据迁移**
  - consolidate_dbs / transfer_db / sync_db
  - 数据库表结构变更
- **安全**
  - Exposed 方法的信任边界
  - 登录挑战（BigWorld: login_challenge / cuckoo_cycle）
  - 加密通道（encryption_filter）
  - rndUUID 的身份识别作用
- **日常运维**
  - 配置管理
  - 日志轮转
  - 性能基线建立

---

## Part IX — 串联与实战

把前面的机制收束成完整主线。

### [Ch22 玩家完整生命周期](22-player-complete-lifecycle.md)

- **七阶段主流程**
  - Loginapp 接入 → DBMgr 状态查询 → Baseappmgr 分配 → Baseapp 会话建立 → Base 实体恢复 → Cell 实体创建 → Witness 建立
- **三条主线**
  - 会话主线：Loginapp → Baseapp
  - 世界主线：Base → Cell → Witness → Client
  - 数据主线：Cell → Base → DBMgr
- **属性/方法/持久化三类数据的流向**
- **下线、重连与恢复**
  - 普通下线 vs 重连 vs 控制权转移（giveClientTo）
  - 四个对象（客户端连接 / Base Proxy / Cell 实体 / Witness）的关系
- **恢复主线：Proxy / clientEntityCall / Witness**

### [Ch23 BigWorld 与 KBEngine 对照](23-bigworld-and-kbengine-comparison.md)

- **进程模型对照**（BigWorld DBApp+DBAppMgr vs KBEngine DBMgr；BigWorld Reviver vs KBE 无）
- **实体模型对照**
- **RPC 模型对照**（BigWorld TwoWay+Deferred vs KBE 纯单向+CallbackMgr）
- **空间模型对照**（BSP 树动态拓扑 vs 简化空间管理）
- **AOI 模型对照**（aoi_update_schemes 可插拔 vs 固定策略）
- **持久化模型对照**
- **负载均衡对照**（BigWorld CellAppGroup+MetaBalance vs KBE 简化）
- **容错对照**（Reviver / Ghost 恢复 / Backup 接管）
- **网络层对照**（UDP 为主 vs TCP 为主；内部通道差异）
- **脚本层对照**（Twisted Deferred vs 简化回调）

### [Ch24 实战源码走读](24-practical-source-walkthroughs.md)

- **统一走读方法**：起点 → 跨组件消息 → 目标端落地 → 回调收束
- **实战一：一次登录请求怎样进入 Baseapp**
  - 函数级走读路径
  - 关键状态：componentID / PendingLoginMgr / entityID
- **实战二：一次 EntityCall 怎样发出去并落地**
  - 方法 ID 来源 / 参数描述解释 / Bundle 构造层 / 定义查找
- **实战三：一个属性更新如何同步给客户端**
  - authoritative 侧判定 → Witness update → Bundle 发送
- **实战四：一次写库请求如何经过 DBMgr**
  - Base → Cell 收束 → Base 整理 → DBMgr 落库 → 回调
- **实战五：一个玩家进入 Space 后如何建立视野**
  - Base 创建 Cell → Witness 建立 → AOI 事件 → 客户端消息
- **推荐的实际阅读顺序**
  - 优先跟三条链：登录 → EntityCall → 写库

---

## Appendix

### [A 源码阅读地图与推荐路径](appendix-reading-map-and-next-steps.md)

- 独立跟踪一次完整链路的能力清单
- 五条专题阅读路径
  - 路径一：从 main.cpp 到主循环
  - 路径二：从脚本调用到 EntityCall
  - 路径三：从属性更新到客户端同步
  - 路径四：从 writeToDB 到 DBMgr
  - 路径五：从 Space 到 Witness
- 后续扩写原则

### [B 关键算法速查](appendix-key-algorithms.md)

- B.1 空间索引：十字链表的插入/删除/查询
- B.2 AOI 触发器：RangeTrigger 的进入/离开判定
- B.3 视野防抖：Hysteresis 区域算法
- B.4 空间拓扑：BSP 树的分割与平衡（BigWorld）
- B.5 动态均衡：InternalNode::doBalance 的 grow/shrink
- B.6 寻路：NavMesh + A* 变体
- B.7 负载均衡：Rendezvous Hash（BigWorld 实体分配）
- B.8 实体 ID 分配：区间分配与 componentID 编码
- B.9 消息压缩：alias 分配算法
- B.10 对象池：世代号复用（BigWorld Handle\<T\>）
- B.11 属性同步：detailLevel 分级过滤
- B.12 广播优化：Bundle payload 复用

### [C 外部参考系统速查](appendix-external-reference-systems.md)

- C.1 gRPC / Protobuf：req-resp 模式的工业标准
- C.2 nng：多模式通信（pair/req-rep/pub-sub/push-pull/survey/bus）
- C.3 Aeron：高性能 UDP 消息传递（单播/多播 + 背压感知）
- C.4 Twisted Deferred：BigWorld 的异步回调模型来源
