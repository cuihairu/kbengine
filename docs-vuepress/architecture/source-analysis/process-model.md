# 进程模型与组件协作

> 本页聚焦 KBEngine 的多进程模型本身，而不是某个单独组件的 API。

## 建议覆盖的主题

- LoginApp / BaseApp / CellApp / DBMgr / Interfaces / Logger / Bots / Machine / Manager 的职责
- 组件发现、注册、心跳与管理
- 跨组件消息的基本流向
- 管理进程如何参与启动、监控与负载协调

## 目标问题

- 玩家登录流程经过哪些组件？
- 普通非空间逻辑为什么落在 BaseApp？
- 场景逻辑为什么落在 CellApp？
- DBMgr 在持久化链路中扮演什么角色？
