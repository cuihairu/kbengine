# BigWorld 学习入口

> 这一组文档用于建立理解 KBEngine 所需的 BigWorld 架构背景。

> 它不是 KBEngine 主线正文，而是背景区。真正按顺序阅读时，先走 [/study/](/study/)；需要补设计来源时，再回到这里。

## 这一组回答什么问题

- BigWorld 为什么采用多进程分布式架构？
- Base / Cell / DB 的分工是怎么形成的？
- Entity、Space、Witness 这些概念之间是什么关系？
- 哪些思想直接影响了 KBEngine？

## 页面范围

- [核心概念](/architecture/bigworld/concepts.md)
- [进程模型](/architecture/bigworld/process-model.md)
- [实体与空间](/architecture/bigworld/entity-space.md)

## 编写原则

- 以概念框架和架构思想为主。
- 不把 BigWorld 文档写成 KBEngine 实现细节页。
- 后续与 KBEngine 的对应关系，统一放到 `comparison/`。
