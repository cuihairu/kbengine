# Ch24 实战源码走读

> **核心问题**：前面学了机制，这一章带你实际走进源码，跟踪五条完整链路。

## 24.1 统一走读方法

每条走读路径遵循同一方法论：

```
1. 起点：哪个函数、哪个组件
2. 跨组件消息：什么 msgID、什么 Bundle
3. 目标端落地：哪个 handler 处理
4. 回调收束：结果怎么回到发起者
```

**推荐工具**：
- `grep` / `rg`：搜索函数名和消息名
- IDE 跳转：从消息 ID 跳到 handler 注册处
- Watcher：运行时验证关键变量值

---

## 24.2 实战一：一次登录请求怎样进入 BaseApp

### 走读路径

```
Client
  │  login(clientType, datas, loginName, password)
  ↓
Loginapp::login()
  │  kbe/src/server/loginapp/loginapp.cpp:876
  │  1. 参数校验（长度、非空）
  │  2. 检查 baseappmgr / dbmgr 是否就绪
  │  3. 调用脚本 onRequestLogin(loginName, password, clientType, datas)
  ↓
Python 脚本 → 返回 (accountName, password, error, ...)
  │
  ↓ (脚本允许登录)
PendingLoginMgr::add()
  │  kbe/src/lib/server/pendingLoginmgr.h
  │  创建 PLInfos，等待后续流程
  ↓
Loginapp → DBMgr: onAccountLogin
  │  消息：DbmgrInterface::onAccountLogin
  │  内容：loginName, password, datas
  ↓
Dbmgr::onAccountLogin()
  │  kbe/src/server/dbmgr/dbmgr.cpp:708
  │  转发给 interfaces_handler 做账号验证
  ↓
DBMgr → Loginapp: onLoginAccountQueryResultFromDbmgr
  │  返回：retcode, accountName, componentID, entityID, dbid, flags, deadline ...
  ↓
Loginapp → BaseAppMgr: registerPendingAccountToBaseapp / registerPendingAccountToBaseappAddr
  │  componentID > 0 走已有 BaseApp
  │  componentID == 0 走新分配
  ↓
BaseAppMgr → BaseApp: registerPendingLogin
  │  先把待登录信息登记到目标 BaseApp
  ↓
BaseAppMgr → Loginapp: onLoginAccountQueryBaseappAddrFromBaseappmgr
  │  返回：分配的 BaseApp 地址 (ip, tcp_port, udp_port)
  ↓
Loginapp → Client: onLoginSuccessfully(addr, port, ...)
  │  告知客户端 BaseApp 地址
  ↓
Client → BaseApp: loginBaseapp(accountName, password)
  │  直接连接 BaseApp
  ↓
BaseApp::loginBaseapp()
  │  kbe/src/server/baseapp/baseapp.cpp:3769
  │  验证 PendingLogin、来源地址、密码、账号标记
  │  entityID > 0 走“已有 Proxy”分支
  │  entityID == 0 时向 DBMgr 发 queryAccount
  ↓
BaseApp → Client: onCreatedProxies(rndUUID, entityID, entityType)
  │  kbe/src/server/baseapp/baseapp.cpp:3160
  │  Proxy 创建完成，通知客户端
  ↓
Proxy::onClientEnabled()
  │  客户端已就绪，触发脚本层的 onClientEnabled
```

### 关键状态

| 状态 | 存储位置 | 说明 |
|------|---------|------|
| `PendingLoginMgr` | LoginApp 内存 | 等待 DBMgr/BaseAppMgr 回复的登录请求 |
| `pending_logins_` | BaseAppMgr 内存 | LoginApp 与 BaseApp 之间的待分配映射 |
| `EntityLog` | DBMgr 数据库 | 在线实体检出记录 |
| `rndUUID` | Proxy 实体 | 64 位随机 UUID，用于重连 |
| `clientEnabled_` | Proxy 实体 | 客户端是否已就绪 |

