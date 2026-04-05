# 12. 玩家完整生命周期

> 本文件是旧稿，已由 [`22-player-complete-lifecycle.md`](./22-player-complete-lifecycle.md) 取代。阅读主线请直接进入新稿。

> 这一章把前面分散的机制收束成一条完整主线：一个玩家从登录，到进入世界，到同步、写库、下线，数据究竟怎么走。

## 16.1 本章要回答的问题

- 一个玩家从客户端发起登录开始，到真正进入世界，中间依次经过哪些组件？
- 玩家实体在 Base、Cell、Client 三侧是怎样建立和变化的？
- 属性、远程调用、空间同步、持久化这些数据分别沿着什么路径流动？
- 玩家下线、掉线、重连、写库时，系统状态如何收束？

## 16.2 先建立一张主流程图

可以先把玩家生命周期粗分成 7 个阶段：

1. 客户端连接 `Loginapp`，提交账号与密码。
2. `Loginapp` 把账号查询请求转给 `Dbmgr`。
3. `Dbmgr` 返回账号状态、实体 ID、数据库 ID、账号标记等信息。
4. `Loginapp` 向 `Baseappmgr` 申请一个可用 `Baseapp`，然后把地址回给客户端。
5. 客户端再连到 `Baseapp`，执行 `loginBaseapp`。
6. `Baseapp` 找到或恢复 Base 实体，并在需要时创建 Cell 实体。
7. Cell 侧建立 `Witness`，客户端开始接收实体进入视野、属性更新和远程方法调用。

如果只看这一层，KBEngine 的设计特点已经很清楚了：

- 登录入口和实际会话入口是分开的。
- 持久化查询与在线实体管理是分开的。
- 非空间逻辑主要停留在 Base 侧。
- 空间内可见性与实时同步主要停留在 Cell 侧。

## 16.3 阶段一：登录请求先到 Loginapp，不直接进 Baseapp

这一阶段最重要的事实是：客户端第一次登录不是直接连 `Baseapp`，而是先通过 `Loginapp` 做接入和分发。

相关资料与源码入口：

- 协议消息定义：`kbe/res/server/messages_fixed_defaults.xml`
- `Client::onLoginSuccessfully`
- `Client::onLoginFailed`
- `Baseapp::loginBaseapp`
- 处理登录查询结果：`kbe/src/server/loginapp/loginapp.cpp`
  `Loginapp::onLoginAccountQueryResultFromDbmgr`

从 `Loginapp::onLoginAccountQueryResultFromDbmgr` 可以直接看出它处理的输入信息包括：

- `loginName`
- `accountName`
- `password`
- `componentID`
- `entityID`
- `dbid`
- `flags`
- `deadline`
- 附带的 `datas`

这说明 `Dbmgr` 返回的不是“账号密码对不对”这么简单，而是一整组和账号在线状态、实体状态、账号限制相关的数据。

这一层的关键设计意图是：

- `Loginapp` 负责接入与调度，不负责长期持有玩家实体。
- 真正和玩家长期绑定的会话与实体管理，应该落到 `Baseapp`。

## 16.4 阶段二：Dbmgr 决定“账号 / 实体”的基础状态

从 `kbe/src/server/dbmgr/dbtasks.cpp` 可以看到登录相关任务最终会回包给：

- `LoginappInterface::onLoginAccountQueryResultFromDbmgr`

而在 `Loginapp::onLoginAccountQueryResultFromDbmgr` 中，`Dbmgr` 返回的内容至少影响以下判断：

- 账号是否被锁定
- 账号是否未激活
- 账号是否过期
- 当前账号是否已经在某个 `Baseapp` 上存活

这里有一个非常关键的点：

- 如果 `componentID > 0`，说明当前账号仍然挂在某个 `Baseapp` 上。
- 这意味着登录不是单纯“查库后创建新实体”，而是先判断“已有在线上下文是否存在”。

这也是为什么 `Dbmgr` 在 KBEngine 中不是一个简单的 SQL 代理。它不仅知道数据库，还参与“在线状态与恢复语义”的协作。

