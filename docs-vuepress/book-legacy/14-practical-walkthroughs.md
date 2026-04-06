# 14. 实战源码走读

> 本文件是旧稿，已由 [`24-practical-source-walkthroughs.md`](/study/24-practical-source-walkthroughs.html) 取代。阅读主线请直接进入新稿。

> 这一章不再重复概念，而是给出几条可以拿着 IDE 直接跟的路径。

## 17.1 怎么使用这一章

建议每个实战都按同样方式阅读：

1. 先看“起点调用”
2. 再看“跨组件消息”
3. 再看“目标端落地”
4. 最后看“回调或状态收束”

本章的价值在于：把前面的机制知识真正串成“能跟代码走”的路径。

## 17.2 实战一：一次登录请求怎样进入 Baseapp

### 目标

回答“玩家输入账号密码之后，到底怎样拿到 Baseapp 地址并进入 Baseapp”。

### 推荐起点

- `kbe/src/server/loginapp/loginapp.cpp`
  `Loginapp::onLoginAccountQueryResultFromDbmgr`
  `Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr`
- `kbe/src/server/baseapp/baseapp.cpp`
  `Baseapp::loginBaseapp`

### 建议跟法

1. 从 `Dbmgr` 回包给 `Loginapp` 开始看。
2. 看 `Loginapp` 如何区分：
   已在线账号
   未在线账号
3. 看它怎样请求 `Baseappmgr` 分配或定位目标 `Baseapp`。
4. 看它怎样通过 `ClientInterface::onLoginSuccessfully` 把地址回给客户端。
5. 再跳到 `Baseapp::loginBaseapp` 看第二跳登录如何真正完成。

### 函数级走读路径

建议按下面顺序点开：

1. `kbe/src/server/dbmgr/dbtasks.cpp`
   看登录查询结果最终如何回给 `LoginappInterface::onLoginAccountQueryResultFromDbmgr`
2. `kbe/src/server/loginapp/loginapp.cpp`
   看 `Loginapp::onLoginAccountQueryResultFromDbmgr`
3. 同文件继续看
   `Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr`
4. `kbe/src/server/baseapp/baseapp.cpp`
   看 `Baseapp::loginBaseapp`
5. 同文件继续看
   `Baseapp::onQueryAccountCBFromDbmgr`

### 走读时要特别盯住的状态

- `componentID > 0`
  说明账号已经绑定在线实体，不一定需要新建账号对象。
- `PendingLoginMgr`
  这是第二跳登录能否被接受的关键凭据。
- `ptinfos->entityID`
  决定接下来是接管旧实体，还是向 `Dbmgr` 恢复新实体。

### 这一条链的关键认识

- “登录成功”分成两步：
  第一步是拿到 Baseapp 地址
  第二步是把会话真正挂到 Baseapp

真正的收束点至少有两个：

- `Baseapp::loginBaseapp`
  第二跳认证通过，会话开始归属某个 `Proxy`
- `Baseapp::onQueryAccountCBFromDbmgr`
  账号实体真正创建并完成初始化

## 17.3 实战二：一次 EntityCall 怎样发出去并落地

### 目标

回答“脚本层调用一个远程方法时，底层到底做了什么”。

### 推荐起点

- `kbe/src/server/baseapp/entity_remotemethod.*`
- `kbe/src/server/cellapp/entity_remotemethod.*`
- `kbe/src/server/cellapp/real_entity_method.*`
- `kbe/src/server/cellapp/client_entity_method.*`
- `kbe/src/lib/entitydef`

### 建议跟法

1. 先找到脚本层方法是如何被映射到定义信息的。
2. 看方法 ID、参数描述如何被解析。
3. 看参数如何进入 `MemoryStream / Bundle`。
4. 看消息怎样通过网络发往目标组件。
5. 看目标端如何反查定义并真正执行。

### 这一条链的关键认识

- KBEngine 的 RPC 不是通用服务调用框架，而是围绕实体定义构造的一套远程方法系统。

### 跟这条链时别漏掉的检查点

- 方法 ID 从哪里来
- 参数描述由谁解释
- `Bundle` 真正在哪一层构造
- 目标端如何通过实体定义反查到脚本方法

如果只看到了 `sendCall()`，那还没有真正看懂；必须继续跟到“定义查找”和“目标方法执行”。

## 17.4 实战三：一个属性更新如何同步给客户端

### 目标

回答“脚本层改一个属性之后，为什么最后只有视野内的客户端会收到更新”。

### 推荐起点

- `kbe/src/server/baseapp/entity.*`
- `kbe/src/server/cellapp/entity.*`
- `kbe/src/server/cellapp/witness.*`

### 建议跟法

1. 从脚本层属性变更切入。
2. 看 Base / Cell 哪一侧持有这个属性的主状态。
3. 看更新如何进入 Cell 侧同步路径。
4. 看 `Witness::update()` 如何决定发送范围。

### 这一条链的关键认识

- 属性同步不是单独的功能，而是和实体定义、空间位置、视野机制绑在一起的。

### 一个必须一直追问的问题

读这条链时始终问自己：

- 这个属性到底是 Base authoritative，还是 Cell authoritative？

只有先搞清“主状态在哪一侧”，后面的同步链路才解释得通。

## 17.5 实战四：一次写库请求如何经过 DBMgr

### 目标

