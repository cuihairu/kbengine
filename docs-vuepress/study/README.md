---
home: true
heroText: KBEngine / BigWorld 源码学习
tagline: 24 章结构化路径，从问题背景到函数级走读，完整掌握 MMO 服务器架构
actions:
  - text: 开始阅读
    link: /study/01-introduction-and-method.html
    type: primary
  - text: 详细目录
    link: /study/table-of-contents.html
    type: secondary
features:
  - title: "\u2776 问题驱动"
    details: 从"为什么传统单进程不行"出发，理解 Login/Base/Cell/DB 分工是如何被 MMO 需求一步步推出来的。
    link: /study/02-bigworld-problems-model-concepts.html
  - title: "\u2777 基础设施先行"
    details: 并发模型、I/O 多路复用、分布式 ID 与注册中心——理解系统跑在什么样的基础设施之上。
    link: /study/07-concurrency-threads-and-memory.html
  - title: "\u2778 核心通信机制"
    details: 序列化 → RPC → 属性同步 → 持久化，逐层构建实体间的通信与数据流。
    link: /study/10-serialization-bundle-and-messages.html
  - title: "\u2779 空间与 AOI"
    details: 十字链表、RangeTrigger、Hysteresis 防抖、BSP 树动态拓扑——面试高频的 AOI 算法详解。
    link: /study/14-space-aoi-and-vision-system.html
  - title: "\u277A 实战走读"
    details: 五条函数级链路（登录、EntityCall、属性同步、写库、视野建立），可直接拿 IDE 跟进。
    link: /study/24-practical-source-walkthroughs.html
  - title: "\u277B 双引擎对照"
    details: BigWorld vs KBEngine 十维度对比——"完整方案 vs 最小可行方案"的设计取舍。
    link: /study/23-bigworld-and-kbengine-comparison.html
footer: 24 章 · 9 个 Part · 3 个附录 · 覆盖从基础设施到运维实战的完整技术栈
---

> 这是本站唯一的源码学习主线。建议先按章节顺序读完整本书，再回到 `architecture` 或 `api` 做专题回查。

> 入口关系：这里是主首页；[详细目录](./table-of-contents.md) 用来按 Part 浏览；3 个附录放在正文之后，作为查表和延伸阅读。

## 系统架构总览

```mermaid
graph TB
    Client[Client 客户端]
    Bots[Bots<br/>压测机器人]
    Login[LoginApp<br/>接入层]
    BaseAppMgr[BaseAppMgr<br/>Base 协调]
    BaseApp[BaseApp<br/>Base 逻辑]
    CellAppMgr[CellAppMgr<br/>Cell 协调]
    CellApp[CellApp<br/>空间逻辑]
    DBMgr[DBMgr<br/>持久化]
    Machine[Machine<br/>注册中心]
    Logger[Logger<br/>日志]
    Interfaces[Interfaces<br/>外部接口网关]

    Client -->|TCP/KCP| Login
    Bots -->|模拟客户端协议| Login
    Bots -->|压测 Base/Cell 路径| BaseApp
    Client -->|TCP/KCP| BaseApp
    Login --> BaseAppMgr
    Login --> DBMgr
    BaseAppMgr --> BaseApp
    BaseApp --> CellAppMgr
    BaseApp --> CellApp
    CellAppMgr --> CellApp
    BaseApp --> DBMgr
    BaseApp --> Machine
    CellApp --> Machine
    DBMgr --> Machine
    Logger --> Machine
    Interfaces --> BaseApp
    Interfaces --> DBMgr
    BaseApp --> Logger
    CellApp --> Logger
    Login --> Logger
    DBMgr --> Logger

    style Client fill:#4FC3F7,color:#000
    style Bots fill:#4FC3F7,color:#000
    style Login fill:#81C784,color:#000
    style BaseApp fill:#FFB74D,color:#000
    style CellApp fill:#E57373,color:#000
    style DBMgr fill:#BA68C8,color:#000
    style Machine fill:#90A4AE,color:#fff
    style Interfaces fill:#AED581,color:#000
    style Logger fill:#90A4AE,color:#fff
```

上图现在明确区分了两层：

- `LoginApp / BaseApp / CellApp / DBMgr` 是玩家业务主线。
- `BaseAppMgr / CellAppMgr / Machine / Logger / Interfaces / Bots` 是调度、注册、日志、外部接入与压测辅助进程。

其中 `Client → BaseApp` 这条线表示两种情况：

- 首次登录时，客户端先连 `LoginApp`，拿到目标 `BaseApp` 地址后再连过去。
- 重连或已有会话恢复时，客户端会直接向目标 `BaseApp` 发 `loginBaseapp / reloginBaseapp`。

## 实体模型

```mermaid
graph LR
    subgraph BaseApp 进程
        Proxy[Proxy<br/>玩家 Base 实体]
    end
    subgraph CellApp 进程
        Real[Cell Real<br/>空间实体]
        Ghost[Cell Ghost<br/>边界副本]
        Witness[Witness<br/>观察者]
    end
    Client2[Client]

    Proxy <-->|EntityCall| Real
    Real <-->|ghost 同步| Ghost
    Real -->|AOI 事件| Witness
    Witness -->|属性同步| Client2
    Client2 -->|Exposed 方法| Proxy

    style Proxy fill:#FFB74D,color:#000
    style Real fill:#E57373,color:#000
    style Ghost fill:#EF9A9A,color:#000
    style Witness fill:#CE93D8,color:#000
    style Client2 fill:#4FC3F7,color:#000
```

## 五条实战走读链路

```mermaid
graph LR
    L[登录链路] --> E[EntityCall 链路]
    E --> S[属性同步链路]
    S --> D[写库链路]
    D --> V[视野建立链路]

    style L fill:#81C784,color:#000
    style E fill:#FFB74D,color:#000
    style S fill:#4FC3F7,color:#000
    style D fill:#BA68C8,color:#fff
    style V fill:#E57373,color:#000
```

## 推荐阅读路径

```mermaid
graph TD
    P1["Part I<br/>为什么长这样<br/>Ch1-3"] --> P2["Part II<br/>运行骨架<br/>Ch4-6"]
    P2 --> P3["Part III<br/>基础设施层<br/>Ch7-9"]
    P3 --> P4["Part IV<br/>通信与协作<br/>Ch10-13"]
    P4 --> P5["Part V<br/>空间与拓扑<br/>Ch14-17"]
    P5 --> P6["Part VI<br/>脚本层<br/>Ch18"]
    P6 --> P7["Part VII<br/>前后端交互<br/>Ch19"]
    P7 --> P8["Part VIII<br/>运维与稳定性<br/>Ch20-21"]
    P8 --> P9["Part IX<br/>串联实战<br/>Ch22-24"]

    style P1 fill:#1a1a2e,color:#e0e0e0
    style P2 fill:#16213e,color:#e0e0e0
    style P3 fill:#0f3460,color:#e0e0e0
    style P4 fill:#533483,color:#e0e0e0
    style P5 fill:#e94560,color:#fff
    style P6 fill:#0f3460,color:#e0e0e0
    style P7 fill:#16213e,color:#e0e0e0
    style P8 fill:#1a1a2e,color:#e0e0e0
    style P9 fill:#e94560,color:#fff
```