## 16.5 阶段三：Loginapp 向 Baseappmgr 申请可用 Baseapp

当 `Loginapp` 拿到 `Dbmgr` 的结果之后，下一步不是直接通知客户端去连某个固定地址，而是通过 `Baseappmgr` 申请或定位目标 `Baseapp`。

源码入口：

- `kbe/src/server/loginapp/loginapp.cpp`
  `Loginapp::onLoginAccountQueryResultFromDbmgr`
  `Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr`

这里分成两种情况：

- `componentID > 0`
  说明账号已经挂在某个 `Baseapp`，于是走 `registerPendingAccountToBaseappAddr`
- `componentID == 0`
  说明没有在线上下文，于是走 `registerPendingAccountToBaseapp`

然后 `Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr` 把最终地址回给客户端，发送：

- `ClientInterface::onLoginSuccessfully`

这里值得注意的一点是：

- `Loginapp` 回给客户端的不是“你已经登录完成”，而是“去连这个 Baseapp”。
- 所以从玩家体验看，登录像一步；从服务器设计看，它其实是“两跳接入”。

## 16.6 阶段四：客户端真正把会话绑定到 Baseapp

真正的玩家在线会话，是在 `Baseapp::loginBaseapp` 里建立的。

源码入口：

- `kbe/src/server/baseapp/baseapp.cpp`
  `Baseapp::loginBaseapp`
  `Baseapp::loginBaseappFailed`

从 `Baseapp::loginBaseapp` 可以看到这一阶段至少会检查：

- 账号名长度与密码长度
- `Dbmgr` 是否就绪
- `PendingLoginMgr` 中是否存在对应待登录记录
- 请求来源地址是否与待登录记录一致
- 密码是否匹配
- 账号标记是否允许登录

也就是说，`Baseapp` 不是盲目接收任何来自客户端的 `loginBaseapp` 请求，而是必须和前一跳 `Loginapp` 留下的待登录状态对上。

这背后的设计意图很明确：

- `Loginapp` 负责第一跳接入。
- `Baseapp` 只接管经过预分配和登记的会话。
- 这样才能防止客户端绕过 `Loginapp` 直接伪造登录。

再往下看 `Baseapp::loginBaseapp` 的实现，可以看到“已在线账号再次登录”并不是一个单纯错误分支，而是一套明确的会话接管策略：

- 如果 `ptinfos->entityID > 0`，说明 `Dbmgr` 已经告诉 `Baseapp`：这个账号挂着一个存活中的实体。
- `Baseapp` 会先通过 `findEntity(ptinfos->entityID)` 找到本地 `Proxy`。
- 然后调用脚本钩子 `Proxy::onLogOnAttempt(...)`，把“是否允许新客户端挤掉旧客户端”交给脚本层决定。

这个判断很关键，因为它说明 KBEngine 的“登录”本质上不只是认证，也是会话控制权仲裁。

当脚本返回 `LOG_ON_ACCEPT` 时，底层动作不是“重新创建实体”，而是：

- 若旧 `clientEntityCall` 仍在，则先踢掉旧客户端通道。
- 把 `Proxy` 重新绑定到新的客户端地址。
- 重新执行 `createClientProxies(...)`。
- 最后调用 `Proxy::onGetWitness()`，把客户端控制权恢复继续下推到 Cell 侧。

所以 `Baseapp::loginBaseapp` 最核心的意义是：

- 把“待登录记录”收束成一个真实在线的 `Proxy`
- 决定是接管现有实体，还是进入后续的账号恢复流程

## 16.7 阶段五：Base 实体的建立与接管

玩家真正“活”起来，取决于 Base 实体是否被找到或创建。

这部分的关键入口不止一个，最重要的是：

- `Baseapp::createEntityFromDBID`
- `Baseapp::createEntityAnywhereFromDBID`
- `Baseapp::onCreateEntityFromDBIDCallback`

源码位置：

- `kbe/src/server/baseapp/baseapp.cpp`

从这里可以看出，KBEngine 在 Base 侧处理实体恢复时会区分几种情况：

