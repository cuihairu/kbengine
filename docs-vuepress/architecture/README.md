# 架构总览

> 本栏目用于梳理 KBEngine 的核心进程模型、设计来源与后续重构思路。

## 阅读顺序

1. [KBEngine 2.0 重设计方案](/architecture/redesign.md)
2. [源码分析](/architecture/source-analysis.md)
3. [BigWorld 对照](/architecture/bigworld.md)

## 当前内容说明

- `重设计方案` 是人工整理的现代化重构草案，不属于 CHM/PDF 原文转写内容。
- `源码分析` 用于后续补充实际代码路径、启动流程、实体链路和 Space/Cell 机制。
- `BigWorld 对照` 用于整理术语映射与设计差异，帮助理解 KBEngine 的来源和边界。

## 阅读建议

- 如果你想快速了解 KBEngine 的整体形态，先看 `重设计方案`。
- 如果你想核对现有实现，再看 `源码分析`。
- 如果你关心设计来源与演化脉络，再看 `BigWorld 对照`。

## 资料边界

- 当前 `architecture` 目录不属于 CHM/PDF 的逐页转写结果。
- 该目录里的说明性内容需要单独标注来源，并与 API/指南原始资料分开看待。
