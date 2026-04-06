# Ch22 玩家完整生命周期

> **核心问题**：一个玩家从点击"登录"到真正看到游戏世界，中间到底发生了什么？下线、断线、重连时，系统又如何收束？

这一章把前面分散在登录、实体创建、空间、AOI、持久化各章的机制收束成一条完整主线。不是再讲一遍"登录流程"，而是从系统设计角度，把玩家生命周期的每一步还原成组件间协作的精确序列。

## 相关 API 回查

- 登录与会话入口：[KBEngine(loginapp)](/api/loginapp/KBEngine.md)、[KBEngine(baseapp)](/api/baseapp/KBEngine.md)、[Proxy(baseapp)](/api/baseapp/Proxy.md)
- 实体侧接口：[Entity(baseapp)](/api/baseapp/Entity.md)、[Entity(cellapp)](/api/cellapp/Entity.md)
- 客户端视角：[KBEngine(client)](/api/client/KBEngine.md)、[Entity(client)](/api/client/Entity.md)

## 22.1 七阶段主流程总览

一个玩家从客户端发起登录到真正"看到世界"，依次经过七个阶段：

| 阶段 | 触发动作 | 参与组件 | 完成标志 |
|------|---------|---------|---------|
| 1. 接入 | 客户端发送账号密码 | LoginApp | LoginApp 收到登录请求 |
| 2. 状态查询 | 查询账号/实体的在线状态 | LoginApp -> DBMgr | DBMgr 返回账号状态 |
| 3. 分配 | 为会话选择承载进程 | LoginApp -> BaseAppMgr | BaseAppMgr 返回目标 BaseApp 地址 |
| 4. 会话建立 | 客户端连接目标 BaseApp | Client -> BaseApp | BaseApp 找到或创建 Proxy |
| 5. 实体恢复 | 从数据库恢复 Base 实体 | BaseApp -> DBMgr | BaseApp 拿到 Proxy 并初始化 |
| 6. Cell 创建 | 在空间中创建 Cell 实体 | BaseApp -> CellApp | CellApp 上实体创建完成 |
| 7. 视野建立 | Witness 驱动客户端同步 | CellApp -> Client | 客户端开始收到视野内实体流 |

如果只看这一层，设计特点很清楚：

- **接入和会话是分开的**：LoginApp 不持有玩家实体，BaseApp 才是会话锚点。
- **查询和管理是分开的**：DBMgr 管状态，BaseAppMgr 管分配。
- **逻辑和空间是分开的**：Base 管非空间逻辑，Cell 管世界状态。
- **实体和视野是分开的**：Cell 实体存在不等于客户端能看到世界。

下面逐步展开每个阶段，同时对照 KBEngine 和 BigWorld 两套实现的差异。

---

## 22.2 阶段一：LoginApp 接入

### 22.2.1 KBEngine 的实现

客户端的第一站是 LoginApp。LoginApp 的职责非常明确：接收请求、转发查询、回传地址。它不创建实体，不持有会话，不做业务判断。

```cpp
// kbe/src/server/loginapp/loginapp.cpp

void Loginapp::onLoginAccountQueryResultFromDbmgr(
    Network::Channel* pChannel, MemoryStream& s)
```

这个 handler 实际上是从 `MemoryStream` 里依次解出：

- `retcode`
- `loginName / accountName / password / needCheckPassword`
- `componentID / entityID / dbid / flags / deadline`
- `datas`

其中 `componentID > 0` 表示该账号当前仍挂在某个 BaseApp 上；这会把后续分支切到 `registerPendingAccountToBaseappAddr`，而不是重新分配一个新的 BaseApp。

### 22.2.2 BigWorld 的实现

BigWorld 的 LoginApp 做了更多事情：

```cpp
// BigWorld: server/loginapp/client_login_request.cpp

ClientLoginRequest::ClientLoginRequest() :
    creationTime_( 0 ),
    pParams_( NULL ),
    pChannel_( NULL ),
    challengeType_(),
    didFailChallenge_( false ),
    pLoginChallenge_( NULL ),
    replyRecord_()
{}
```

BigWorld 在 LoginApp 层引入了 **Login Challenge** 机制（`LoginChallenge`）：在真正查数据库之前，先给客户端发一个计算挑战（如 Cuckoo Cycle Proof-of-Work），防止暴力登录。KBEngine 没有这层防护。

