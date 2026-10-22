# BigWorld 进程模型

> 本页用于理解 BigWorld 风格的服务器为什么被拆成多种长期运行的组件进程。

## 建议覆盖的主题

- Login / Base / Cell / DB 的基本职责
- Manager 进程的控制与负载协调作用
- 组件之间的典型消息流转
- 为什么空间逻辑与非空间逻辑要拆分
- 为什么玩家代理通常落在 Base 侧

## 对 KBEngine 学习的价值

- 这页不是为了复述 BigWorld 历史，而是为了给 KBEngine 源码中的多进程结构提供理解框架。
