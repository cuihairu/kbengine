# 11. 钩子、回调、定时器与事件

> 本文件是旧稿，已由 [`18-hooks-callbacks-timers-and-events.md`](/study/18-hooks-callbacks-timers-and-events.html) 取代。阅读主线请直接进入新稿。

> 这是理解 KBEngine 脚本层行为的关键章节之一。要看懂这一章，必须先把“引擎主动调用你”和“你注册一个异步结果回调”彻底区分开。

## 14.1 本章核心问题

- KBEngine 提供了哪些“钩子”？
- 这些钩子到底是生命周期回调、事件回调、定时器回调，还是异步操作回调？
- 它们分别在什么时机被触发？
- 这些回调在源码里是如何从 C++ 进入 Python 的？

## 14.2 先建立四大类，不要混着看

这一章最容易出问题的地方，就是把所有 Python 可调用入口都叫“钩子”。更准确的分类应该是：

### 生命周期钩子

由引擎在固定阶段主动调用。

例如：

- `onInit`
- `onBaseAppReady`
- `onReadyForLogin`
- `onReadyForShutDown`
- `onGetWitness`
- `onWriteToDB`
- `onPreArchive`

### 定时器回调

由脚本主动注册，之后由引擎在未来时间触发。

例如：

- `Entity.addTimer`
- `EntityComponent.addTimer`

### 异步结果回调

由脚本发起某个异步操作时显式传入，随后由 `callbackMgr` 一类机制保存并在结果返回时取回。

例如：

- `writeToDB` 回调
- `createEntityFromDBID` 回调
- 删除实体回调
- interfaces / DB 结果回调

### 事件注册 / 恢复

这类不是固定生命周期，也不是一次性异步回调，而是运行时注册的事件响应关系。

从 `cellapp/entity.cpp` 可见事件还能序列化 / 反序列化恢复。

只有先把这四类拆开，后面读源码才不会乱。

## 14.3 生命周期钩子是如何从 C++ 进入 Python 的

这一类通常都有非常明确的 C++ 触发点。

### 组件启动阶段

源码入口：