BigWorld 的 `DatabaseReplyHandler` 处理 DBAppMgr 返回的结果：

```cpp
// BigWorld: server/loginapp/database_reply_handler.cpp

void DatabaseReplyHandler::handleMessage(...)
{
    uint8 status;
    data >> status;

    if (status != LogOnStatus::LOGGED_ON)
    {
        // 处理各种失败：IP封禁、密码错误、过载等
        // ...
        return;
    }

    LoginReplyRecord lrr;
    data >> lrr;
    // lrr 包含 BaseApp 地址、会话密钥等
}
```

### 22.2.3 关键差异

| 维度 | KBEngine | BigWorld |
|------|---------|---------|
| 认证挑战 | 无 | LoginChallenge（Cuckoo Cycle） |
| 数据库查询 | DBMgr 统一处理 | DBApp -> DBAppMgr 两级 |
| 过载保护 | 基本拒绝 | 区分 BaseApp/CellApp/DBApp 过载 |
| IP 封禁 | 无 | LoginApp 层 IP Ban + 超时 |

---

## 22.3 阶段二：DBMgr 状态查询

### 22.3.1 为什么不直接查数据库

LoginApp 不直接查数据库，而是通过 DBMgr。因为 DBMgr 掌握的不是数据库数据，而是**在线状态**。

LoginApp 这一跳真正依赖的不是整张实体持久化表，而是“账号状态 + 在线检出信息”：

- `dbid`：账号或实体对应的数据库 ID
- `componentID`：当前挂载的 BaseApp 组件 ID
- `entityID`：在线实体 ID
- `flags / deadline`：账号级别的锁定、激活、过期信息

如果 `componentID > 0`，说明这个账号当前挂在一个活着的 BaseApp 上——登录不是"查库创建新实体"，而是"判断在线上下文是否存在并决定如何处理"。

### 22.3.2 BigWorld 的 checkout 机制

BigWorld 的 DBApp 把这个过程封装得更精细：

```cpp
// BigWorld: server/dbapp/login_handler.cpp

void LoginHandler::checkOutEntity()
{
    if ((pBaseRef_ == NULL) &&
        DBApp::instance().onStartEntityCheckout( entityKey_ ))
    {
        // 未检出 → 预留 BaseMailbox → 分配 BaseApp
        DBApp::setBaseRefToLoggingOn( baseRef_, entityKey_.typeID );
        DBApp::instance().setBaseEntityLocation( entityKey_, baseRef_,
                reserveBaseMailboxHandler_ );
    }
    else
    {
        // 已检出 → 已在线，走"已登录用户"逻辑
        DBApp::instance().onLogOnLoggedOnUser( entityKey_.typeID,
            entityKey_.dbID, pParams_, clientAddr_, replyAddr_, replyID_,
            pBaseRef_, dataForClient_, dataForBaseEntity_ );
    }
}
```

BigWorld 区分了"实体未检出（离线）"和"实体已检出（在线）"两种情况，对已在线的用户有专门的处理路径。KBEngine 通过 `componentID` 做类似判断，但逻辑更集中在 LoginApp 和 BaseApp 侧。

---

## 22.4 阶段三：BaseAppMgr 分配

当 LoginApp 拿到 DBMgr 的结果后，下一步是通过 BaseAppMgr 获取目标 BaseApp 地址。

### 22.4.1 KBEngine 的两路分支

```cpp
// kbe/src/server/loginapp/loginapp.cpp

// 分支一：componentID > 0，账号已挂在某 BaseApp
//   → registerPendingAccountToBaseappAddr
//   → 直接定位到已有 BaseApp

// 分支二：componentID == 0，没有在线上下文
//   → registerPendingAccountToBaseapp
//   → BaseAppMgr 分配一个新 BaseApp
```

之后 LoginApp 回给客户端的不是"你已经登录完成"，而是"去连这个 BaseApp"。这就是**两跳接入**的设计：

```
Client → LoginApp → DBMgr → BaseAppMgr → Client
                                                  ↓
Client ← LoginApp（BaseApp 地址）                  ↓
Client → BaseApp（loginBaseapp）─────────────────→←
```

### 22.4.2 BigWorld 的 PendingLogins