### BigWorld 对照

BigWorld 的登录流程类似，但多了：
- **LoginChallenge**：Cuckoo Cycle PoW 验证
- **ClientLoginRequest**：封装登录状态的对象
- **DBAppMgr 分配**：数据库操作由 DBAppMgr 路由到 DBApp

---

## 24.3 实战二：一次 EntityCall 怎样发出去并落地

### 走读路径（KBEngine）

```
Python 脚本层
  │  someEntity.remoteMethod(args)
  ↓
EntityRemoteMethod::tp_call()  /  ClientEntityMethod::tp_call()
  │  kbe/src/server/baseapp/entity_remotemethod.cpp
  │  kbe/src/server/cellapp/client_entity_method.cpp
  │  1. 从 MethodDescription 获取 utype (方法 ID)
  │  2. 创建 Bundle
  ↓
EntityCall::newCall_(msgHandler)
  │  kbe/src/lib/entitydef/entity_call.cpp
  │  1. Bundle::newMessage(msgHandler)
  │  2. 写入 entityID
  │  3. MethodDescription::addToStream(args) 序列化参数
  ↓
EntityCall::sendCall(bundle)
  │  根据 EntityCall 类型路由：
  │  - BaseEntityCall → 通过内部 Channel 发到 BaseApp
  │  - CellEntityCall → 通过内部 Channel 发到 CellApp
  │  - ClientEntityCall → 通过 Proxy 发到客户端
  ↓
网络传输 (TCP)
  │  Bundle → Packet → Channel::send()
  ↓
目标端 MessageHandler
  │  MessageHandlers::handle(msgID, ...)
  │  通过 msgID 查找注册的 handler
  ↓
Base/Cell Entity::onRemoteMethodCall(channel, stream)
  │  kbe/src/server/baseapp/entity.cpp
  │  kbe/src/server/cellapp/entity.cpp
  │  1. 从 stream 读取 entityID → 查找实体
  │  2. 从 stream 读取 methodUtype → 查找 MethodDescription
  │  3. MethodDescription::createFromStream(stream) 反序列化参数
  │  4. 调用 Python 方法：PyObject_CallMethod(entity, methodName, args)
```

### 方法 ID 的来源

```
MethodDescription 在 EntityDef 初始化时分配 utype：
  → ScriptDefModule::addMethodDescription()
  → 每个 .def 文件中定义的方法按顺序编号
  → utype 在同一实体的所有方法中唯一
  → 通过 utype 标识"调用哪个方法"，而非方法名字符串
```

### Ghost 上的特殊路径

```
如果目标实体是 ghost（非 real）：
  ↓
RealEntityMethod::__py_call()
  │  不直接执行，而是转发给 real entity
  │  消息路由：ghost 的 CellApp → real 的 CellApp
  ↓
Real entity 端执行
  │  Entity::onRemoteMethodCall()
  │  在 real 上执行后，结果可能通过 ghost 同步回 ghost 所在的客户端
```

---

## 24.4 实战三：一个属性更新如何同步给客户端

### 走读路径（KBEngine）

```
Python 脚本层
  │  entity.someProp = newValue
  ↓
Entity::__py_setattr(name, value)
  │  kbe/src/server/baseapp/entity.cpp / kbe/src/server/cellapp/entity.cpp
  │  1. 查找 PropertyDescription
  │  2. DataType::isSameType 做类型检查
  │  3. PropertyDescription::onSetValue 写入脚本对象
  ↓
Entity::onDefDataChanged(...)
  │  kbe/src/server/cellapp/entity.cpp
  │  1. real entity 才继续
  │  2. 先同步给 ghost
  │  3. 再遍历当前 witnesses_ 推送给可见客户端
  │  4. 最后同步给实体自己的客户端
  ↓
Bundle 发送
  │  通过 Proxy / client mailbox 发到客户端
  ↓
Client: onUpdatePropertys / onUpdateData_xz_ypr_...
  │  客户端 SDK 接收
  │  1. 查找对应实体
  │  2. 更新属性值
  │  3. 如果是位置/朝向 → 做插值平滑
```

