# BigWorld 核心概念

> 本页定义学习 BigWorld 与 KBEngine 时反复出现的核心对象与术语。

## 建议覆盖的主题

- Entity：逻辑实体、Base 实体、Cell 实体、Client 表示
- Space：场景容器、空间归属、跨空间迁移
- Cell：空间切分、负载拆分、跨 Cell 同步
- Witness / View：视野、可见集、客户端同步
- Ghost：跨边界副本与跨 Cell 观察
- Manager 组件：负责协调、负载与进程管理

## 编写方式

- 每个术语单独定义“职责 / 所在进程 / 和其他对象的关系”。
- 如果 KBEngine 中有同名或近似实现，后续在 `comparison/terminology.md` 建立映射。