BigWorld 的 BaseApp 有 `PendingLogins` 系统：

```cpp
// BigWorld: server/baseapp/pending_logins.cpp

SessionKey PendingLogins::add( Proxy * pProxy,
        const Mercury::Address & loginAppAddr )
{
    SessionKey loginKey = pProxy->sessionKey();
    pProxy->regenerateSessionKey();

    // 确保同一个 Proxy 不在 pending 列表中出现两次
    for (iterator iter = container_.begin(); iter != container_.end(); ++iter)
    {
        if (iter->second.pProxy() == pProxy)
        {
            container_.erase( iter );
            break;
        }
    }

    container_.insert( Container::value_type( loginKey,
        PendingLogin( pProxy, loginAppAddr ) ) );

    // 30 秒超时
    const int PENDING_LOGINS_TIMEOUT = 30;
    queue_.push_back( QueueElement(
            BaseApp::instance().time() +
                PENDING_LOGINS_TIMEOUT * BaseAppConfig::updateHertz(),
            pProxy->id(), loginKey ) );

    return loginKey;
}
```

BigWorld 用 `SessionKey` 做登录凭证（KBEngine 用 `rndUUID`），并且有明确的 30 秒超时——如果客户端在 30 秒内没有连到 BaseApp，`PendingLogins::tick()` 会触发 `Proxy::onClientDeath`。

---

## 22.5 阶段四：BaseApp 会话建立

### 22.5.1 KBEngine：loginBaseapp 不是盲目接收

```cpp
// kbe/src/server/baseapp/baseapp.cpp

// BaseApp::loginBaseapp 检查：
// 1. 账号名长度与密码长度
// 2. DBMgr 是否就绪
// 3. PendingLoginMgr 中是否存在对应待登录记录
// 4. 请求来源地址是否与待登录记录一致
// 5. 密码是否匹配
// 6. 账号标记是否允许登录
```

BaseApp 只接管经过 LoginApp 预分配和登记的会话。这防止了客户端绕过 LoginApp 直接伪造登录。

如果 `ptinfos->entityID > 0`（已在线），BaseApp 会调用 `Proxy::onLogOnAttempt()`，把"是否允许新客户端挤掉旧客户端"交给脚本层决定。当脚本返回 `LOG_ON_ACCEPT` 时，底层动作是：

1. 若旧 `clientEntityCall` 仍在，踢掉旧客户端通道
2. 把 Proxy 重新绑定到新客户端地址
3. 重新执行 `createClientProxies()`
4. 调用 `Proxy::onGetWitness()`，把客户端控制权恢复推到 Cell 侧

### 22.5.2 BigWorld：LoginHandler + attachToClient

BigWorld 的对应流程更分散，但有明确的超时和统计：

```cpp
// BigWorld: server/baseapp/login_handler.cpp

void LoginHandler::login( ... )
{
    PendingLogins::iterator pendingIter = pPendingLogins_->find( args.key );

    if (pendingIter == pPendingLogins_->end())
    {
        // 没有对应的 pending login → 拒绝
        return;
    }

    const PendingLogin & pending = pendingIter->second;
    SmartPointer<Proxy> pProxy = pending.pProxy();

    // 更新统计（NAT 检测、重试次数）
    this->updateStatistics( srcAddr, pending.addrFromLoginApp(), args.numAttempts );

    pPendingLogins_->erase( pendingIter );

    // 关键：attachToClient
    if (pProxy->attachToClient( srcAddr, header.replyID, header.pChannel.get() ))
    {
        // 成功绑定
    }
}
```

BigWorld 的 `attachToClient` 比 KBEngine 的 `loginBaseapp` 多做了 NAT 检测和多次重试统计——这对于真实的公网部署很重要。

---

## 22.6 阶段五：Base 实体恢复

如果账号还没有在线实体，BaseApp 会向 DBMgr 发送 `queryAccount`，然后在 `onQueryAccountCBFromDbmgr()` 里创建或恢复 Proxy。

### 22.6.1 KBEngine 的恢复链路

```
BaseApp::loginBaseapp
  → DBMgrInterface::queryAccount
  → BaseApp::onQueryAccountCBFromDbmgr
  → createEntity(...)
  → initializeEntity(pyDict)
  → createClientProxies(...)
```

`onQueryAccountCBFromDbmgr` 是关键落点：

