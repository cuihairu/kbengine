---
layout: home
hero:
  image:
    src: /logo.png
    alt: KBEngine logo
  text: KBEngine 中文文档
  tagline: 单一学习主线，辅以源码专题、背景对照、API 回查与文档说明
  actions:
    - theme: brand
      text: 开始阅读
      link: /zh/study/
    - theme: alt
      text: 专题深挖
      link: /zh/architecture/source-analysis/
    - theme: alt
      text: API 参考
      link: /zh/api/
features:
  - title: 学习主线
    details: “study/** 是唯一按顺序阅读的主入口。第一次阅读只沿 24 章推进，先建立完整系统地图。”
    link: /zh/study/
  - title: 专题深挖
    details: “architecture/source-analysis/** 负责把具体问题继续拆到文件、类、函数和调用链，不再单独充当第二套正文。”
    link: /zh/architecture/source-analysis/
  - title: 背景与对照
    details: “architecture/** 负责 BigWorld 背景、系统地图和两套引擎的差异对照，用来补'为什么这样设计'。”
    link: /zh/architecture/
  - title: API 参考
    details: “api/** 只做接口契约与快速回查。源码实现和架构解释分别回到专题页和学习主线。”
    link: /zh/api/
  - title: 文档说明
    details: “resources/** 记录来源边界、校验状态和结构设计，回答'这页是否来自原始资料、应该如何维护'。”
    link: /zh/resources/
footer: 文档来源为仓库内已有资料，持续更新中
---

## 进程架构

KBEngine MMOG 引擎由一组协作进程组成，通过动态负载均衡横向扩展。玩家主线 `Client → LoginApp → BaseApp → CellApp → DBMgr`：LoginApp 负责接入与认证，BaseApp 承载玩家长期逻辑与客户端会话，CellApp 负责实时空间模拟，DBMgr 集中持久化到 MySQL / Redis。外围进程（Machine、BaseAppMgr、CellAppMgr、Logger、Interfaces、Bots）提供服务注册、调度、日志汇聚、外部对接与压测。组件与数据流依据 `kbe/src/server` 源码。

![KBEngine 多进程架构：玩家主线 Client → LoginApp → BaseApp → CellApp → DBMgr，以及注册中心、调度器、日志、外部接口与压测客户端](/images/architecture-zh.svg)

[打开完整 SVG](/images/architecture-zh.svg) · [架构概览](/zh/architecture/) · [系统地图](/zh/architecture/)

