---
home: true
heroImage: /logo.png
heroText: KBEngine 中文文档
tagline: 以源码学习为主线，覆盖 BigWorld 背景、KBEngine 实现、架构专题与 API 参考
actions:
  - text: 开始阅读
    link: /study/
    type: primary
  - text: 详细目录
    link: /study/table-of-contents.html
    type: secondary
  - text: 查看架构专题
    link: /architecture/
    type: secondary
features:
  - title: 先走主线
    details: 24 章书式路径是本站主入口。从 BigWorld 问题背景、KBEngine 进程模型，到序列化、RPC、AOI、持久化、玩家生命周期与实战走读，按依赖顺序展开。
    link: /study/
  - title: 架构深度解析
    details: 架构专题用于按主题回看系统，不替代主线阅读。适合已经走过书式路径后，按组件或机制做二次归纳。
    link: /architecture/
  - title: BigWorld / KBEngine 对照
    details: 十维度全景对照：进程模型、实体模型、RPC 模式、空间拓扑、AOI 策略、持久化、负载均衡、容错、网络层、脚本层。用于收束两套系统的继承与分歧。
    link: /study/23-bigworld-and-kbengine-comparison.html
  - title: 完整 API 参考
    details: API 是工具书入口，不是学习主线。适合在源码阅读过程中回查脚本接口、数据类型和组件能力。
    link: /api/
  - title: 关键算法速查
    details: 十字链表空间索引、AOI RangeTrigger、Hysteresis 防抖、BSP 树动态拓扑、NavMesh+A* 寻路、Rendezvous Hash、alias 消息压缩、Bundle payload 复用等核心算法独立参考。
    link: /study/appendix-key-algorithms.html
  - title: 外部系统对比
    details: 与 gRPC/Protobuf、nng、Aeron、Twisted Deferred 的设计对比，理解游戏服务器为什么需要自建通信层而不是复用通用框架。
    link: /study/appendix-external-reference-systems.html
footer: 文档来源为仓库内已有资料，持续更新中
---