- 数据库中查到了实体，并且它当前未被激活
- 数据库中查到了实体，但实体已经在某个 `Baseapp` 上激活
- 查询失败

其中一个很有代表性的设计是：

- 当实体已经激活但当前进程拿不到本地对象时，代码会退化为构造一个 `EntityCall`
- 这说明在 KBEngine 的语义里，“远端实体引用”本身就是一等对象

这和后面 `RPC / EntityCall` 章节是直接相连的。

如果账号此前还没有在线实体，那么 `Baseapp::loginBaseapp` 会改为向 `Dbmgr` 发送 `queryAccount`。随后 `Baseapp::onQueryAccountCBFromDbmgr` 才是真正把账号对象恢复出来的关键落点。

这一段源码建议一定要仔细看，因为它非常集中地体现了 KBEngine 的账号恢复模型：

- `createEntity(...)` 创建账号对应的 `Proxy`
- 安装 `dbid`、客户端类型、登录附加数据
- 通过 `createDictDataFromPersistentStream(...)` 从持久化流恢复脚本属性
- 调用 `initializeEntity(pyDict)` 完成脚本对象初始化
- 若客户端连接还在，则立即构造 `clientEntityCall` 并执行 `createClientProxies(...)`

也就是说，玩家真正拿到可运行 Base 实体的常见链路，其实是：

`loginBaseapp -> DbgmgrInterface::queryAccount -> Baseapp::onQueryAccountCBFromDbmgr -> createEntity -> initializeEntity`

这比“登录成功后创建玩家对象”要细得多，也更贴近源码真实路径。

## 16.8 阶段六：Base 实体创建 Cell 实体，玩家进入世界

玩家有了 Base 实体，不代表已经进入世界。进入空间发生在 Base 侧触发 Cell 实体创建之后。

相关入口：

- `Entity::createCellEntity`
- `Entity::createCellEntityInNewSpace`
- `Entity::restoreCell`
- `Baseapp::createCellEntity`
- `Baseapp::createCellEntityInNewSpace`
- `Baseapp::onEntityGetCell`

源码位置：

- `kbe/src/server/baseapp/entity.cpp`
- `kbe/src/server/baseapp/baseapp.cpp`

从 `Baseapp::createCellEntity` 可以直接看到这一阶段的关键动作：

- 构造 `CellappInterface::onCreateCellEntityFromBaseapp` 消息
- 把 `entityType`、`entity id`、`base componentID`
- 以及通过 `addCellDataToStream()` 序列化出来的 Cell 初始数据
  一起发给目标 `Cellapp`

这里说明了两件事：

1. 玩家进入世界，本质上是“Base 侧把 Cell 所需初始状态序列化后交给 Cell 侧创建运行时对象”。
2. Base / Cell 的分离不是概念分离，而是真有一次跨组件的数据交接。

把这一段放回玩家生命周期里看，最好把“进入世界”拆成三小步：

1. Base 上先存在一个已经与客户端绑定的 `Proxy`
2. `Entity::createCellEntity` 把 Cell 初始状态序列化后交给目标 `Cellapp`
3. Cell 创建完成后，再通过 `Proxy::onGetWitness()` 把“客户端控制权”继续绑定到 Cell 侧

所以玩家进入世界并不是一次构造动作，而是：

- Base 逻辑实体先建立
- Cell 世界实体再建立
- Witness 最后把客户端表现层接上

## 16.9 阶段七：Cell 侧建立 Witness，客户端开始看到世界

当 Cell 实体建立后，玩家真正开始看到周围世界，是由 `Witness` 驱动的。

关键源码：

- `kbe/src/server/cellapp/witness.cpp`
  `Witness::onEnterSpace`
  `Witness::installViewTrigger`
  `Witness::onEnterView`
  `Witness::onLeaveView`
  `Witness::update`

从 `Witness::onEnterView` 可以直接看出：

- 当其他实体进入视野，`Witness` 会创建或更新 `EntityRef`
- 把该实体放进 `viewEntities_`
- 标记为 `ENTITYREF_FLAG_ENTER_CLIENT_PENDING`
- 触发 `addWitnessed` 与 `onEnteredView`