1. `createEntity()` 创建账号实体类型对应的 Proxy
2. 安装 `dbid`、客户端类型、登录附加数据和 `createDatas`
3. `createDictDataFromPersistentStream()` 从持久化流恢复脚本属性
4. 注入 `__ACCOUNT_NAME__` 与 `__ACCOUNT_PASSWORD__`
5. `initializeEntity(pyDict)` 完成脚本对象初始化
6. 若客户端连接还在，构造 `clientEntityCall` 并执行 `createClientProxies()`

### 22.6.2 BigWorld 的恢复链路

BigWorld 的恢复发生在 `Base::restoreTo()` 中，它从 backup 数据流中恢复整个 Base 实体：

```cpp
// BigWorld: server/baseapp/base.cpp

// Base::restoreTo 从备份流中恢复：
// 1. cellAddr → 设置 Cell 通道
// 2. hasChannel → 是否有客户端连接
// 3. isCreateCellPending / isGetCellPending / isDestroyCellPending
// 4. spaceID / shouldAutoBackup / shouldAutoArchive
// 5. cellBackupData_ → Cell 备份数据
// 6. Proxy 的 readBackupData → 恢复客户端相关状态
// 7. restoreTimers → 恢复定时器
// 8. restoreAttributes → 恢复属性
// 9. restoreCellData → 恢复 Cell 数据
```

恢复完成后调用脚本钩子：

```cpp
// 如果有客户端连接 → 调用 onOnload
// 如果没有客户端连接 → 调用 onRestore
// 两种情况最后都调用 Proxy::onRestored()
```

BigWorld 的恢复粒度更细——它不是从数据库恢复，而是从**备份流**恢复（由 BackupSender 周期性跨 BaseApp 备份）。这意味着恢复速度更快（不需要查数据库），但需要额外的备份基础设施。

---

## 22.7 阶段六：Cell 实体创建

### 22.7.1 从 Base 到 Cell 的数据交接

玩家有了 Base 实体不代表已经进入世界。进入空间发生在 Base 侧触发 Cell 实体创建之后。

KBEngine 的关键入口：

```
Entity::createCellEntity
Entity::createCellEntityInNewSpace
Entity::restoreCell
```

`BaseApp::createCellEntity` 的核心动作：

```cpp
// 构造 CellappInterface::onCreateCellEntityFromBaseapp 消息
// 包含：
//   - entityType
//   - entity id
//   - base componentID
//   - addCellDataToStream() 序列化的 Cell 初始数据
```

这说明了 Base/Cell 分离的本质——不是概念分离，而是真有一次跨组件的数据交接。Base 把 Cell 所需的初始状态序列化后交给 CellApp。

### 22.7.2 进入世界的三小步

把"进入世界"拆开来看：

1. **Base 上先存在一个已与客户端绑定的 Proxy**
2. **`Entity::createCellEntity` 把 Cell 初始状态序列化后交给目标 CellApp**
3. **Cell 创建完成后，通过 `Proxy::onGetWitness()` 把"客户端控制权"绑定到 Cell 侧**

所以进入世界不是一次构造动作，而是三步接力。

### 22.7.3 BigWorld 的 restoreTo + createEntity

BigWorld 在恢复场景下走 `Base::restoreTo()`，它直接发送 `restoreEntity` 消息给 CellApp：

```cpp
// BigWorld: server/baseapp/base.cpp

// restoreTo → 发送 restoreEntity 到 CellApp
bundle.startMessage( *pToCall );  // createEntity 或 restoreEntity
bundle << spaceID;
bundle << pChannel_->version();
bundle << true /*isRestore*/;
bundle.addBlob( cellBackupData_.data(), cellBackupData_.length() - footerSize );
```

BigWorld 区分了 `createEntity`（全新创建）和 `restoreEntity`（从备份恢复）两种消息，后者附带备份数据，CellApp 可以直接从中恢复 Cell 实体。

---

## 22.8 阶段七：Witness 建立

### 22.8.1 KBEngine 的 Witness 建立

```cpp
// kbe/src/server/baseapp/proxy.cpp

void Proxy::onGetWitness()
{
    if(cellEntityCall())
    {
        // 通知 CellApp 获得客户端
        Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
        (*pBundle).newMessage(CellappInterface::onGetWitnessFromBase);
        (*pBundle) << this->id();
        sendToCellapp(pBundle);
    }
}
```

