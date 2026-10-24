# 8. RPC、EntityCall 与远程方法调用

> 这是理解 KBEngine 的核心章节之一。很多人把它笼统地叫成“RPC”，但如果不把 `EntityCall`、实体定义、网络消息三层拆开，就很容易看错。

## 8.1 本章要回答的问题

- KBEngine 的“RPC”到底是什么，不是什么？
- `EntityCall` 在模型里扮演什么角色？
- 一个脚本层远程方法调用如何变成底层网络消息？
- Base / Cell / Client 三侧调用边界是如何被约束的？

## 8.2 先给结论：KBEngine 的 RPC 不是通用服务 RPC

如果用一句话概括，KBEngine 的远程调用本质是：

- 以实体为中心
- 以实体定义为协议
- 以 `EntityCall` 为远端引用
- 以 `MethodDescription + MemoryStream + Bundle` 为封包机制

所以它不是典型的通用服务 RPC 框架，不是：

- 任意服务名 + 方法名调用
- 运行时动态发现服务
- 面向接口注册中心的通用分布式调用

它更像：

- “我手里有一个远端实体引用”
- “这个引用知道自己指向 Base / Cell / Client 哪一侧”
- “我调用它暴露的方法时，底层会按实体定义把参数序列化并发往正确组件”

这正是 BigWorld / KBEngine 一系设计最关键的风格之一。

## 8.3 `EntityCall` 是什么

源码入口：

- `kbe/src/lib/entitydef/entity_call.cpp`
- `kbe/src/lib/entitydef/entitycallabstract.cpp`
- `kbe/src/lib/entitydef/remote_entity_method.cpp`

`EntityCall` 不是一次“调用请求”，而是一个长期可持有的远端实体引用。这个引用至少带着几类信息：

- 目标实体 ID
- 目标组件地址 / 组件 ID
- 调用类型 `type_`
  例如 Base、Cell、Client
- 目标实体对应的 `ScriptDefModule`

所以脚本层写出：

```python
self.cell.moveTo(x, y, z)
```

底层并不是立刻查一个全局方法表，而是：

1. `self.cell` 先得到一个 `EntityCall`
2. 访问 `moveTo` 时，通过实体定义拿到 `MethodDescription`
3. 再生成一个可调用的 `RemoteEntityMethod`
4. 执行时由这个对象做参数检查、序列化和发包

这就是为什么理解 KBEngine 的 RPC，第一步不是看网络层，而是先看 `EntityCall`。

## 8.4 方法定义不是散落代码里，而是先固化进实体定义

源码入口：

- `kbe/src/lib/entitydef/entitydef.cpp`
- `kbe/src/lib/entitydef/scriptdef_module.cpp`
- `kbe/src/lib/entitydef/method.*`

从 `EntityDef::loadDefBaseMethods`、`loadDefCellMethods`、`loadDefClientMethods` 可以直接看出，方法定义加载阶段就确定了几件关键事情：

- 方法名
- 参数类型列表
- 方法所属侧
  Base / Cell / Client
- 是否 `Exposed`
- 方法 `Utype`

这里的 `Utype` 非常关键。它不是装饰信息，而是实际协议字段。若 XML 中没显式指定，系统会自动分配；若重复，则加载阶段就报冲突。

所以 KBEngine 的远程方法不是靠字符串方法名在运行时传来传去，而是：

- 定义阶段生成 `MethodDescription`
- 运行阶段用 `Utype` 和参数描述驱动序列化与反序列化

这样做的直接收益是：

- 包更小
- 路由更快
- 协议更稳定

代价则是：

- 改定义要更谨慎
- 方法签名演进成本高于纯动态脚本系统

## 8.5 脚本层一次远程调用是怎样落到网络层的

最基础的主链在：

- `kbe/src/lib/entitydef/remote_entity_method.cpp`
  `RemoteEntityMethod::tp_call`

这段代码非常值得逐行看，因为它几乎就是 KBEngine RPC 的最短闭环：

1. `methodDescription->checkArgs(args)`
   先按实体定义检查参数合法性
2. `methodDescription->addToStream(mstream, args)`
   把 Python 参数编码进 `MemoryStream`
3. `entityCall->newCall((*pSendBundle))`
   让 `EntityCall` 写入消息头和路由信息
4. 把参数流 append 到 `Bundle`
5. `entityCall->sendCall(pSendBundle)`
   通过目标 `Channel` 真正发出去

这里最重要的一点是：

- `RemoteEntityMethod` 负责“方法级语义”
- `EntityCall` 负责“目标级语义”

也就是：

- 这个方法怎么检查、怎么编码
- 这个包该发给谁、用什么消息类型发

这两件事是拆开的。

## 8.6 `EntityCall::newCall_` 真正决定“发给谁”

源码入口：

- `kbe/src/lib/entitydef/entitycallabstract.cpp`
  `EntityCallAbstract::newCall_`

这是另一个必须重点看的函数。它直接把 KBEngine 的远程调用模型写死在分支里了。

### 服务端发起调用

当当前不是 `CLIENT_TYPE / BOTS_TYPE` 时：

- 若 `componentID_ == 0`
  说明目标是客户端，于是使用 `ClientInterface::onRemoteMethodCall`
