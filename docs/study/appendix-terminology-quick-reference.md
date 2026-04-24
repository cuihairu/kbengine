# 附录 D 专业术语速查

> 这一页是“查词典”用途：每个术语只给一句话定义 + 首读章节。  
> 读正文卡住时，先回这里对齐语义，再回章节看调用链。

---

## D.1 使用方式

- 先看“术语”列，确认你看到的是同一个概念（避免同名误解）。
- 再看“一句话定义”，先建立最小心智模型。
- 最后按“首读章节”跳转做深读。

---

## D.2 实体与运行角色

| 术语 | 英文/代码 | 一句话定义 | 首读章节 |
|------|-----------|-----------|---------|
| 实体 | Entity | 游戏对象的运行时承载单元，不是纯数据结构。 | Ch2 |
| Base 实体 | Base Entity | 驻留 BaseApp，承载长期业务逻辑（账号/背包/任务等）。 | Ch2 / Ch22 |
| Cell 实体 | Cell Entity | 驻留 CellApp，承载实时空间逻辑（移动/AOI/战斗等）。 | Ch2 / Ch14 |
| 代理实体 | Proxy | 带客户端会话语义的特殊 Base 实体。 | Ch2 / Ch19 |
| 空间 | Space | 游戏世界或场景上下文。 | Ch2 / Ch14 |
| Cell | Cell | Space 的运行承载单元（BigWorld 可动态拓扑）。 | Ch2 / Ch15 |
| 见证者 | Witness | 把 AOI 变化转成客户端同步的执行点。 | Ch2 / Ch14 |
| 幽灵实体 | Ghost | 远端 real 实体在本地 Cell 的协作副本。 | Ch2 / Ch17 |

---

## D.3 通信与 RPC

| 术语 | 英文/代码 | 一句话定义 | 首读章节 |
|------|-----------|-----------|---------|
| 邮箱引用 | Mailbox | BigWorld 的远端实体地址抽象，核心语义是“投递消息”。 | Ch2 / Ch11 |
| 远端实体引用 | EntityCall | KBEngine 的远端实体引用，接口外观更像远程方法调用。 | Ch2 / Ch11 |
| 单向调用 | OneWay / Fire-and-forget | 发出后不等待返回结果。 | Ch11 |
| 双向调用 | TwoWay | 带回复通道的异步远程调用（非同步阻塞）。 | Ch11 / Ch6 |
| 延迟结果对象 | Deferred | 异步结果容器 + 回调/错误链调度器。 | Ch6 / Ch11 |
| 回调管理器 | CallbackMgr | callbackID 到回调函数的映射表（KBEngine 常用旁路机制）。 | Ch6 / Ch11 / Ch18 |
| 远端方法桥 | `RemoteEntityMethod::tp_call` | 把 Python 方法调用转换为网络消息发送。 | Ch11 |
| 调用路径类型 | `ENTITYCALL_TYPE` | 定义 Base/Cell/Client 直连或 via 转发路径。 | Ch11 |
| 消息包 | Bundle | 网络消息聚合和发送单元。 | Ch10 |
| 通道 | Channel | 连接状态与收发抽象。 | Ch8 / Ch10 |

---

## D.4 启动、身份与分布式

| 术语 | 英文/代码 | 一句话定义 | 首读章节 |
|------|-----------|-----------|---------|
| 组件标识 | componentID / CID | 组件级路由主键，贯穿注册、路由、运维。 | Ch4 / Ch9 |
| 身份收束 | `checkComponentID()` | KBEngine 启动早期确定/分配 CID 的关键步骤。 | Ch4 |
| 自动分配器 | `IDComponentQuerier` | CID 未显式指定时向 machine 请求可用 CID。 | Ch4 |
| 注册中心（BW） | bwmachined | BigWorld 的机器级注册与管理中心。 | Ch23 / Ch9 |
| 注册中心（KBE） | machine | KBEngine 的简化注册中心。 | Ch4 / Ch9 |
| 启动收束进度 | InitProgress | “进程启动完成”到“可对外服务”的中间收敛过程。 | Ch4 |
| 身份冲突熔断 | `onIdentityillegal()` | 发现组件身份冲突时主动关停的保护机制。 | Ch4 |

---

## D.5 定义、脚本与类型系统

| 术语 | 英文/代码 | 一句话定义 | 首读章节 |
|------|-----------|-----------|---------|
| 实体定义 | EntityDef / `.def` | 同时描述脚本可见结构、网络协议字段、持久化映射。 | Ch5 |
| 方法描述 | MethodDescription | 运行时方法元数据（参数、序列化规则、标识等）。 | Ch5 / Ch11 |
| 数值方法标识 | utype | 协议编解码时定位方法/属性描述的数值 ID。 | Ch5 / Ch10 |
| 脚本定义模块 | ScriptDefModule | EntityDef 在脚本层的描述与绑定载体。 | Ch5 |
| 暴露方法 | Exposed Method | 可从客户端或远端触达的方法边界定义。 | Ch5 / Ch19 |

---

## D.6 持久化与容错

| 术语 | 英文/代码 | 一句话定义 | 首读章节 |
|------|-----------|-----------|---------|
| 写库入口 | `writeToDB` | 将在线实体状态异步收束到持久化层。 | Ch13 |
| 周期归档器 | Archiver | 周期触发实体归档写库。 | Ch13 / Ch21 |
| 周期备份器 | Backuper | 周期触发备份数据写入（不等价于跨进程热备恢复）。 | Ch13 / Ch21 |
| 在线检出记录 | EntityLog | 记录实体在线归属，辅助恢复和冲突清理。 | Ch13 |
| 进程守护 | Reviver | BigWorld 的自动拉起与故障恢复组件。 | Ch21 / Ch23 |

---

## D.7 常见易混概念（一眼区分）

| 易混对 | 区分要点 |
|--------|---------|
| Mailbox vs EntityCall | 都是远端实体引用；Mailbox 语义更像“收件箱地址”，EntityCall 外观更像“远程方法入口”。 |
| OneWay vs TwoWay | OneWay 不等结果；TwoWay 有异步回复通道。 |
| Deferred vs CallbackMgr | Deferred 强在可组合与统一错误链；CallbackMgr 强在简单直接。 |
| Base vs Cell | Base 偏长期业务状态；Cell 偏实时空间状态。 |
| CID vs EntityID | CID 标识组件实例；EntityID 标识实体实例。 |
| Ghost vs Witness | Ghost 解决跨 Cell 协作副本；Witness 解决 AOI 到客户端同步。 |
| bwmachined vs machine | 都是注册中心；前者更完整，后者更简化。 |
| Actor 心智 vs RPC 外观 | 本质是异步消息系统；RPC 只是接口表现，不是本地函数语义。 |

---

## D.8 快速排障读法（卡住时按这个顺序）

1. 先判定执行域：Base / Cell / Client 在哪一侧执行。
2. 再判定调用模式：OneWay 还是 TwoWay，是否有回调链。
3. 再看路由键：CID、EntityID、`ENTITYCALL_TYPE` 是否一致。
4. 最后看收束点：是否进入 DB、是否进入 InitProgress、是否触发熔断。
