# 架构总览

> 这一页只做系统地图，不再承担结构设计提案。它回答的是“背景、专题、对照各放在哪里”，而不是“整站目录未来应该怎么重命名”。

## 系统地图

```mermaid
flowchart TD
    A["study/**\n主学习路径\n按主题递进建立整体理解"] --> B["architecture/\n架构总览与系统地图"]
    A --> C["source-analysis/**\n源码学习专题\n按问题继续深挖"]
    A --> D["api/kbengine/**\nKBEngine 接口契约\n保持 CHM 一致"]
    A --> G["api/bigworld/**\nBigWorld 源码抽取版 API"]
    B --> E["bigworld/**\n背景概念与设计来源"]
    B --> F["comparison/**\nBigWorld / KBEngine 对照"]
    C --> D
    C --> G
    C --> F

    style A fill:#f5f1e8,stroke:#8a6d3b,color:#3b2d1f
    style B fill:#e8f1f5,stroke:#3b6d8a,color:#173042
    style C fill:#edf5e8,stroke:#5a8a3b,color:#203817
    style D fill:#f4f4f4,stroke:#666,color:#222
    style E fill:#f8efe3,stroke:#9a6a2f,color:#4a2f12
    style F fill:#efe8f5,stroke:#6b4a8c,color:#2e1b45
    style G fill:#f8efe3,stroke:#9a6a2f,color:#4a2f12
```

## 目录定位

- `BigWorld` 用来建立概念模型，回答“这套架构为什么会这样设计”。
- `源码专题` 用来追具体问题，回答“当前代码到底怎么实现”。
- `对照分析` 用来回答“哪些思想继承了 BigWorld，哪些地方变成了 KBEngine 自己的实现”。
- `重设计方案` 是附录性质的工程草案，不作为源码分析主线。

这里不是第二套学习主线。按顺序系统学习时，仍然应先进入 [/study/](/study/)；这里负责背景地图、专题入口和对照分析。

## 推荐阅读顺序

1. [BigWorld 学习入口](/architecture/bigworld/)
2. [源码专题总览](/architecture/source-analysis/)
3. [BigWorld / KBEngine 对照](/architecture/comparison/)
4. [KBEngine 2.0 重设计方案](/architecture/redesign.md)

## 当前编写原则

- `architecture/` 负责系统地图、目录边界和阅读路线，不承担问题驱动的源码细节展开。
- `source-analysis` 负责具体问题的源码学习，正文以源码路径、调用链、关键类为中心展开。
- `bigworld` 目录提供概念背景，不替代对 KBEngine 代码的实际阅读。
- `comparison` 目录只写有依据的对应关系，不写想当然的“继承关系”。
- `redesign` 明确属于工程草案，与原始资料转写分开。

## 资料边界

- 当前 `architecture` 目录不属于 CHM/PDF 的逐页转写结果。
- 该目录中的分析结论应以仓库源码为主，BigWorld 仅作为参考框架。