在 CellApp 侧，`Entity::onGetWitness(bool fromBase)` 被调用时：

- 如果 `fromBase == true`，通过 `controlledBy(baseEntityCall())` 把客户端控制关系重新绑回当前 Cell 实体
- 主动把 `spaceID` 和客户端属性打包发给客户端
- 如果还没有 Witness 就创建；如果已有，执行 `onAttach()` + `resetViewEntities()` 重建视野

这意味着"重连恢复"不是简单补一个 socket，而是：

1. 重新绑定客户端控制对象
2. 重新同步关键客户端属性
3. 重新建立视野状态

### 22.8.2 BigWorld 的 Witness 构建

BigWorld 的 Witness 构造更复杂，包含 alias 预分配和 AOI 初始化：

```cpp
// BigWorld: server/cellapp/witness.cpp

Witness::Witness( RealEntity & owner, BinaryIStream & data,
        CreateRealInfo createRealInfo, bool hasChangedSpace ) :
    real_( owner ),
    entity_( owner.entity() ),
    aoiHyst_( 5.0 ),
    aoiRadius_( CellAppConfig::defaultAoIRadius() ),
    pAoIRoot_( entity_.pRangeListNode() ),
    bandwidthDeficit_( 0 ),
    numFreeAliases_( 0 )
{
    ++g_numWitnesses;
    ++g_numWitnessesEver;

    // 初始化 alias 池
    memset( freeAliases_, 1, sizeof( freeAliases_ ) );
    // ... alias 分配逻辑
}
```

BigWorld 的 Witness 内部维护了 `entityQueue_`（优先级队列）和 `aoiMap_`（AOI 实体映射），这些在 KBEngine 中由更简单的 `viewEntities_` 替代。

### 22.8.3 从 AOI 事件到客户端消息

```
空间索引判断实体进入 AOI
  → Witness::addToAoI()
  → 标记 ENTER_PENDING
  → Witness::update() 在 tick 末统一执行
  → 构造客户端消息（实体创建 + 初始属性）
  → 发送给客户端
```

`Witness::update()` 是整条链的收束点——它把进入/离开/属性变化 pending 统一处理，避免在单次 tick 内频繁发送消息。

---

## 22.9 三条主线

把七个阶段压缩成三条数据流主线：

### 会话主线：LoginApp -> BaseApp

```
Client ──login──→ LoginApp
                LoginApp ──DbmgrInterface::onAccountLogin──→ DBMgr
                LoginApp ←──onLoginAccountQueryResultFromDbmgr── DBMgr
                LoginApp ──registerPendingAccountToBaseapp[Addr]──→ BaseAppMgr
                BaseAppMgr ──registerPendingLogin──→ BaseApp
                LoginApp ←──onLoginAccountQueryBaseappAddrFromBaseappmgr── BaseAppMgr
Client ←──onLoginSuccessfully(baseapp addr)── LoginApp
Client ──loginBaseapp──→ BaseApp
                BaseApp ──DbmgrInterface::queryAccount──→ DBMgr
                BaseApp ←──onQueryAccountCBFromDbmgr── DBMgr
                BaseApp 创建/恢复 Proxy
```

### 世界主线：Base -> Cell -> Witness -> Client

```
Base.Proxy ──createCellEntity──→ CellApp
CellApp 创建 Cell 实体
Base.Proxy ──onGetWitness──→ CellApp
CellApp 创建 Witness
Witness ──AOI event──→ 触发 addToAoI
Witness ──update()──→ 构造客户端消息
Client ←──实体流── CellApp
```

### 数据主线：Cell -> Base -> DBMgr

```
Cell 实体状态变更
  → Base 写库时先向 Cell 请求状态收束
  → Cell 执行 onWriteToDB + backupCellData
  → Base 收到 Cell 数据后执行 onPreArchive
  → Base 调用 addPersistentsDataToStream 序列化
  → 发送 DbgmrInterface::writeEntity
  → DBMgr 执行 DBTaskWriteEntity / EntityDBTask
  → 回调 Entity::onWriteToDBCallback
```

---

## 22.10 下线、重连与恢复

### 22.10.1 普通下线

