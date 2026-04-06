# KBEngine 源码分析总览

> 本目录是整个文档站最核心的部分，目标是围绕源码理解 KBEngine 的实际实现。
>
> 当前状态说明：这一组页面已经从“提纲索引”推进到“按主题组织的源码分析”。不同页面的细节深度仍不完全一致，但都应以实际文件、类、函数和调用链为依据，而不是概念性概述。

## 分析主线

1. [启动入口与引导流程](/architecture/source-analysis/entry-and-bootstrap.md)
2. [进程模型与组件协作](/architecture/source-analysis/process-model.md)
3. [实体系统](/architecture/source-analysis/entity-system.md)
4. [空间、AOI 与视野同步](/architecture/source-analysis/space-aoi.md)
5. [网络与消息系统](/architecture/source-analysis/networking.md)
6. [持久化与数据库](/architecture/source-analysis/persistence.md)
7. [脚本运行时与热重载](/architecture/source-analysis/scripting.md)

## 每一页建议采用的结构

- 入口源码位置
- 核心类与关键文件
- 调用链或消息链
- 运行时职责
- 与 BigWorld 的对应点
- 未确认点 / 待继续跟踪点

## 分析原则

- 优先写“代码怎么走”，其次才写“为什么这么设计”。
- 结论要尽量落到文件、类、函数、调用路径。
- BigWorld 只作为参考系，不替代对 KBEngine 源码本身的分析。