### alias 机制的关键步骤

```
实体首次进入客户端视野时：
  → onEnterView: 在 Witness 侧为 EntityRef 分配 alias
  → alias 本质上是当前视野列表里的短编号
  → 后续同步用 alias 代替完整 entityID
  → 当前实现里发送给客户端时要求 aliasID <= 255
```

### BigWorld 对照

BigWorld 在 Witness update 中多了 **aoi_update_schemes** 策略层：

```
aoi_update_scheme（可插拔策略）
  → 决定哪些属性同步给哪些 Witness
  → 支持按距离、按类型、按优先级等不同策略
  → KBEngine 没有 this 抽象，策略硬编码
```

---

## 24.5 实战四：一次写库请求如何经过 DBMgr

### 走读路径（KBEngine）

```
Python 脚本层
  │  entity.writeToDB(callback, shouldAutoLoad)
  ↓
Entity::writeToDB()
  │  kbe/src/server/baseapp/entity.cpp
  │  1. 收集所有 persistent 属性
  │  2. addPersistentsDataToStream() 序列化到 MemoryStream
  ↓
Base → Cell：同步 Cell 侧数据
  │  Cell 上也有 persistent 属性（如位置）
  │  Cell 整理后通过 EntityCall 回传给 Base
  ↓
Base 整理完整数据
  │  Base 侧 persistent 属性 + Cell 侧 persistent 属性
  ↓
Base → DBMgr: writeEntity
  │  消息：DbmgrInterface::writeEntity
  │  内容：实体数据流 + entityDBID + callbackID + shouldAutoLoad
  ↓
Dbmgr::writeEntity()
  │  kbe/src/server/dbmgr/dbmgr.cpp
  │  创建 DBTaskWriteEntity 投递到线程池
  ↓
DBTaskWriteEntity::db_thread_process()
  │  kbe/src/server/dbmgr/dbtasks.cpp:328
  │  在工作线程中执行 SQL：
  │  1. 如果 entityDBID == 0 → INSERT（新实体）
  │  2. 否则 → UPDATE（更新已有实体）
  │  3. 同时更新 EntityLog（在线检出记录）
  ↓
DBTaskWriteEntity::presentMainThread()
  │  回到主线程，将结果发送回 BaseApp
  ↓
BaseApp 收到写库结果
  │  Entity::onWriteToDBCallback()
  │  再触发 CallbackMgr 的回调
```

### 三段式写库

```
Base 侧持久化数据  ──┐
                      ├──→  合并  ──→  DBMgr ──→  MySQL
Cell 侧持久化数据  ──┘
```

- Base 先把自己的 persistent 属性序列化
- 向 Cell 请求 Cell 侧的 persistent 属性
- 两部分数据合并后发送给 DBMgr
- DBMgr 在工作线程执行 SQL，完成后回调 BaseApp

### BigWorld 对照

BigWorld 的写库路径类似，但有**归档 vs 直写**的区别：

```
KBEngine: writeToDB → DBMgr → MySQL（即时写）
BigWorld:
  Archive (定期归档) → DBApp → MySQL
  Backup (跨进程备份) → 另一个 BaseApp（内存）
  writeToDB → DBAppMgr → DBApp → MySQL（即时写）
```

---

## 24.6 实战五：一个玩家进入 Space 后如何建立视野

### 走读路径（KBEngine）