KBEngine 的 `BaseApp::logoutBaseapp`：

1. 找到目标 Proxy
2. 校验 `rndUUID`
3. 把客户端通道 condemn
4. 连接关系收束，不一定意味着实体立即销毁

BigWorld 的 `Proxy::onClientDeath`：

```cpp
// BigWorld: server/baseapp/proxy.cpp

void Proxy::onClientDeath( ClientDisconnectReason reason,
        bool shouldExpectClient /* = true */ )
{
    // 1. 如果已经 dead 了就忽略
    if (!this->hasClient() && shouldExpectClient)
        return;

    // 2. 踢掉客户端通道
    if (shouldExpectClient || this->hasClient())
        this->logOffClient( shouldCondemnClient );

    // 3. 调用 Python 脚本 onClientDeath
    PyObject * pFunc = PyObject_GetAttrString( this, "onClientDeath" );
    // ... 执行脚本回调
}
```

BigWorld 的 `onClientDeath` 有精细的断线原因分类（`CLIENT_DISCONNECT_TIMEOUT`、`CLIENT_DISCONNECT_RATE_LIMITS_EXCEEDED`、`CLIENT_DISCONNECT_SHUTDOWN` 等），KBEngine 的分类更粗。

### 22.10.2 重连

KBEngine 的 `BaseApp::reloginBaseapp`：

1. 校验 `entityID` 与 `rndUUID`
2. 若旧 `clientEntityCall` 还在，踢掉旧通道
3. 把客户端地址改成新地址
4. 重新执行 `createClientProxies(proxy, true)`
5. 通过 `proxy->onGetWitness()` 通知 Cell 侧恢复控制权
6. 回给客户端 `onReloginBaseappSuccessfully`

重连成功包含两层恢复：

- **Base 层**：恢复会话（Proxy 绑定新客户端）
- **Cell 层**：恢复世界表现（Witness 重建视野）

### 22.10.3 控制权转移

`Proxy::giveClientTo` 更进一步——"客户端控制哪个 Proxy"本身可以切换。

KBEngine 实现：

```cpp
// kbe/src/server/baseapp/proxy.cpp

void Proxy::giveClientTo(Proxy* proxy)
{
    // 校验：自身不能 destroyed，必须有 clientEntityCall
    // 校验：目标不能 destroyed，不能是自身，不能已有 clientEntityCall

    Network::Channel* lpChannel = clientEntityCall_->getChannel();
    // ... 把客户端通道转移给新 Proxy
}
```

这条链里会发生：

1. 旧 Proxy 如有 Cell，向 Cell 发送 `onLoseWitness`
2. 客户端收到 `onEntityDestroyed`，删除旧控制实体
3. 新 Proxy 创建自己的 `clientEntityCall`
4. `BaseApp::createClientProxies()` 把新控制实体同步给客户端
5. `Proxy::onGetWitness()` 再次驱动 Cell 侧建立 Witness/视野

### 22.10.4 恢复主线

```
Proxy（Base 层逻辑实体）
  ↕ clientEntityCall（客户端连接引用）
  ↕ Cell 实体（空间状态）
  ↕ Witness（视野同步通道）
```

这四个对象彼此关联但不是同一个对象。正因为拆开了，系统才有能力支持重连、挤号和控制权迁移。

---

## 22.11 属性/方法/持久化三类数据的流向

### 属性更新

```
脚本 setattr → 标记脏 → Cell 确定 authoritative 侧
  → Witness 收集 → Bundle 构造 → 发送给客户端
```

"属性更新"不是简单广播，而是被实体位置和视野约束。

### 远程方法调用

```
脚本发起 EntityCall
  → 根据实体定义查找方法与参数信息
  → 参数序列化进入 Bundle/MemoryStream
  → 网络层发送到目标组件
  → 目标端解包并调用
```

EntityCall 在玩家生命周期里扮演"跨 Base/Cell/Client 协作的动作通道"。

### 持久化

```
Base::writeToDB
  → Cell::reqWriteToDBFromBaseapp（如果 Cell 存在）
  → Cell::writeToDB + backupCellData
  → Base::onCellWriteToDBCompleted
  → Base::onPreArchive + addPersistentsDataToStream
  → DBMgr::writeEntity
  → DBTaskWriteEntity::presentMainThread
  → Base::onWriteToDBCallback
```