这说明客户端看到“某个实体出现在屏幕上”，在服务器内部并不是一个 UI 事件，而是：

- 空间索引判断进入 AOI
- Witness 记录进入事件
- 后续 `update()` 把进入 / 离开 / 属性变化真正发给客户端

所以从玩家生命周期看，真正“进入世界”的完成标志不是 Base 实体存在，而是：

- Cell 实体存在
- Witness 建立
- 客户端开始收到视野内实体流

这里还要把 `Proxy::onGetWitness -> CellappInterface::onGetWitnessFromBase -> Entity::onGetWitness(true)` 单独记成一条主线，因为它同时承担三类场景：

- 正常登录后的首次绑定
- 客户端断线后的重连
- `Proxy::giveClientTo` 这种控制权迁移

从 `cellapp/entity.cpp` 的 `Entity::onGetWitness(bool fromBase)` 可以看出，`fromBase == true` 时系统会额外做这些动作：

- 如果当前没有 `clientEntityCall`，就从 `baseEntityCall().client` 重新取回客户端引用
- 主动把 `spaceID` 和客户端属性重新打包发给客户端
- 如果还没有 `Witness` 就创建；如果已有 `Witness`，则执行 `onAttach()` 与 `resetViewEntities()`，让客户端重新建立视野

这说明 KBEngine 对“重连恢复”的处理不是简单补一个 socket，而是显式要求：

- 重新绑定客户端控制对象
- 重新同步关键客户端属性
- 重新建立视野状态

## 16.10 属性、方法、持久化三类数据的流向

### 属性更新

典型路径是：

1. 脚本层修改实体属性
2. 运行时对象更新内部状态
3. 相关更新进入 Base / Cell 的同步链路
4. Cell 侧通过 `Witness` 决定哪些客户端需要收到更新

这说明“属性更新”不是简单地广播，而是被实体位置和视野约束。

### 远程方法调用

典型路径是：

1. 脚本层发起 `EntityCall`
2. 根据实体定义查找方法与参数信息
3. 参数序列化进入 `Bundle / MemoryStream`
4. 网络层发送到目标组件
5. 目标端解包并调用对应对象

这部分的细节在第 8 章展开，但在玩家生命周期里，它扮演的是“跨 Base / Cell / Client 协作的动作通道”。

### 持久化

最典型的入口是：

- `kbe/src/server/baseapp/entity.cpp`
  `Entity::writeToDB`

从这段代码可以直接看出：

- 如果当前还没有 Cell 实体，Base 侧直接继续写库
- 如果已经有 Cell 实体，Base 会先向 Cell 发送 `reqWriteToDBFromBaseapp`

也就是说，写库前要先把 Cell 侧状态收束回来，不能假设 Base 侧掌握完整实时状态。

随后 `Entity::onCellWriteToDBCompleted` 会：

- 调用 `onPreArchive`
- `addPersistentsDataToStream`
- 再把持久化数据发往 `Dbmgr`

如果把这条链按函数展开，完整路径其实是：

1. `Baseapp::Entity::writeToDB`
2. `Cellapp::reqWriteToDBFromBaseapp`
3. `Cellapp::Entity::writeToDB`
4. `Baseapp::onCellWriteToDBCompleted`
5. `Baseapp::Entity::onCellWriteToDBCompleted`
6. `Dbmgr::writeEntity`
7. `DBTaskWriteEntity::presentMainThread`
8. `Baseapp::onWriteToDBCallback`
9. `Baseapp::Entity::onWriteToDBCallback`

中间每一步的职责都不一样：

- Cell 侧先执行 `onWriteToDB()` 和 `backupCellData()`，把空间侧运行时状态收束好
- Base 侧执行 `onPreArchive` 并统一序列化持久化字段
- `Dbmgr` 负责真正数据库写入与在线日志维护
- 最后的 Python 回调由 Base 实体收尾执行

