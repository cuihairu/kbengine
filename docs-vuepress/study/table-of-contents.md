# 全部目录

> 这是详细目录页，不是主入口。主入口是 [源码学习首页](./README.md)；这里用于按 Part 浏览全书结构与阅读路线。

---

## 推荐阅读顺序

1. 先读 Part I，建立问题背景和系统地图。
2. 再读 Part II，搞清组件怎样启动、实体定义怎样变成运行对象。
3. 接着读 Part III，理解基础设施层（并发、网络、分布式）。
4. 然后按 Part IV 的顺序读核心机制：序列化 → RPC → 广播 → 持久化。
5. 再读 Part V，看 AOI、空间、运动、Ghost 这些空间层高级机制。
6. 然后读 Part VI，理解脚本层行为。
7. 接着读 Part VII，从前后端交互角度理解客户端视角。
8. 最后用 Part VIII 把运维调试掌握，再用 Part IX 串联实战。

---

## Part I — 为什么长这样

### [Ch1 导读与阅读方法](01-introduction-and-method.md)

- 这本书的组织方式
- 每章统一结构：核心问题 → BigWorld 背景 → KBEngine 实现 → 关键源码入口 → 关键调用链 → 小结
- 建议的阅读方式（通读 → IDE 跟读 → 专题深挖）
- 源码引用地址

### [Ch2 BigWorld：问题、模型与核心概念](02-bigworld-problems-model-concepts.md)

- 大规模多人在线场景的技术约束
- Login / Base / Cell / DB 分工是如何被需求推出来的
- Entity / Proxy / Base / Cell / Space / Witness / Ghost 各是什么
- BigWorld 模型是"被问题塑造出来的"

### [Ch3 KBEngine 系统全景](03-kbengine-system-panorama.md)

- 项目定位与对 BigWorld 的取舍
- 五类进程的职责边界（接入 / 逻辑 / 空间 / 持久化 / 管理）
- 四条系统边界（接入边界 / 逻辑边界 / 持久化边界 / 管理边界）

---

## Part II — 运行骨架

### [Ch4 启动流程与进程模型](04-startup-and-process-model.md)

- `main → kbeMainT → ServerApp → EntityApp → 组件特化 → 集群注册 → InitProgress`
- 公共骨架 vs 实体骨架 vs 组件特化 vs 集群注册的四层分层
- 主循环：`EventDispatcher::processUntilBreak`

### [Ch5 EntityDef 与实体定义系统](05-entitydef-and-entity-definition.md)

- `.def` 不是配置，而是运行时骨架（脚本描述 + 网络协议描述 + 持久化描述 三合一）
- 属性的四层身份：Python 名 / 所属侧 / 协议字段 ID / 是否持久化
- `EntityApp::createEntity`：定义世界 → 运行世界

### [Ch6 Python 运行时与脚本桥接](06-python-runtime-and-script-bridge.md)

- 为什么游戏服务器选 Python 做业务语言
- C++ 与 Python 的桥接
- 热重载机制
- BigWorld Twisted Deferred vs KBEngine CallbackMgr

---

## Part III — 基础设施层

### [Ch7 并发模型、线程与内存基础设施](07-concurrency-threads-and-memory.md)

- 主线程单线程运行游戏逻辑，工作线程只做 I/O 密集型任务
- 对象池：MemoryStream / Packet 的分配频率
- 内存分配器：kbemalloc / ned_allocator

### [Ch8 网络基础设施：I/O 模型与进程间通信](08-network-infrastructure.md)

- I/O 事件模型：select / poll / epoll / kqueue / IOCP / io_uring
- Reactor 模式与 Channel / Endpoint
- TCP vs UDP：KBEngine 内部 TCP，BigWorld 内部 UDP + 自建可靠性

### [Ch9 分布式基础：ID、发现、注册与一致性](09-distributed-foundation.md)

- 分布式 ID 生成：componentID / entityID
- Machine 注册中心：UDP 广播 vs BigWorld bwmachined 洪流发现
- 一致性保证：real vs ghost 的权威模型

---

## Part IV — 通信与协作

### [Ch10 序列化、Bundle 与网络消息](10-serialization-bundle-and-messages.md)

- 三层职责：MemoryStream / Bundle / MessageHandler
- 为什么用手工流编码而不是通用结构体序列化
- PacketReader：拆包与防御

### [Ch11 RPC、EntityCall 与通信模式](11-rpc-entitycall-and-communication-patterns.md)

- BigWorld：Mailbox + TwoWay + PyDeferred
- KBEngine：EntityCall = 纯单向 + CallbackMgr
- 单向消息流下的工程化解法：CPS、Future/Promise、Actor ask、Reducer + Effects、Saga、CQRS
- `tell / ask / 读模型 / 数据归属重构` 的实战决策表
- Base/Cell/Client 三侧的专用方法类
- 与 gRPC / Protobuf 的设计对比

### [Ch12 属性同步与数据包广播](12-property-sync-and-broadcast.md)

- "tick 内收集，tick 末批量发"的心智模型
- Bundle payload 复用：一份 payload + N 份 header
- alias 机制与 detailLevel 分级