这完全不是"本地对象直接写数据库"，而是一条跨 Base/Cell/DBMgr 的状态收束流水线。

---

## 22.12 KBEngine 与 BigWorld 对照

| 维度 | KBEngine | BigWorld |
|------|---------|---------|
| **接入层安全** | 无登录挑战 | LoginChallenge（Cuckoo Cycle PoW） |
| **会话凭证** | `rndUUID`（64位随机数） | `SessionKey`（每次重新生成） |
| **数据库层** | 单一 DBMgr | DBApp + DBAppMgr 两级 |
| **在线状态** | KBEEntityLogTable（DBMgr 维护） | bigworldLogOns（DBApp 维护） |
| **PendingLogin** | PendingLoginMgr（简单映射） | PendingLogins（超时队列 + NAT 检测） |
| **实体恢复** | 从数据库恢复（`createEntityFromDBID`） | 从备份流恢复（`restoreTo`） |
| **恢复速度** | 需查数据库，较慢 | 从备份恢复，较快 |
| **Cell 创建** | `createCellEntity` | `createEntity` / `restoreEntity`（区分新创建和恢复） |
| **Witness 初始化** | 简单 viewEntities_ | 复杂（alias 池 + 优先级队列 + AOI 映射） |
| **断线原因** | 粗粒度 | 精细分类（TIMEOUT/RATE_LIMIT/SHUTDOWN/...） |
| **重连恢复** | 重新绑定 Proxy + 重建 Witness | 类似 + onRestored 钩子 |
| **备份基础设施** | 无跨进程备份 | BackupSender 跨 BaseApp 备份 |

---

## 22.13 源码入口表

| 源码模块 | 文件路径 | 关键类/函数 |
|---------|---------|------------|
| **阶段 1-2：登录接入** | | |
| KBEngine LoginApp | `kbe/src/server/loginapp/loginapp.cpp` | `onLoginAccountQueryResultFromDbmgr` |
| KBEngine LoginApp 接口 | `kbe/src/server/loginapp/loginapp_interface.h` | LoginAppInterface 消息定义 |
| BigWorld LoginApp | `BigWorld-Engine-14.4.1/server/loginapp/loginapp.cpp` | LoginApp 主循环 |
| BigWorld 登录请求 | `BigWorld-Engine-14.4.1/server/loginapp/client_login_request.cpp` | `ClientLoginRequest` |
| BigWorld 数据库回复 | `BigWorld-Engine-14.4.1/server/loginapp/database_reply_handler.cpp` | `DatabaseReplyHandler::handleMessage` |
| **阶段 2-3：状态查询** | | |
| KBEngine DBMgr | `kbe/src/server/dbmgr/dbtasks.cpp` | 登录相关 Task |
| BigWorld DBApp Login | `BigWorld-Engine-14.4.1/server/dbapp/login_handler.cpp` | `checkOutEntity` |
| **阶段 3-4：分配与会话** | | |
| KBEngine BaseAppMgr | `kbe/src/server/baseappmgr/baseappmgr.cpp` | 分配 BaseApp |
| BigWorld PendingLogins | `BigWorld-Engine-14.4.1/server/baseapp/pending_logins.cpp` | `PendingLogins::add` / `tick` |
| BigWorld LoginHandler | `BigWorld-Engine-14.4.1/server/baseapp/login_handler.cpp` | `LoginHandler::login` |
| **阶段 4-5：会话建立与实体恢复** | | |
| KBEngine BaseApp | `kbe/src/server/baseapp/baseapp.cpp` | `loginBaseapp` / `reloginBaseapp` / `logoutBaseapp` |
| KBEngine Proxy | `kbe/src/server/baseapp/proxy.cpp` | `giveClientTo` / `onGetWitness` |
| BigWorld Proxy | `BigWorld-Engine-14.4.1/server/baseapp/proxy.cpp` | `onClientDeath` / `onRestored` / `attachToClient` |
| BigWorld Base 恢复 | `BigWorld-Engine-14.4.1/server/baseapp/base.cpp` | `restoreTo` / `restoreCellData` |
| **阶段 6：Cell 创建** | | |
| KBEngine Entity | `kbe/src/server/baseapp/entity.cpp` | `createCellEntity` / `createCellEntityInNewSpace` |
| KBEngine CellApp | `kbe/src/server/cellapp/entity.cpp` | `onGetWitness` |
| **阶段 7：Witness 建立** | | |
| KBEngine Witness | `kbe/src/server/cellapp/witness.cpp` | `onEnterSpace` / `installViewTrigger` / `update` |
| BigWorld Witness | `BigWorld-Engine-14.4.1/server/cellapp/witness.cpp` | `Witness()` 构造 / `update` / `addToAoI` |