- 若 `componentID_ > 0`
  则先找目标组件
  - 目标是 `BASEAPP_TYPE`，使用 `BaseappInterface::onEntityCall`
  - 否则走 `CellappInterface::onEntityCall`

随后再写入：

- `entityID`
- 若目标是服务端，还会附加 `type_`

### 客户端发起调用

当当前是客户端或 bots：

- 调 Base 用 `BaseappInterface::onRemoteMethodCall`
- 调 Cell 用 `BaseappInterface::onRemoteCallCellMethodFromClient`

这说明两件事：

1. KBEngine 的 RPC 路由不是统一总线，而是显式地按组件类型分消息。
2. 客户端并不直接连 Cell；很多调用先到 Base，再由 Base 转发或仲裁。

这和 MMO 的信任边界直接相关。

## 8.7 Base / Cell / Client 三侧为什么会有不同“远程方法对象”

源码入口：

- `kbe/src/server/baseapp/entity_remotemethod.cpp`
- `kbe/src/server/cellapp/entity_remotemethod.cpp`
- `kbe/src/server/cellapp/client_entity_method.cpp`
- `kbe/src/server/cellapp/clients_remote_entity_method.cpp`
- `kbe/src/server/cellapp/real_entity_method.cpp`

这部分很容易被忽略，但它正是“同样叫远程调用，为何不同侧代码长得不一样”的原因。

### Base 侧 `EntityRemoteMethod`

`baseapp/entity_remotemethod.cpp` 里有一个很有代表性的分支：

- 若目标不是客户端，直接复用通用 `RemoteEntityMethod`
- 若目标是客户端，则走特殊路径

特殊路径会额外做：

- 事件统计
- 带宽统计
- 通过 `Proxy::sendToClient(...)` 发往真实客户端

所以 Base 侧“调客户端方法”不是简单走同一个模板，而是和玩家连接、统计、代理能力绑定在一起的。

### Cell 侧 `ClientEntityMethod`

`cellapp/client_entity_method.cpp` 更能体现 KBEngine 的场景语义。它在真正发给客户端之前会先检查：

- 源实体是否存在
- 是否 real entity
- 是否有 `Witness`
- 目标实体是否已经在这个客户端视野中

只有这些前提都满足，才会继续：

- 生成远程方法参数流
- 根据是否有 alias 选择 `onRemoteMethodCall` 或优化消息
- 通过 `Witness` 发给该客户端

所以 Cell 侧“调客户端方法”不是“只要你有实体就能调”，而是强绑定 AOI / Witness 语义。

### Ghost 上的 `RealEntityMethod`

在 `cellapp/entity_component.cpp` 可以看到：

- 如果组件 owner 是 ghost entity
- 访问 cell 方法时会返回 `RealEntityMethod`

这说明 ghost 并不直接执行本地逻辑，而是再转一次到 real entity。也就是说：

- 同一份脚本接口
- 在 real / ghost 上可能对应不同执行语义

这是分布式空间同步模型里非常关键的一层。

## 8.8 一个完整的函数级走读路径

如果你要在 IDE 里真正跟一条 RPC，我建议按下面顺序：

1. `kbe/src/lib/entitydef/entity_call.cpp`
   看 `EntityCall::onScriptGetAttribute`
2. `kbe/src/lib/entitydef/remote_entity_method.cpp`
   看 `RemoteEntityMethod::tp_call`
3. `kbe/src/lib/entitydef/entitycallabstract.cpp`
   看 `EntityCallAbstract::newCall_`
4. 对应侧的专用方法对象
   Base 看 `server/baseapp/entity_remotemethod.cpp`
   Cell 看 `server/cellapp/client_entity_method.cpp`
5. 目标端接口消息处理
   看 `BaseappInterface::*` / `CellappInterface::*` 的落地逻辑

跟的时候要一直问自己三件事：

- 这个调用的“协议描述”来自哪份实体定义？
- 这个调用的“目标语义”是 Base、Cell 还是 Client？
- 这个调用是否受 Witness / 在线连接 / real-ghost 关系约束？

## 8.9 这套设计为什么适合 KBEngine

它的优点很明确：

- 协议和实体模型强绑定，脚本体验自然
- 方法 ID 化后消息紧凑
- `EntityCall` 作为一等对象，天然支持远端引用、恢复和转移
- 能把 Base / Cell / Client 三侧语义直接编码进调用路径

它的代价也同样明确：

- 协议演进灵活性不如通用 JSON / Protobuf RPC
- 代码入口分散，初学者容易迷路
- 不同侧存在专用调用类，理解门槛较高
- 一旦实体定义设计不稳，后续维护成本会迅速上升

## 8.10 本章小结

理解 KBEngine 的 RPC，最重要的不是背接口名，而是把下面这条链立住：

- 实体定义生成 `MethodDescription`
- `EntityCall` 表示远端实体引用
- 访问远程方法时生成 `RemoteEntityMethod`
- `checkArgs + addToStream` 负责方法级协议
- `newCall_ + sendCall` 负责目标级路由
- 不同侧的专用方法类再叠加 Client / Witness / real-ghost 语义

只有把这一整层看清楚，后面理解属性同步、客户端调用、控制权转移时才不会乱。