### [Ch13 数据库、DBMgr 与持久化](13-database-dbmgr-and-persistence.md)

- Base → Cell → DBMgr 三段式写库
- KBEngine: MySQL + Redis 双后端
- BigWorld: MySQL + XML + Primary/Secondary + 属性级映射
- EntityLog 在线检出与恢复

---

## Part V — 空间、运动与拓扑

### [Ch14 Space、AOI 与视野系统](14-space-aoi-and-vision-system.md)

- 十字链表空间索引的插入/删除/查询
- RangeTrigger 进入/离开检测
- Hysteresis 防抖与 Witness 状态机

### [Ch15 空间拓扑与动态扩容](15-space-topology-and-dynamic-scaling.md)

- BigWorld BSP 树动态拓扑
- grow/shrink Cell 边界移动
- KBEngine 简化空间管理

### [Ch16 移动、寻路与导航](16-movement-pathfinding-and-navigation.md)

- NavMesh + A* 变体寻路
- 移动控制器架构
- 寻路与 AOI 的关系

### [Ch17 Ghost 系统](17-ghost-system.md)

- real / ghost 区分与 GhostManager
- RealEntityMethod：ghost 转接到 real
- controlledBy 控制权分布

---

## Part VI — 脚本层行为

### [Ch18 钩子、回调、定时器与事件](18-hooks-callbacks-timers-and-events.md)

- 生命周期钩子 / 定时器回调 / 异步结果回调 / 事件注册
- CallbackMgr 与 ScriptTimers

---

## Part VII — 前后端交互

### [Ch19 客户端协议与前后端交互](19-client-protocol-and-interaction.md)

- 客户端实体生命周期
- 属性同步的客户端侧插值与平滑
- 断线重连与 Exposed 方法信任边界

---

## Part VIII — 运维、调试与稳定性

### [Ch20 可观测性：监控、性能分析与调试](20-observability-monitoring-profiling-and-debugging.md)

- Watcher / Profiler / NetworkStats
- BigWorld ForwardingWatcher 分布式查询
- 分布式链路追踪缺失与 OTel 接入可能

### [Ch21 热更新、容错与运维工具](21-hotupdate-fault-tolerance-and-ops.md)

- 脚本热重载的边界
- BigWorld Reviver + Backup + Archive 三级容错
- Bots 压测与安全机制

---

## Part IX — 串联与实战

### [Ch22 玩家完整生命周期](22-player-complete-lifecycle.md)

- 七阶段主流程：Login → DB → BaseAppMgr → BaseApp → Cell → Witness
- 会话主线 / 世界主线 / 数据主线

### [Ch23 BigWorld 与 KBEngine 对照](23-bigworld-and-kbengine-comparison.md)

- 十维度全景对照表
- 进程模型 / RPC 模式 / 空间拓扑 / 持久化 / 容错 / 网络层 / 脚本层

### [Ch24 实战源码走读](24-practical-source-walkthroughs.md)

- 五条函数级走读链路
- 推荐的实际阅读顺序

---

## Appendix

### [A 源码阅读地图与推荐路径](appendix-reading-map-and-next-steps.md)

- 五条专题阅读路径

### [B 关键算法速查](appendix-key-algorithms.md)

- 十字链表 / RangeTrigger / Hysteresis / BSP / NavMesh+A* / Rendezvous Hash / alias / 世代号复用

### [C 外部参考系统速查](appendix-external-reference-systems.md)

- gRPC / nng / Aeron / Twisted Deferred 与游戏服务器的对比

### [D 专业术语速查](appendix-terminology-quick-reference.md)

- 实体、通信、分布式、持久化核心术语的一句话定义
- Mailbox / EntityCall、TwoWay / Deferred、CID 等高频易混概念速查

### [E 引擎适用场景与游戏类型选型指南](appendix-engine-suitability-and-game-types.md)

- 四类游戏类型的引擎匹配分析（房间制 / MOBA / 中型 MMO / 大型 MMO）
- 决策流程：从项目特征到引擎选择
- 常见误区澄清与替代方案推荐
- 与 Ch23 选型建议的交叉参考

### [F 坐标系约定：BigWorld 与 KBEngine](appendix-coordinate-system-conventions.md)

- 轴约定（Y-up、XZ 地平面）、Y 轴开关的设计取舍
- 旋转表示：Direction3D（Euler 角弧度制）与角度压缩（angle2int8）
- 坐标压缩：PackXZ/PackY/PackXYZ 的范围、精度与带宽节省
- 同步协议中的坐标传输（更新标志位组合）
- 客户端接入的坐标变换（Unity/Unreal/Godot）
- 数学库选择（D3DX vs G3D）

### [G 服务器时间管理与世界时钟](appendix-server-time-management-and-world-clock.md)

- 物理时间、逻辑 tick、集群世界时钟三层模型
- KBEngine 的 `timestamp() / EventDispatcher / g_kbetime / ScriptTimers`
- BigWorld 的 `advanceTime / ScriptTimeQueue / TimeKeeper / gameTime` 持久化
- 为什么 MMO 不能直接用 wall clock 驱动世界逻辑
- 如果自己设计一套世界时钟，应该怎样分层实现