---

## 22.14 源码漫游路径

### 路径 A：跟随一次完整登录

```
1. kbe/src/server/loginapp/loginapp.cpp
   → 找到 login 方法，看客户端请求如何进入系统

2. kbe/src/server/dbmgr/dbtasks.cpp
   → 找到登录查询 Task，看 DBMgr 如何处理账号查询

3. kbe/src/server/loginapp/loginapp.cpp
   → 找到 onLoginAccountQueryBaseappAddrFromBaseappmgr
   → 看 BaseApp 地址如何回传给客户端

4. kbe/src/server/baseapp/baseapp.cpp
   → 找到 loginBaseapp
   → 看 PendingLoginMgr 如何匹配待登录记录
   → 跟踪 onQueryAccountCBFromDbmgr → createEntity → initializeEntity

5. kbe/src/server/baseapp/proxy.cpp
   → 找到 onGetWitness()
   → 看如何通知 CellApp 获得客户端

6. kbe/src/server/cellapp/entity.cpp
   → 找到 onGetWitness(bool fromBase)
   → 看客户端引用如何恢复、Witness 如何建立

7. kbe/src/server/cellapp/witness.cpp
   → 找到 update()
   → 看进入/离开/属性变更如何统一发给客户端
```

### 路径 B：跟随一次写库

```
1. kbe/src/server/baseapp/entity.cpp
   → Entity::writeToDB

2. kbe/src/server/cellapp/entity.cpp
   → Entity::writeToDB（Cell 侧）
   → backupCellData

3. kbe/src/server/baseapp/entity.cpp
   → Entity::onCellWriteToDBCompleted
   → onPreArchive + addPersistentsDataToStream

4. kbe/src/server/dbmgr/dbtasks.cpp
   → DBTaskWriteEntity

5. kbe/src/server/baseapp/entity.cpp
   → Entity::onWriteToDBCallback
```

### 路径 C：跟随一次重连

```
1. kbe/src/server/baseapp/baseapp.cpp
   → reloginBaseapp
   → 校验 entityID + rndUUID

2. kbe/src/server/baseapp/proxy.cpp
   → createClientProxies
   → onGetWitness()

3. kbe/src/server/cellapp/entity.cpp
   → onGetWitness(true)  // fromBase = true
   → 恢复 clientEntityCall
   → 重建 Witness / resetViewEntities
```

---

## 22.15 小结

玩家生命周期不是几个组件名字的串联，而是四条主线的交织：

| 主线 | 路径 | 承载的内容 |
|------|------|-----------|
| **会话主线** | LoginApp -> BaseApp | 认证、分配、会话绑定 |
| **世界主线** | Base -> Cell -> Witness -> Client | 实体创建、空间状态、视野同步 |
| **数据主线** | Cell -> Base -> DBMgr | 状态收束、持久化落库 |
| **恢复主线** | Proxy / clientEntityCall / Witness | 重连、挤号、控制权迁移 |

理解这四条主线的关键在于：**客户端连接、Base Proxy、Cell 实体、Witness 是四个独立对象**。它们彼此关联但不是同一个东西。正因为拆开了，系统才能支持：

- **重连**：客户端换通道，Proxy 不变，Witness 重建
- **挤号**：旧客户端被踢，Proxy 转绑新客户端
- **控制权迁移**：`giveClientTo` 把客户端从一个 Proxy 切到另一个
- **离线写库**：客户端断开后，Base 仍可完成写库流程

BigWorld 在每个阶段都做了更精细的处理（LoginChallenge、PendingLogins 超时、BackupSender 跨进程备份、精细断线分类），但核心模型是一样的——这就是 BigWorld 定义的 Login/Base/Cell/DB 架构的力量：它把问题分解得足够清晰，以至于后续实现者（如 KBEngine）即使在简化细节时，骨架仍然适用。