```
Python 脚本层
  │  entity.teleport(spaceID, position, direction)
  ↓
Base → CellAppMgr: 请求创建 Cell 实体
  │  消息包含：entityID, spaceID, position, direction
  ↓
Base / Cell 路径落到目标 CellApp: 创建实体
  │  是落到已有 Space 的 Cell，还是先为 Space 分配 Cell，
  │  取决于当前 Space 运行态，而不是“每次 teleport 都重新负载均衡”
  ↓
CellApp::onCreateEntityFromCellappmgr()
  │  在目标 CellApp 上创建 Cell 实体
  │  实体被插入到 Space 的 CoordinateSystem
  ↓
CoordinateSystem::insert(entity)
  │  插入十字链表的 X 轴和 Z 轴
  ↓
RangeTrigger 检测
  │  新实体的 ViewTrigger 激活
  │  搜索视野范围内的所有实体
  ↓
AOI 事件触发
  │  onEnterView / onEnterAoI
  │  创建 Witness（如果是玩家实体）
  ↓
Witness 建立
  │  1. 为玩家实体创建 Witness
  │  2. 设置 ViewRadius（视野半径）
  │  3. 扫描视野内所有实体
  ↓
客户端同步
  │  对视野内每个实体：
  │  → onEntityEnterWorld(entityID, entityType, ...)
  │  → onUpdatePropertys(所有初始属性)
  │  → onUpdateBasePos / onUpdateBaseDir (位置朝向)
  ↓
tick 开始后
  │  实体移动 → AOI 更新 → Witness update → 客户端同步
```

### BigWorld 对照

BigWorld 的路径类似，但多了 BSP 树动态分配：

```
1. Base 请求 CellAppMgr 创建 Cell 实体
2. CellAppMgr 通过 BSP 树确定实体应落在哪个 CellApp
3. 如果该 Space 尚未分配 CellApp → CellAppMgr 分配
4. BSP 树的 InternalNode 可能需要 grow/shrink
5. 实体创建后插入 Cell 的 CoordinateSystem
6. Witness 建立 → AOI 事件 → 客户端同步
```

---

## 24.7 推荐的实际阅读顺序

### 优先级排序

```
第一条链：登录（理解系统全貌）
  → Ch22 + 本实战一
  → 涉及组件最多：LoginApp → DBMgr → BaseAppMgr → BaseApp → Client
  → 理解后对系统整体有清晰认识

第二条链：EntityCall（理解 RPC 机制）
  → Ch11 + 本实战二
  → 涉及核心机制：序列化 → 消息路由 → handler 查找 → Python 调用

第三条链：属性同步（理解数据流）
  → Ch12 + 本实战三
  → 涉及性能关键路径：脏标记 → Witness → Bundle 复用 → 客户端

第四条链：写库（理解持久化）
  → Ch13 + 本实战四
  → 涉及数据安全：Base/Cell 协作 → 线程池 → SQL → 回调

第五条链：Space + 视野（理解空间系统）
  → Ch14 + 本实战五
  → 涉及最复杂的子系统：AOI → Witness → 客户端
```

### 阅读方法

1. **第一遍**：读走读路径，理解整体流程，不深入细节
2. **第二遍**：在 IDE 中实际跳转代码，在每个关键函数打断点
3. **第三遍**：启动集群，用 Bots 触发流程，观察 Watcher 和日志输出
4. **第四遍**：修改代码加 debug 日志，验证你的理解是否正确

---

## 24.8 小结

五条实战走读路径覆盖了 MMO 服务器最核心的五条链路：

| 链路 | 起点 | 终点 | 涉及组件数 | 核心机制 |
|------|------|------|-----------|---------|
| 登录 | Client | BaseApp | 4+ | 会话建立、实体恢复 |
| EntityCall | Python 脚本 | 目标端 Entity | 2-3 | 序列化、消息路由 |
| 属性同步 | setattr | Client SDK | 2-3 | AOI、Witness、Bundle |
| 写库 | Python 脚本 | DBMgr | 3 | Base/Cell 协作、线程池 |
| Space 视野 | teleport | Client | 3+ | AOI、CoordinateSystem、Witness |

掌握这五条链路，就掌握了整个引擎的核心运行机制。其余所有功能（战斗、AI、社交、经济）都是在这五条链路上组装出来的。