回答“`writeToDB()` 调用之后，为什么不是直接写数据库，而要经过 Cell 与 DBMgr 的协作”。

### 推荐起点

- `kbe/src/server/baseapp/entity.cpp`
  `Entity::writeToDB`
  `Entity::onCellWriteToDBCompleted`
  `Entity::onWriteToDBCallback`
- `kbe/src/server/baseapp/baseapp.cpp`
  `Baseapp::onCellWriteToDBCompleted`
  `Baseapp::onWriteToDBCallback`
- `kbe/src/server/cellapp/entity.cpp`
  `Entity::writeToDB`
- `kbe/src/server/cellapp/cellapp.cpp`
  `Cellapp::reqWriteToDBFromBaseapp`
- `kbe/src/server/dbmgr/*`
  `Dbmgr::writeEntity`
  `DBTaskWriteEntity::presentMainThread`

### 建议跟法

1. 先看 Base 侧 `writeToDB` 的分支：
   没有 Cell
   已有 Cell
2. 再看为什么已有 Cell 时必须先向 Cell 请求收束状态。
3. 看 `Cellapp::reqWriteToDBFromBaseapp` 如何把请求落到 Cell 实体。
4. 看 Cell 侧 `onWriteToDB()` 与 `backupCellData()` 为什么先发生。
5. 回到 Base，确认 `onPreArchive` 和 `addPersistentsDataToStream()` 在什么时机发生。
6. 最后看 `Dbmgr` 怎样接管真正的数据库操作，以及结果如何再回到原实体。

### 函数级走读路径

建议按下面顺序直接跟：

1. `baseapp/entity.cpp`
   `Entity::writeToDB`
2. `cellapp/cellapp.cpp`
   `Cellapp::reqWriteToDBFromBaseapp`
3. `cellapp/entity.cpp`
   `Entity::writeToDB`
4. `baseapp/baseapp.cpp`
   `Baseapp::onCellWriteToDBCompleted`
5. `baseapp/entity.cpp`
   `Entity::onCellWriteToDBCompleted`
6. `dbmgr/dbmgr.cpp`
   `Dbmgr::writeEntity`
7. `dbmgr/dbtasks.cpp`
   `DBTaskWriteEntity::presentMainThread`
8. `baseapp/baseapp.cpp`
   `Baseapp::onWriteToDBCallback`
9. `baseapp/entity.cpp`
   `Entity::onWriteToDBCallback`

### 这一条链的关键认识

- KBEngine 的持久化不是“谁有数据谁写库”，而是“先收束运行时状态，再集中写库”。

### 这一条链最容易看漏的点

- Cell 不直接做最终持久化，而是先把运行时状态收束好，再把控制权交回 Base。
- 最终执行 Python 回调的是 Base 实体，不是 `Dbmgr`。
- `dbid` 的建立或更新也是在回调收束阶段完成的。

## 17.6 实战五：一个玩家进入 Space 后如何建立视野

### 目标

回答“玩家进入世界后，周围实体是怎样一步步出现在客户端里的”。

### 推荐起点

- `kbe/src/server/baseapp/baseapp.cpp`
  `Baseapp::createCellEntity`
- `kbe/src/server/cellapp/witness.cpp`
  `Witness::onEnterSpace`
  `Witness::installViewTrigger`
  `Witness::onEnterView`
  `Witness::update`

### 建议跟法

1. 从 Base 侧创建 Cell 实体开始。
2. 看 Cell 侧何时为实体建立 `Witness`。
3. 看进入视野时怎样创建 `EntityRef`。
4. 看 `ENTER_CLIENT_PENDING` / `LEAVE_CLIENT_PENDING` 这种状态怎样在 `update()` 中变成真正的客户端消息。

### 建议补看的两条支线

- `kbe/src/server/baseapp/proxy.cpp`
  `Proxy::onGetWitness`
- `kbe/src/server/cellapp/entity.cpp`
  `Entity::onGetWitness`

因为“玩家为什么又突然能看到世界”这个问题，不只发生在首次进世界，还包括：

- 重连
- 挤号
- `giveClientTo`

这些情况最后也都要回到 `onGetWitness` 这条线上收束。

### 这一条链的关键认识

- 客户端“看到世界”并不是一次性结果，而是 `AOI -> Witness -> update bundle` 的持续过程。

## 17.7 推荐的实际阅读顺序

如果只打算真正跟三条链，我建议先看：

1. 实战一：登录到 Baseapp
2. 实战二：EntityCall
3. 实战四：写库到 DBMgr

原因很简单：

- 这三条链分别对应了 KBEngine 最核心的三类问题：
  会话建立
  跨组件调用
  状态持久化

如果准备把“玩家完整一生”真正跟通，我建议改成这个顺序：

1. 登录到 Baseapp
2. 账号实体恢复
3. Base 创建 Cell
4. `onGetWitness` 建立控制权
5. EntityCall
6. 写库到 DBMgr

这个顺序更接近真实运行时，而不是按概念拆开。

## 17.8 本章小结

这章的意义不在于多列几个专题，而在于给出一种阅读源码的方法：

- 先抓起点
- 再抓跨组件消息
- 再抓目标端落地
- 最后抓回调与状态收束

如果能按这个方法把登录、EntityCall、写库、视野同步四条链跟通，KBEngine 的整体骨架基本就立住了。