- `kbe/src/server/baseapp/initprogress_handler.cpp`
- `kbe/src/server/cellapp/initprogress_handler.cpp`
- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/dbmgr/dbmgr.cpp`

从这些代码可以直接看到：

- BaseApp 启动过程中会调用入口脚本的 `onBaseAppReady`
- BaseApp / CellApp 在准备接入前会轮询 `onReadyForLogin`
- BaseApp / DBMgr / Logger 在关闭准备阶段会调用 `onReadyForShutDown`
- CellApp 初始化阶段会调用入口脚本的 `onInit`

这里很重要的一点是：

- 有些钩子是“一次性通知”
- 有些钩子是“引擎轮询直到你返回准备完成”

例如 `onReadyForLogin` 就明显属于后者。

### 实体生命周期与运行阶段

源码入口：

- `kbe/src/server/baseapp/entity.cpp`
- `kbe/src/server/cellapp/entity.cpp`
- `kbe/src/server/baseapp/proxy.cpp`

比较典型的有：

- `onGetWitness`
  客户端控制关系建立或恢复时调用
- `onWriteToDB`
  写库收束时调用
- `onPreArchive`
  Base 侧真正打持久化流前调用

这一类回调的特点是：

- 时机稳定
- 由引擎明确主导
- 通常和实体运行状态切换强绑定

## 14.4 异步结果回调不是“钩子”，而是 `callbackMgr` 托管结果

源码入口：

- `kbe/src/lib/server/callbackmgr.h`
- `kbe/src/server/baseapp/entity.cpp`
- `kbe/src/server/baseapp/baseapp.cpp`
- `kbe/src/server/dbmgr/dbmgr.h`
- `kbe/src/server/tools/interfaces/interfaces.h`

`CallbackMgr` 的思路非常直接：

1. 发起异步操作时，把 Python 回调保存进 map
2. 为它分配一个 `callbackID`
3. 把 `callbackID` 随消息带出去
4. 结果回来后，通过 `take(callbackID)` 取回原回调
5. 执行回调并收束状态

这套机制用在很多地方：

- 实体写库
- 创建实体
- 删除实体
- interfaces / 外部接口结果

它和生命周期钩子的最大区别在于：

- 生命周期钩子不需要你先注册
- 异步结果回调必须由某次操作显式绑定一个 `callbackID`

这是两种完全不同的模型。

## 14.5 定时器机制的真实实现

源码入口：

- `kbe/src/lib/server/script_timers.h`
- `kbe/src/lib/server/script_timers.cpp`
- `kbe/src/server/baseapp/entity_component.cpp`
- `kbe/src/server/cellapp/entity_component.cpp`

从 `ScriptTimers` 可以看出，这套机制非常朴素但很实用：

- 内部维护 `ScriptID -> TimerHandle` 的映射
- `addTimer(initialOffset, repeatOffset, userArg, handler)`
  真正向底层定时器系统注册
- 成功后返回脚本侧 timerID
- 到时后通过 `TimerHandler` 回调
- 释放时移除映射

这说明脚本看到的 timerID 不是底层 `TimerHandle`，而是额外包了一层脚本友好的标识。

从 `ScriptTimersUtil` 可以进一步看出：

- `ScriptTimers` 可以按需创建
- 空了之后还能自动释放

这是一个比较典型的“脚本对象友好包装层”。

## 14.6 为什么说定时器不只是调度工具，还承担恢复语义

这一点很多人第一次读源码时会忽略。

从 `cellapp/entity.cpp` 可见，实体还会把 timer 信息写入流：

- `addTimersToStream(...)`

同时事件也能通过：

- `createEventsFromStream(...)`

恢复出来。

这意味着在 KBEngine 的设计里，定时器和事件注册关系不是纯瞬时状态，而是：

- 在某些迁移 / 恢复场景下，需要跟实体状态一起恢复

所以这套机制比普通 Web 服务器里“随手 setTimeout 一个回调”更重。

## 14.7 事件注册机制的含义

从 `cellapp/entity.cpp` 的 `createEventsFromStream` 可以看到：

- 事件名会被恢复
- 回调名会被恢复
- 之后再通过 `registerEvent(eventName, pyCallback)` 重新挂回实体

这说明事件系统的重点不是“发一个消息给订阅者”，而是：

- 实体在运行期间维护一组事件响应关系
- 这组关系在迁移 / 恢复后还能重建

也就是说，KBEngine 的事件更接近实体运行态的一部分，而不是外挂式观察者工具。

## 14.8 一个容易被忽略的事实：同名回调在不同组件上语义可能不同

例如：

- `onInit`
  在不同组件上触发阶段和参数就可能不同
- `onReadyForLogin`
  在 BaseApp / CellApp 上都可能出现，但表示的是各自组件是否准备好接入
- `onWriteToDB`
  Base 和 Cell 都可能调用，但各自负责的数据收束上下文不同

所以看 API 名字远远不够，必须回到：

- 它在什么组件里
- 是谁调用它
- 调用前后系统状态是什么

这也是为什么你要把源码章节和 API 文档彻底分开。

## 14.9 一条建议的源码走读路径

如果想把这一章真正读懂，我建议按这个顺序：

1. `baseapp/initprogress_handler.cpp`
   看 `onBaseAppReady`、`onReadyForLogin`
2. `cellapp/initprogress_handler.cpp`
   看 `onInit`、`onReadyForLogin`
3. `baseapp/entity.cpp`
   看 `onPreArchive`、`onWriteToDB`
4. `baseapp/proxy.cpp` 与 `cellapp/entity.cpp`
   看 `onGetWitness`
5. `lib/server/callbackmgr.h`
   看异步回调保存 / 取回模型
6. `lib/server/script_timers.cpp`
   看 timerID 和底层 timer 的关系
7. `cellapp/entity.cpp`
   看事件 / 定时器恢复相关逻辑

## 14.10 这套设计的优点与代价

### 优点

- 脚本入口足够统一，开发体验较自然
- 生命周期、异步回调、定时器都有稳定落点
- 与实体迁移 / 恢复体系兼容
- 组件级语义可以通过不同 C++ 触发点清晰表达

### 代价

- 同名回调跨组件语义不完全一致
- 入口多，初学者容易混淆
- 调试时必须同时理解 Python 层和 C++ 触发点
- 回调过多时，业务时序容易变得不透明

## 14.11 本章小结

这一章最重要的收获应该是：

- 生命周期钩子是引擎主动调用你
- 异步结果回调是你先注册，再由 `callbackMgr` 带回来
- 定时器回调是脚本对底层计时器系统的包装
- 事件注册关系在某些场景下也是实体运行态的一部分

把这四类分清楚之后，再去读 KBEngine 脚本层行为，很多以前看起来混乱的“回调”就会突然清楚很多。