这说明 `writeToDB()` 完全不是“本地对象直接写数据库”，而是一条跨 Base / Cell / DBMgr 的状态收束流水线

这恰好说明 KBEngine 的持久化是三段式的：

1. 收束 Cell 侧状态
2. Base 侧整理持久化数据
3. DBMgr 负责真正数据库交互

## 16.11 下线、重连与恢复

在玩家生命周期里，下线不是简单的“断连接”。

从现有代码线索可以看出，系统至少要处理：

- 普通登录失败
- 重登录 `reloginBaseapp`
- `Proxy::giveClientTo` 一类客户端控制权转移
- 已在线实体恢复
- 空间恢复与 `restoreCell`

相关源码入口：

- `kbe/src/server/baseapp/baseapp.cpp`
  `Baseapp::reloginBaseapp`
- `kbe/src/server/baseapp/proxy.cpp`
  `Proxy::giveClientTo`
- `kbe/src/server/baseapp/restore_entity_handler.cpp`

这些机制共同说明：

- “玩家在线状态”并不完全等于“TCP 连接还在”
- KBEngine 允许一定程度上的连接重接管和实体恢复

这也是 MMO 服务器设计里和普通请求响应式服务器很不一样的地方。

如果进一步落到关键函数，至少要把下面三条区分清楚：

### 普通下线

`Baseapp::logoutBaseapp` 做的主要是：

- 找到目标 `Proxy`
- 校验 `rndUUID`
- 把客户端通道 `condemn`

也就是说，这一步首先收束的是“连接关系”，不一定意味着实体立即销毁。

### 重连

`Baseapp::reloginBaseapp` 的重点不是重新创建实体，而是重新把新客户端挂回已有 `Proxy`：

- 校验 `entityID` 与 `rndUUID`
- 若旧 `clientEntityCall` 还在，则先踢掉旧通道
- 把客户端地址改成新地址
- 重新执行 `createClientProxies(proxy, true)`
- 再通过 `proxy->onGetWitness()` 通知 Cell 侧恢复控制权
- 最后回给客户端 `onReloginBaseappSuccessfully`

这里的重点是：重连成功包含两层恢复

- Base 层恢复会话
- Cell 层恢复世界表现

### 控制权转移

`Proxy::giveClientTo` 更进一步，它表达的是“客户端控制哪个 Proxy”本身也是可以切换的。

这一条链里会发生：

- 旧 `Proxy` 如有 Cell，则先向 Cell 发送 `onLoseWitness`
- 客户端先收到 `onEntityDestroyed`，删除旧控制实体
- 新 `Proxy` 创建自己的 `clientEntityCall`
- `Baseapp::createClientProxies(...)` 把新控制实体同步给客户端
- `Proxy::onGetWitness()` 再次驱动 Cell 侧建立 Witness / 视野

这段代码非常值得反复看，因为它揭示了 KBEngine 的一个核心抽象：

- 客户端连接
- Base `Proxy`
- Cell 实体
- Witness

这四者彼此关联，但不是同一个对象。正因为拆开了，系统才有能力支持重连、挤号和控制权迁移。

## 16.12 本章小结

如果把玩家生命周期压缩成一句话，可以这样概括：

- `Loginapp` 负责接入与分发
- `Dbmgr` 负责账号 / 实体基础状态查询
- `Baseappmgr` 负责把会话挂到合适的 `Baseapp`
- `Baseapp` 负责玩家的长期逻辑主体
- `Cellapp` 负责玩家进入空间后的实时世界状态
- `Witness` 负责把“世界里谁该被看到”变成真正的客户端同步
- `Dbmgr` 再次负责最终持久化落库

所以理解 KBEngine 玩家生命周期的关键，不是记住几个组件名字，而是记住三条主线：

- 会话主线：`Loginapp -> Baseapp`
- 世界主线：`Base -> Cell -> Witness -> Client`
- 数据主线：`Cell -> Base -> DBMgr`

再往前走一步，还应该补上一条：

- 恢复主线：`Proxy / clientEntityCall / Witness`

只有把这四条主线放在一起，玩家生命周期才真正闭环。
