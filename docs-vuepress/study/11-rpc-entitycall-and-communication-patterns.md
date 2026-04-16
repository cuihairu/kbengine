# 11. RPC、EntityCall 与通信模式

> 这一章回答：为什么 MMO 不用 gRPC？EntityCall 和 Mailbox 本质上是什么？tp_call 怎么把一次 Python 函数调用变成跨进程网络消息？BigWorld 的 TwoWay 和 KBEngine 的 CallbackMgr 有什么本质区别？

## 11.1 本章核心问题

- MMO 实体系统的 RPC 需求和传统微服务有什么不同？
- BigWorld Mailbox 和 KBEngine EntityCall 的对应关系？
- tp_call 的完整链路是怎样的？
- newCall_ 的路由逻辑怎么决定消息发往哪个组件？
- Ghost 上的 RealEntityMethod 怎么把调用转接到 real entity？
- Exposed 方法的信任边界是什么？
- TwoWay + Deferred vs 纯单向 + CallbackMgr 的取舍？
- 在纯单向消息模型里，工程上怎么优雅地“拿结果”而不退化成回调地狱？

## 11.2 RPC 的四种通信模式

在讲实现之前，先看通信模式的选择空间：

| 模式 | 描述 | 典型使用者 |
|------|------|-----------|
| **Fire-and-forget** | 发出不管，不关心结果 | UDP 位置更新 |
| **Request-Response** | 发请求，阻塞等回复 | gRPC、HTTP |
| **Deferred/Callback** | 发请求，注册回调，结果来了再触发 | BigWorld TwoWay + PyDeferred |
| **Streaming** | 双向持续流 | WebSocket、gRPC streaming |

### 为什么 MMO 不能选 Request-Response 作为默认

1. **实体方法调用是高频的**：一个 tick 内可能有数百次 Base↔Cell 通信，如果每次都阻塞等回复，10Hz tick 预算根本不够
2. **大部分调用不需要返回值**：`entity.cell.onMove(x, y, z)`、`entity.base.onDamage(100)`——调用方不需要等结果
3. **无状态 vs 有状态**：gRPC 设计给无状态服务，MMO 实体是有状态的——EntityCall 持有远端实体的 ID 和地址，不是每次都做服务发现
4. **顺序保证比响应更重要**：同一 Channel 上的消息需要保序，但不一定需要回复确认

**结论**：MMO 实体 RPC 以 **fire-and-forget 为主**，需要返回值时用 Deferred/Callback 补充。

## 11.3 BigWorld 的选择：Mailbox + 单向为主 + TwoWay 补充

### Mailbox 是什么

Mailbox 是远端实体的"邮箱地址"——一个 Python 对象，持有远端实体的 ID、地址、类型信息。调用 Mailbox 上的方法，就是向远端实体发送一条消息。

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/mailbox.hpp（简化）
class ServerEntityMailBox : public PyEntityMailBox
{
    Py_Header(ServerEntityMailBox, PyEntityMailBox)
public:
    ServerEntityMailBox(EntityTypePtr pBaseType,
            const Mercury::Address& addr, EntityID id,
            PyTypeObject* pType = &s_type_);

    // 获取到远端的输出流（开始写消息）
    virtual BinaryOStream* getStream(const MethodDescription& methodDesc,
            std::auto_ptr<Mercury::ReplyMessageHandler> pHandler);

    virtual ScriptObject pyGetAttribute(const ScriptString& attrObj);
    void sendStream();

    virtual const Mercury::Address& address() const { return addr_; }
    virtual Mercury::UDPChannel* pChannel() const;
    Mercury::Bundle& bundle() const { return this->pChannel()->bundle(); }

protected:
    Mercury::Address addr_;
    EntityID id_;
    EntityTypePtr pLocalType_;
};
```

**Mailbox 的继承层次**：

```
PyEntityMailBox（Python 脚本层 Mailbox）
  └── ServerEntityMailBox（服务器端 Mailbox 基类）
        ├── CommonCellEntityMailBox
        │     └── CellEntityMailBox    ← 调用 Cell 侧实体
        └── CommonBaseEntityMailBox
              └── BaseEntityMailBox    ← 调用 Base 侧实体
```

> 避免误读：这说明 BigWorld 也显式区分 Cell/Base 执行域，不是把这层架构“隐藏掉”。KBEngine 的 `entity.base/entity.cell` 与 BigWorld 的 Mailbox 分域在模型上是同源设计。

### Mailbox 的 getStream：单向 vs TwoWay 的分叉点

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/baseapp/mailbox.cpp:1054（简化）
BinaryOStream* BaseEntityMailBox::getStreamEx(
        const MethodDescription& methodDesc,
        std::auto_ptr<Mercury::ReplyMessageHandler> pHandler)
{
    Mercury::Bundle& bundle = this->bundle();

    BaseAppIntInterface::setClientArgs::start(bundle).id = id_;

    if (pHandler.get())
    {
        // TwoWay：有回复处理器 → 使用 startRequest
        bundle.startRequest(BaseAppIntInterface::callBaseMethod,
                pHandler.release());
    }
    else
    {
        // 单向：无回复处理器 → 使用 startMessage
        bundle.startMessage(BaseAppIntInterface::callBaseMethod);
    }

    bundle << methodDesc.internalIndex();

    return &bundle;
}
```

**关键**：`pHandler` 是否为空决定了单向还是 TwoWay。同一个 Mailbox、同一个 Bundle，区别只在调用 `startMessage` 还是 `startRequest`。

### TwoWay RPC 的完整链路

```
Python: result = entity.base.someMethod(arg1, arg2)
  │
  ├── 方法描述标记为 hasReturnValues_
  │
  ├── BaseEntityMailBox::getStream()
  │     pHandler = new ReturnValuesHandler(deferred, methodDesc)
  │     bundle.startRequest(ie, pHandler)
  │       → 创建 ReplyOrder：{handler, arg, timeout, pReplyID}
  │       → Packet 设置 FLAG_HAS_REQUESTS
  │
  ├── 参数序列化到 Bundle
  │
  ├── Bundle 发送到远端
  │
  ├── [远端执行方法]
  │
  ├── 远端 bundle.startReply(replyID)
  │     → 将结果序列化到回复 Bundle
  │     → 发送回来
  │
  └── 本端 ReturnValuesHandler::handleMessage()
        → 创建回复 Deferred
        → deferred_.callback(result) 或 deferred_.errback(error)
        → Python 脚本的回调被触发
```

### ReturnValuesHandler：TwoWay 的回复处理器

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/return_values_handler.cpp（简化）
void ReturnValuesHandler::handleMessage(
        const Mercury::Address& srcAddr,
        Mercury::UnpackedMessageHeader& header,
        BinaryIStream& data, void* arg)
{
    ScriptObject pValues;

    // 尝试从流中创建返回值，如果失败则创建错误
    uint8 isError = methodDescription_.createReturnValuesOrFailureFromStream(
            data, pValues);

    bool isOkay = isError ?
        deferred_.errback(pValues) :    // 错误回调
        deferred_.callback(pValues);    // 成功回调

    delete this;   // 一次性处理器，用完即销毁
}
```

### CellEntityMailBox 的限制：不支持 TwoWay

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/mailbox.cpp:797（简化）
BinaryOStream* CellEntityMailBox::getStream(
        const MethodDescription& methodDesc,
        std::auto_ptr<Mercury::ReplyMessageHandler> pHandler)
{
    Mercury::Channel* pChannel = this->pChannel();
    if (!pChannel) return NULL;

    // CellApp 不支持 TwoWay 调用
    if (pHandler.get())
    {
        PyErr_Format(PyExc_TypeError,
                "Cannot call two-way method '%s' from CellApp",
                methodDesc.name().c_str());
        return NULL;
    }

    // 只能单向
    Mercury::Bundle& bundle = pChannel->bundle();
    bundle.startMessage(CellAppIntInterface::callEntityMethod);
    // ...
}
```

**设计原因**：CellApp 是高性能空间计算进程，TwoWay 的阻塞等待会影响 tick 频率。只有 BaseApp 可以发起 TwoWay 调用。

### RequestManager：TwoWay 的请求追踪

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/network/request_manager.hpp（简化）
class RequestManager
{
public:
    void addReplyOrder(const ReplyOrder& replyOrder, Channel* pChannel);
    void cancelRequestsFor(Channel* pChannel);
    void cancelRequestsFor(ReplyMessageHandler* pHandler, Reason reason);

    typedef BW::map<int, Request*> RequestMap;
    RequestMap requestMap_;     // replyID → Request
};
```

每个 Bundle 的 `startRequest` 创建一个 ReplyOrder（包含 handler、timeout），发送前注册到 RequestManager。回复到达时，RequestManager 根据 replyID 找到对应的 handler。

## 11.4 KBEngine 的选择：EntityCall = 纯单向 + CallbackMgr

### EntityCall 的本质

EntityCall 不是"一次调用请求"，而是**长期可持有的远端实体引用**。

```cpp
// 文件：kbe/src/lib/entitydef/entitycallabstract.h（简化）
class EntityCallAbstract : public script::ScriptObject
{
public:
    EntityCallAbstract(PyTypeObject* scriptType,
            const Network::Address* pAddr,
            COMPONENT_ID componentID,
            ENTITY_ID eid,
            uint16 utype,
            ENTITYCALL_TYPE type);

    // 核心方法
    virtual Network::Channel* getChannel(void);
    virtual bool sendCall(Network::Bundle* pBundle);
    virtual void newCall(Network::Bundle& bundle);
    virtual void newCall_(Network::Bundle& bundle);

    // 类型判断
    bool isClient() const;
    bool isCell() const;
    bool isBase() const;
    bool isCellReal() const;
    bool isBaseReal() const;

protected:
    Network::Address addr_;             // 远端地址
    COMPONENT_ID componentID_;          // 远端组件 ID
    ENTITYCALL_TYPE type_;              // 调用类型
    ENTITY_ID id_;                      // 远端实体 ID
    ENTITY_SCRIPT_UID utype_;           // 实体脚本类型 UID
};
```

### ENTITYCALL_TYPE：七种通信路径

```cpp
// 文件：kbe/src/lib/common/common.h
enum ENTITYCALL_TYPE
{
    ENTITYCALL_TYPE_CELL            = 0,  // 直接调用 CellApp
    ENTITYCALL_TYPE_BASE            = 1,  // 直接调用 BaseApp
    ENTITYCALL_TYPE_CLIENT          = 2,  // 直接调用客户端
    ENTITYCALL_TYPE_CELL_VIA_BASE   = 3,  // 通过 BaseApp 转发到 CellApp
    ENTITYCALL_TYPE_BASE_VIA_CELL   = 4,  // 通过 CellApp 转发到 BaseApp
    ENTITYCALL_TYPE_CLIENT_VIA_CELL = 5,  // 通过 CellApp 转发到客户端
    ENTITYCALL_TYPE_CLIENT_VIA_BASE = 6   // 通过 BaseApp 转发到客户端
};
```

**为什么需要"Via"类型**：CellApp 上的实体可能没有到客户端的直接 Channel，需要通过 BaseApp 转发。`VIA` 类型描述了这种间接路径。

### EntityCall 继承链

```cpp
// 文件：kbe/src/lib/entitydef/entity_call.h（简化）
class EntityCall : public EntityCallAbstract
{
    INSTANCE_SCRIPT_HREADER(EntityCall, EntityCallAbstract)
public:
    EntityCall(ScriptDefModule* pScriptModule,
            const Network::Address* pAddr,
            COMPONENT_ID componentID,
            ENTITY_ID eid,
            ENTITYCALL_TYPE type);

    // Python 脚本获取属性/方法
    PyObject* onScriptGetAttribute(PyObject* attr);

    // 创建远程方法代理
    virtual RemoteEntityMethod* createRemoteMethod(
            MethodDescription* pMethodDescription);

    virtual void newCall(Network::Bundle& bundle);

    static ENTITYCALLS entityCalls;    // 全局 EntityCall 表

protected:
    ScriptDefModule* pScriptModule_;   // 实体定义模块
};
```

### newCall_ 的路由逻辑

```cpp
// 文件：kbe/src/lib/entitydef/entitycallabstract.cpp:65（简化）
void EntityCallAbstract::newCall_(Network::Bundle& bundle)
{
    // 服务器端
    if (g_componentType != CLIENT_TYPE && g_componentType != BOTS_TYPE)
    {
        if (componentID_ == 0)
        {
            // componentID==0 → 客户端
            bundle.newMessage(ClientInterface::onRemoteMethodCall);
        }
        else
        {
            // 找到目标组件
            Components::ComponentInfos* cinfos =
                Components::getSingleton().findComponent(componentID_);

            if (cinfos != NULL)
            {
                if (cinfos->componentType == BASEAPP_TYPE)
                    bundle.newMessage(BaseappInterface::onEntityCall);
                else
                    bundle.newMessage(CellappInterface::onEntityCall);
            }
        }

        bundle << id_;                // 实体 ID
        if (componentID_ > 0)
            bundle << type_;          // ENTITYCALL_TYPE
    }
    else
    {
        // 客户端发往服务端
        switch (type_)
        {
        case ENTITYCALL_TYPE_BASE:
            bundle.newMessage(BaseappInterface::onRemoteMethodCall);
            break;
        case ENTITYCALL_TYPE_CELL:
            bundle.newMessage(BaseappInterface::onRemoteCallCellMethodFromClient);
            break;
        default:
            break;
        }
        bundle << id_;
    }
}
```

**路由总结**：

| 来源 | 目标 | 路由方式 |
|------|------|---------|
| 服务器 → 客户端 | componentID==0 | `ClientInterface::onRemoteMethodCall` |
| 服务器 → BaseApp | 查找 componentID | `BaseappInterface::onEntityCall` |
| 服务器 → CellApp | 查找 componentID | `CellappInterface::onEntityCall` |
| 客户端 → Base | type_==BASE | `BaseappInterface::onRemoteMethodCall` |
| 客户端 → Cell | type_==CELL | `BaseappInterface::onRemoteCallCellMethodFromClient`（经 BaseApp 中转） |

## 11.5 tp_call：Python 函数调用变成网络消息

这是整个 RPC 系统的核心桥梁——当 Python 脚本执行 `entity.cell.onDamage(100)` 时发生了什么：

```cpp
// 文件：kbe/src/lib/entitydef/remote_entity_method.cpp:45（简化）
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args,
    PyObject* kwds)
{
    RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
    MethodDescription* methodDescription = rmethod->getDescription();
    EntityCallAbstract* entityCall = rmethod->getEntityCall();

    // 1. 参数类型检查
    if (methodDescription->checkArgs(args))
    {
        // 2. 序列化参数到 MemoryStream
        MemoryStream* mstream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
        try
        {
            methodDescription->addToStream(mstream, args);
        }
        catch (MemoryStreamWriteOverflow& err)
        {
            MemoryStream::reclaimPoolObject(mstream);
            S_Return;
        }

        // 3. 创建 Bundle
        Network::Channel* pChannel = entityCall->getChannel();
        Network::Bundle* pSendBundle;
        if (!pChannel)
            pSendBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
        else
            pSendBundle = pChannel->createSendBundle();

        // 4. 写入调用头（实体ID + ENTITYCALL_TYPE + 组件属性占位）
        entityCall->newCall((*pSendBundle));

        // 5. 追加方法参数
        if (mstream->wpos() > 0)
            (*pSendBundle).append(mstream->data(), mstream->wpos());

        // 6. 发送
        MemoryStream::reclaimPoolObject(mstream);
        entityCall->sendCall(pSendBundle);
    }

    S_Return;   // 返回 None（纯单向，无返回值）
}
```

这里要分清两层：

- `EntityCallAbstract::newCall_()` 负责决定“这次调用发往哪个接口消息”，并写入 `entityID`，必要时再写入 `ENTITYCALL_TYPE`
- `EntityCall::newCall()` 在这层之上继续补一个组件属性占位字段；如果是普通实体方法，这个字段就是 0，表示“不是组件内方法”

也就是说，方法自身的目标解析并不是全部塞在 `EntityCallAbstract::newCall_()` 一个函数里完成的。

**完整链路**：

```
Python: entity.cell.onDamage(100)
  │
  ├── EntityCall.onScriptGetAttribute("onDamage")
  │     → ScriptDefModule 查找方法描述
  │     → 创建 RemoteEntityMethod(methodDescription, entityCall)
  │     → 返回这个 Python 可调用对象
  │
  ├── RemoteEntityMethod.tp_call(self, args=(100,))
  │     │
  │     ├── MethodDescription::checkArgs(args)
  │     │     检查参数数量和类型是否匹配 .def 定义
  │     │
  │     ├── MethodDescription::addToStream(mstream, args)
  │     │     将参数序列化到 MemoryStream
  │     │     mstream << (int32)100
  │     │
  │     ├── EntityCall::newCall(bundle)
  │     │     写入消息头：
  │     │       bundle << ENTITY_ID          ← 目标实体 ID
  │     │       bundle << ENTITYCALL_TYPE    ← 调用类型
  │     │       bundle << utype              ← 方法类型 ID
  │     │
  │     ├── bundle.append(mstream)           ← 追加参数数据
  │     │
  │     └── EntityCall::sendCall(bundle)     ← 通过 Channel 发送
  │           └── Channel::send() → TCP → 远端进程
  │
  └── 返回 Py_None（KBEngine 的 RPC 是纯单向）
```

## 11.6 onScriptGetAttribute：方法查找与代理创建

当 Python 脚本访问 `entity.cell.onDamage` 时：

```cpp
// EntityCall::onScriptGetAttribute 的核心逻辑（简化）
PyObject* EntityCall::onScriptGetAttribute(PyObject* attr)
{
    // 在 ScriptDefModule 中查找方法描述
    MethodDescription* pMethodDescription =
        pScriptModule_->findCellMethod(attr);

    if (pMethodDescription)
    {
        // 创建远程方法代理对象
        return createRemoteMethod(pMethodDescription);
    }

    // 查找 base 方法、client 方法...
    // 都没找到则返回 NULL（属性错误）
}
```

**注意**：每次 `entity.cell.onDamage` 访问都会创建一个新的 `RemoteEntityMethod` 对象。这不是性能问题——RemoteEntityMethod 是轻量级的，只是 methodDescription 指针 + entityCall 指针的包装。

## 11.7 接收端：消息如何落地为函数调用

```cpp
// 文件：kbe/src/server/cellapp/entity.h（简化）
class Entity : public script::ScriptObject
{
    // 接收远程方法调用
    void onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s);

    // 接收来自客户端的方法调用
    void onRemoteCallMethodFromClient(Network::Channel* pChannel,
            ENTITY_ID srcEntityID, MemoryStream& s);

    // Ghost 接收对 real 的调用
    void onRemoteRealMethodCall(KBEngine::MemoryStream& s);
};
```

接收端的处理流程：

```
远端发送 onEntityCall 消息
  │
  ├── CellappInterface::onEntityCall handler
  │     反序列化 ENTITY_ID → 找到实体
  │     反序列化 ENTITYCALL_TYPE
  │
  ├── Entity::onRemoteMethodCall(channel, stream)
  │     反序列化 utype（方法类型 ID）
  │     在 ScriptDefModule 中查找 MethodDescription
  │     MethodDescription::createFromStream(stream) → 反序列化参数为 PyObject
  │     调用 Python 脚本方法
  │
  └── Python: entity.onDamage(100) 被调用
```

## 11.8 Ghost 上的 RealEntityMethod：转接到 real

Ghost 实体只有只读副本。当 Ghost 上需要调用一个应该由 real 处理的方法时，通过 `RealEntityMethod` 转发：

```
Ghost 上的 Python 调用: entity.real.onDamage(100)
  │
  ├── RealEntityMethod（不是 RemoteEntityMethod）
  │     检查当前实体是 ghost
  │     将调用转发到 real entity 所在的 CellApp
  │
  ├── Real Entity 收到 onRemoteRealMethodCall
  │     在 real entity 上执行实际逻辑
  │
  └── 结果通过属性同步回 ghost
```

BigWorld 的 RealEntity 使用 `Haunt` 类管理 ghost 的位置信息：

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp（简化）
class RealEntity
{
    class Haunt
    {
    public:
        Haunt(CellAppChannel* pChannel, GameTime creationTime) :
            pChannel_(pChannel),
            creationTime_(creationTime)
        {}

        CellAppChannel& channel() { return *pChannel_; }
        Mercury::Bundle& bundle() { return pChannel_->bundle(); }

    private:
        CellAppChannel* pChannel_;
        GameTime creationTime_;
    };

    void addHaunt(CellAppChannel& channel);      // 添加 ghost 位置
    Haunts::iterator delHaunt(Haunts::iterator);  // 删除 ghost 位置
    void deleteGhosts();

    typedef BW::list<Haunt> Haunts;
    Haunts haunts_;    // 所有 ghost 的位置列表
};
```

**Haunt = ghost 的位置记录**。Real entity 维护所有 ghost 的 Haunt 列表，通过 Haunt 向各个 ghost 推送属性更新。

## 11.9 Exposed 方法的信任边界

### KBEngine

```cpp
// 文件：kbe/src/lib/entitydef/method.h（简化）
class MethodDescription
{
    enum EXPOSED_TYPE
    {
        NO_EXPOSED,                  // 内部调用，客户端不可调用
        EXPOSED,                     // 客户端可调用，不加调用者参数
        EXPOSED_AND_CALLER_CHECK     // 客户端可调用 + 传入调用者 EntityID
    };
    EXPOSED_TYPE exposedType_;
};
```

### BigWorld

BigWorld 使用 `<Exposed/>` XML 标签标记：

```xml
<!-- BigWorld .def 文件 -->
<BaseMethods>
    <logOff>
        <Exposed/>    <!-- 客户端可调用 -->
    </logOff>
</BaseMethods>
```

**Exposed 的信任边界**：

```
客户端 → 服务器方法调用 的安全模型：

1. 只有标记为 Exposed 的方法可以被客户端调用
2. 调用者身份由引擎保证（通过 Proxy/Channel 关联）
3. EXPOSED_AND_CALLER_CHECK：额外传入调用者 EntityID，脚本层可校验
4. 非 Exposed 方法：客户端无法调用——消息 ID 不在暴露范围内
```

BigWorld 使用 `ExposedMessageRange` 管理暴露消息的 ID 范围，这是一种高效的消息过滤机制。

## 11.10 CallbackMgr vs TwoWay + Deferred 的本质区别

### KBEngine CallbackMgr

```cpp
// 文件：kbe/src/lib/server/callbackmgr.h（简化）
// 简单的回调注册表
// 发起请求时：callbackMgr_.save(callbackID, pyCallback)
// 结果回来时：callbackMgr_.take(callbackID) → 执行 pyCallback
```

**使用模式**：

```python
# KBEngine 脚本层（伪代码）
def queryDB(self):
    callbackID = callbackMgr.save(self.onQueryResult)
    sendRequest(callbackID, query)

def onQueryResult(self, result):
    # 处理结果
    pass
```

### BigWorld TwoWay + PyDeferred

```python
# BigWorld 脚本层
def queryDB(self):
    deferred = self.queryDB()
    deferred.addCallback(self.onSuccess)
    deferred.addErrback(self.onError)

def onSuccess(self, result):
    # 处理成功结果
    pass

def onError(self, error):
    # 处理错误
    pass
```

### 对比

| 维度 | KBEngine CallbackMgr | BigWorld TwoWay + PyDeferred |
|------|---------------------|---------------------------|
| 模式 | 简单的 ID → 回调映射 | Promise/Deferred 链 |
| 错误处理 | 脚本层自行处理 | errback 链式错误处理 |
| 组合能力 | 无 | `addCallback(f1).addCallback(f2)` |
| 复杂度 | 低 | 中（需要理解 Twisted Deferred） |
| 适用场景 | DB 查询、延迟操作 | DB 查询、远程调用、异步编排 |
| 超时处理 | 无内置 | RequestManager 内置超时 |

### KBEngine CallbackMgr 的优缺点

优点：

- 实现简单：`save/take` 模式直观，改动面小
- 运行时开销低：不需要维护 Deferred 状态机与链调度
- 工程落地快：短链路请求（一次请求一次回调）接入成本低
- 排障直接：可按 `callbackID` 快速定位一次调用闭环

缺点：

- 组合能力弱：多阶段异步流程需要手工串接，易出现回调嵌套
- 错误通道不统一：成功/失败依赖业务约定，规范容易漂移
- 生命周期风险高：超时、重复回调、未回调都要脚本层自行兜底
- 随复杂度恶化：流程一长，回调分散，维护和重构成本明显上升

### 选型决策清单

| 场景特征 | 更推荐 |
|---------|--------|
| 一次请求一次结果、链路短 | CallbackMgr |
| 需要统一错误链与超时管理 | Deferred/TwoWay |
| 多步异步编排（A→B→C） | Deferred/TwoWay |
| 团队优先追求低学习成本与快速交付 | CallbackMgr |

**一句话**：BigWorld 的 Deferred 是可组合的异步原语（类似 Promise），KBEngine 的 CallbackMgr 是简单的回调注册表。

## 11.11 单向消息流下，如何优雅地“拿结果”

前面说到 `KBEngine` 的 `EntityCall` 本质上是**单条消息单向发送**。这里最容易产生一个误解：

- “单向消息”不等于“没有结果返回”
- 它真正不提供的是“像本地函数一样同步返回值”

换句话说，`KBEngine` 的问题不是“不能拿结果”，而是“不能把跨进程调用写成同步栈帧语义”。一旦接受这个前提，工程上的重点就不再是“怎么假装它是本地函数”，而是：

1. 怎么把“请求 + 响应”表达清楚
2. 怎么把超时、错误、取消、重试放进统一模型
3. 怎么避免业务代码退化成回调地狱

### 11.11.1 先把问题说透：单向消息 vs 同步返回值

`KBEngine` 远程调用最准确的心智模型不是“远程函数调用”，而是：

```text
发送一条命令或请求消息
  → 目标进程在自己的主循环里处理
  → 如有需要，再通过另一条响应消息把结果送回来
```

所以：

- `entity.base.someMethod()` / `entity.cell.someMethod()` 这类调用，默认是 fire-and-forget
- 需要结果时，通常要显式附带一个 `callbackID / reqId`
- 结果返回时，再命中本地的请求跟踪项，继续后续逻辑

这也是 `CallbackMgr` 的工程角色：它不是“TwoWay RPC”，而是**给单向消息流补一个旁路返回通道**。

### 11.11.2 各种工程解法总览

下面这些方案并不是互斥关系，而是从底层到上层、从机制到组织方式的一组连续谱：

| 解法 | 核心思想 | 适合场景 | 主要问题 |
|------|----------|----------|----------|
| 原始 Callback | 结果回来后直接调函数 | 低频、短链路异步请求 | 容易回调嵌套 |
| CPS | 把“后续逻辑”显式当参数传递 | 轻量组织异步续延 | 错误与超时仍要自行管理 |
| Callback Registry | `reqId -> callback` 注册表 | KBEngine 风格的最小统一封装 | 组合能力弱 |
| Future | 把“未来的结果”对象化 | 想把异步结果变成可传递对象 | 需要统一调度器 |
| Promise | `resolve/reject` 驱动 Future 完成 | 通信层封装请求/响应完成态 | 生命周期处理要严谨 |
| Deferred | callback/errback 链式编排 | 老式 Python 异步链 | 可读性不如 `async/await` |
| `async/await` | 用同步语法写异步流程 | 长链路、多步骤流程 | 需要协程调度整合 |
| Actor `ask` | `tell` 与 `ask` 分离 | Base/Cell/DBMgr 进程式架构 | 不能滥用 ask |
| Reducer + Effects | 纯函数只产出副作用 | 函数式、可测试业务 | 初期改造成本较高 |
| Saga / 状态机 | 显式管理长事务步骤与补偿 | 交易、迁移、写库、多步流程 | 设计过重会拖慢开发 |
| CQRS / Read Model | 把常见查询变成本地读模型 | 高频查询、减少跨进程往返 | 要接受最终一致 |
| 数据归属重构 | 让决策发生在数据拥有者所在进程 | 强一致、频繁要结果的逻辑 | 涉及领域边界调整 |

下面按抽象层级逐层展开。

### 11.11.3 原始 Callback 与 CPS：最贴近 KBEngine 当前风格

最原始的写法是：

```python
def askCell(self, payload, callback):
    callbackID = callbackMgr.save(callback)
    sendReqToCell(callbackID, payload)

def onCellRsp(self, callbackID, result):
    cb = callbackMgr.take(callbackID)
    if cb:
        cb(result)
```

这已经是一个可用的请求/响应模型，但它的问题在于：业务一旦变成多步流程，回调就会层层嵌套。

`CPS` 是 `Continuation-Passing Style` 的缩写，意思是：

- 不直接返回结果
- 而是把“拿到结果后该继续做什么”一起传进去

同步思维是：

```python
result = query()
nextStep(result)
```

CPS 写法是：

```python
query(lambda result: nextStep(result))
```

对于 `KBEngine` 这类消息系统，CPS 的价值在于：它承认“跨进程结果一定是晚点到”，然后把“后续流程”显式化。

但要注意，CPS 只是把异步续延表达清楚了，并没有自动解决：

- 超时
- 错误传播
- 取消
- 多步组合

所以它更适合作为底层思维，不适合作为大型业务系统的最终暴露接口。

### 11.11.4 Callback Registry：最小可用统一封装

如果团队希望保持 `KBEngine` 当前风格，同时减少样板代码，最实用的第一步是做一个统一的请求跟踪层：

```python
pending = {}

def ask(dst, method, payload, on_ok, on_err=None, timeout=5):
    req_id = alloc_req_id()
    pending[req_id] = {
        "on_ok": on_ok,
        "on_err": on_err,
        "deadline": now() + timeout,
    }
    send(dst, method, req_id, payload)
    return req_id

def on_rsp(req_id, ok, result=None, error=None):
    req = pending.pop(req_id, None)
    if not req:
        return

    if ok:
        req["on_ok"](result)
    elif req["on_err"]:
        req["on_err"](error)
```

这一步的意义不在于“高级”，而在于**把散落在各业务里的 `callbackID` 管理收口到一处**。

优点：

- 低改造成本
- 与 `CallbackMgr` 心智一致
- 超时、重复响应、未命中请求都可以统一兜底

缺点：

- 调用方仍然要思考回调
- 多步流程仍然容易散
- 结果还不是一等对象，难以组合

如果项目现在大量使用裸 `callbackID`，这一层是非常值得先做的。

### 11.11.5 Future / Promise / Deferred：把“异步结果”对象化

`Future` 的核心思想是：把“未来会完成的结果”包装成一个对象，而不是把后续逻辑直接塞进函数参数。

```python
future = askCellFuture(entityID, "GetHP", {})
future.then(self.onHP)
future.catch(self.onHPError)
```

可以把职责理解成：

- `Future`：给调用方看的只读结果对象
- `Promise`：给通信层持有的可写完成句柄

```python
promise = Promise()
pending[req_id] = promise
sendReq(req_id, payload)

def onRsp(req_id, ok, data):
    promise = pending.pop(req_id)
    if ok:
        promise.resolve(data)
    else:
        promise.reject(data)
```

`Deferred` 可以看成更早期、更显式 callback/errback 链的 Promise 体系。BigWorld 的 `TwoWay + PyDeferred` 本质上就在这里。

这类封装的主要收益是：

- 成功/失败通道统一
- 可以做链式组合
- 可以集中实现超时、取消、重试、日志、追踪
- 业务层不再直接接触 `pending[reqId]`

但也不要误读：

- 它们没有把异步变同步
- 只是把“结果回来后怎么继续”从裸回调提升成了可组合对象

### 11.11.6 `async/await`：可读性最好，但需要调度器

如果再往上封装一层，就会得到：

```python
async def upgradeSkill(self, skillID):
    combatInfo = await askCell(self.id, "GetCombatInfo", {})
    skillInfo = await askDB("LoadSkill", {"skillID": skillID})

    result = calcUpgrade(combatInfo, skillInfo)
    await askDB("SaveSkill", result)

    self.client.onUpgradeSkillResult(result)
```

这类写法的优点非常明显：

- 复杂流程最清晰
- 可以自然使用 `try/except`
- 多步链路的控制流接近同步思维

先把版本边界说清楚，否则很容易把“语法支持”和“引擎运行时支持”混为一谈：

| 能力 | Python 版本 |
|------|-------------|
| `yield from` 生成器式协程语法 | `3.3+` |
| `asyncio` 标准库进入 CPython | `3.4+` |
| `async def` / `await` 正式语法 | `3.5+` |
| `asyncio.run()` | `3.7+` |

这意味着：

- `await` 不是 Python `3.12` 才有的语法，而是 **Python `3.5` 就已经正式支持**
- `KBEngine` 最后维护时使用的 Python `3.7.3`，在语法层面已经可以完整使用 `async def` / `await`
- 当前库升级到 Python `3.12.13`，收益主要是 `asyncio` 生态、任务管理、调试信息和运行时实现更成熟，而**不是**“终于能写 `await` 了”

这里最关键的理解是：

- `await` 不等于“把远程调用变成同步返回值”
- `await` 的真实语义是：**当前协程先挂起，等一个 awaitable 完成后，再由调度器恢复执行**

所以在 `KBEngine` 语境里，真正的难点从来不是关键字本身，而是下面这些运行时能力有没有补齐：

- 请求 ID 分配与响应路由
- `pending` 请求表管理
- 超时、取消、异常传播
- 收到响应后恢复正确的协程
- 把协程调度和 `BaseApp / CellApp` 主循环整合起来

如果这些能力没有封装好，那么 `await askCell(...)` 只是更好看的伪代码，而不是现成可落地的 API。

下面给两个例子。第一个例子是标准 Python，可直接说明 `await` 究竟在做什么；第二个例子再贴近 `KBEngine` 的 `Req/Rsp` 封装方式。

#### 一个可直接运行的完整 Python 例子

这段代码在 Python `3.7.3` 和 `3.12.13` 上都可以运行，用它来理解 `await` 的语义最直接：

```python
import asyncio


async def ask_cell(entity_id, method, payload):
    print(f"[send] entity={entity_id}, method={method}, payload={payload}")

    # 模拟远端处理和网络往返。await 到这里时，当前协程会挂起，
    # 主循环可以先去执行其他任务，而不是阻塞整个线程。
    await asyncio.sleep(0.1)

    if method == "GetCombatInfo":
        return {"hp": 120, "mp": 80, "level": 15}

    if method == "UpgradeSkill":
        return {"ok": True, "newLevel": 4}

    raise ValueError(f"unknown method: {method}")


async def upgrade_skill(entity_id, skill_id):
    combat = await ask_cell(entity_id, "GetCombatInfo", {})
    if combat["level"] < 10:
        return {"ok": False, "reason": "level too low"}

    result = await ask_cell(entity_id, "UpgradeSkill", {"skillID": skill_id})
    return {
        "ok": result["ok"],
        "skillID": skill_id,
        "newLevel": result["newLevel"],
    }


async def main():
    result = await upgrade_skill(1001, 20001)
    print("[result]", result)


if __name__ == "__main__":
    asyncio.run(main())
```

这段例子能说明三件事：

1. `await` 从 Python `3.5` 开始就是正式语法，所以 `3.7.3` 完全能写
2. `asyncio.run()` 是 Python `3.7` 引入的，所以 `KBEngine` 停留在 `3.7.3` 时也已经能用这种入口
3. `await asyncio.sleep(0.1)` 的本质不是“卡住 0.1 秒”，而是“把当前协程交回调度器，等完成后再继续”

#### 一个贴近 KBEngine 的完整封装示例

下面这个例子不是说 `KBEngine` 现在就原生提供了这样的接口，而是说明：如果要把 callback 风格封装成 `await` 风格，工程上大致需要哪些部件。

```python
import asyncio


class RpcBridge:
    def __init__(self):
        self._next_req_id = 1
        self._pending = {}

    async def ask(self, remote_stub, method, payload, timeout=3.0):
        loop = asyncio.get_running_loop()
        future = loop.create_future()

        req_id = self._next_req_id
        self._next_req_id += 1
        self._pending[req_id] = future

        remote_stub.call_remote(method, {
            "reqID": req_id,
            "payload": payload,
        })

        try:
            return await asyncio.wait_for(future, timeout=timeout)
        finally:
            self._pending.pop(req_id, None)

    def on_response(self, req_id, data=None, error=None):
        future = self._pending.get(req_id)
        if future is None or future.done():
            return

        if error is not None:
            future.set_exception(RuntimeError(error))
        else:
            future.set_result(data)


class FakeRemoteCell:
    def __init__(self, bridge):
        self._bridge = bridge

    def call_remote(self, method, message):
        req_id = message["reqID"]
        payload = message["payload"]
        asyncio.create_task(self._process(req_id, method, payload))

    async def _process(self, req_id, method, payload):
        await asyncio.sleep(0.1)

        if method == "GetCombatInfoReq":
            self._bridge.on_response(req_id, {
                "level": 15,
                "hp": 120,
                "mp": 80,
            })
            return

        if method == "UpgradeSkillReq":
            self._bridge.on_response(req_id, {
                "ok": True,
                "skillID": payload["skillID"],
                "newLevel": 4,
            })
            return

        self._bridge.on_response(req_id, error=f"unknown method: {method}")


class Avatar:
    def __init__(self):
        self.rpc = RpcBridge()
        self.cell = FakeRemoteCell(self.rpc)

    async def upgrade_skill(self, skill_id):
        combat = await self.rpc.ask(self.cell, "GetCombatInfoReq", {})
        if combat["level"] < 10:
            return {"ok": False, "reason": "level too low"}

        result = await self.rpc.ask(self.cell, "UpgradeSkillReq", {
            "skillID": skill_id,
        })
        return result


async def main():
    avatar = Avatar()
    result = await avatar.upgrade_skill(20001)
    print(result)


if __name__ == "__main__":
    asyncio.run(main())
```

这个例子里，`await` 能成立，不是因为语法神奇，而是因为下面这条链路已经被补齐：

```text
业务协程
  -> await rpc.ask(...)
  -> ask 内部创建 Future 并登记到 pending
  -> 远端返回 reqID 对应的响应
  -> bridge.on_response() 完成 Future
  -> 调度器恢复原先挂起的协程
```

但它的前提不是“会写 `async`”，而是**你要有一个和主循环整合良好的协程调度器**：

- `await` 不能阻塞主线程
- 响应回来时要恢复被挂起的协程
- 超时要能抛异常或返回失败态
- 协程取消要有明确语义

所以在 `KBEngine` 里，`async/await` 适合作为成熟封装的最上层语法糖，而不适合作为第一步改造。

### 11.11.7 Actor `tell/ask`：最符合 Base/Cell/DBMgr 心智模型

如果团队希望统一“分布式进程 + 实体”这套认知，最贴切的模型其实是 Actor：

| 模式 | 含义 |
|------|------|
| `tell` | 告诉对方做一件事，不等结果 |
| `ask` | 发一个带回复通道的请求，异步等待结果 |

对 `KBEngine` 来说，这是一个很自然的分层：

- 大多数 `EntityCall` 都应该被视为 `tell`
- 少数确实要结果的链路，再显式建模为 `ask`

例如：

```python
self.cell.tell("ApplyDamage", payload)

self.cell.ask("CheckCanAttack", payload) \
    .then(self.onCanAttack) \
    .catch(self.onAttackError)
```

这个模型有两个重要好处：

1. 它不会误导开发者把远程对象当成本地对象
2. 它能迫使团队区分“命令”和“查询”

但也要控制边界：`ask` 不能滥用。否则系统虽然语法上仍是异步，思维上却会退回“满屏远程查询响应”。

### 11.11.8 Reducer + Effects：最接近“函数式化解单向数据流”

如果问题不是“怎么拿结果”，而是“怎么让单向消息系统的业务代码不痛苦”，函数式里最有价值的方案不是 Hook，而是：

- `Reducer`
- `Effects`

也就是：

```text
收到一条消息
  → 纯函数决定状态怎么变
  → 同时产出需要执行的副作用
  → 副作用执行器去发消息、查库、启动定时器
  → 结果回来后再变成一条新消息，喂回 reducer
```

伪代码：

```python
def reduce(state, msg):
    if msg["type"] == "UpgradeSkillRequested":
        return state, [
            AskCell("GetCombatInfo", {"entityID": state.entityID}),
            AskDB("LoadSkill", {"skillID": msg["skillID"]}),
        ]

    if msg["type"] == "CombatInfoLoaded":
        return state.withCombatInfo(msg["data"]), []

    if msg["type"] == "SkillUpgradeCommitted":
        new_state = applyUpgrade(state, msg["data"])
        return new_state, [
            NotifyClient("SkillUpgradeResult", msg["data"])
        ]
```

它特别适合 `KBEngine` 这种单向消息流系统，因为：

- 业务逻辑可以做成纯函数，容易测试
- 网络、副作用、超时集中管理
- 响应回来不是“回调跳转”，而是“进入下一轮消息处理”

如果要找一个和前端更接近的类比，它更像：

- `Redux`
- `Elm Architecture`

而不是 `React Hooks`

`Hooks` 主要解决组件内部复用与生命周期问题；`Reducer + Effects` 才是“单向数据流 + 外部副作用”的正对位方案。

### 11.11.9 状态机 / Saga：处理长事务而不是普通回调

当流程进入多步骤、跨组件、需要补偿的区间时，继续叠 Promise 或回调就不够稳了。这时更合适的是：

- 显式状态机
- Saga / Process Manager

例如交易流程：

```text
Idle
→ WaitingCellLock
→ WaitingDBCommit
→ WaitingCellApply
→ Completed / Failed
```

每一条响应消息都只是推动状态机前进一步。

这类方案适合：

- 扣货币发道具
- 跨服迁移
- 交易
- 传送
- 邮件领奖
- 多步写库与回滚

关键价值不是“语法更优雅”，而是：

- 补偿逻辑清楚
- 失败边界清楚
- 超时语义清楚
- 重启恢复更容易做

### 11.11.10 CQRS / Read Model / 数据归属重构：从架构上减少“想拿结果”的次数

很多“我想在一个调用里立刻拿到结果”的场景，根因不是封装不够高级，而是：

- 查询频率太高
- 决策位置不合理
- 数据归属不清晰

这时候比 Promise/Future 更有效的，往往是架构上的处理。

第一类是 `CQRS / Read Model`：

- 把高频查询需要的数据做成本地可读快照
- 不要每次都跨进程 ask

例如：

```text
CellApp 持有实时战斗权威状态
  → 同步 CombatSnapshot 到 BaseApp
  → BaseApp 直接读取本地摘要
```

代价是：

- 接受最终一致
- 不适合强一致决策

第二类是数据归属重构：

如果一个流程总是：

```text
Base 问 Cell 能不能做
Cell 回答可以
Base 再命令 Cell 去做
```

那通常意味着这个决策本来就应该在 Cell 做。

正确重构往往是：

```text
Base 发“请求执行攻击”
Cell 在本地完成校验与执行
Cell 再把结果回给 Base
```

这比“先 ask 再 tell”更符合单向消息系统的设计。

### 11.11.11 反模式：不要把远程调用硬写成阻塞 RPC

最需要避免的方案其实很简单：

```python
result = remoteCallAndWait()
```

如果这发生在 `BaseApp / CellApp` 的主循环线程里，问题会非常直接：

- 阻塞消息泵
- 增大 tick 抖动
- 容易形成链式超时
- 极端情况下造成死锁

管理工具、后台线程、离线脚本可以做同步等待；主业务循环不要这样做。

### 11.11.12 一个最小可用的 `Req/Rsp + Promise/Future` 模板

如果要把这套思路落到 `KBEngine` 项目里，一个足够克制、又有明显收益的最小模板大致长这样：

```python
class Promise:
    def __init__(self):
        self.future = Future()

    def resolve(self, value):
        self.future._resolve(value)

    def reject(self, error):
        self.future._reject(error)


class Future:
    def __init__(self):
        self.done = False
        self.result = None
        self.error = None
        self.callbacks = []
        self.errbacks = []

    def then(self, callback):
        if self.done and self.error is None:
            callback(self.result)
        else:
            self.callbacks.append(callback)
        return self

    def catch(self, errback):
        if self.done and self.error is not None:
            errback(self.error)
        else:
            self.errbacks.append(errback)
        return self

    def _resolve(self, value):
        if self.done:
            return
        self.done = True
        self.result = value
        for cb in self.callbacks:
            cb(value)

    def _reject(self, error):
        if self.done:
            return
        self.done = True
        self.error = error
        for eb in self.errbacks:
            eb(error)
```

通信层再封一层：

```python
class RpcSession:
    def __init__(self):
        self.pending = {}

    def ask(self, target, method, payload, timeout=5.0):
        req_id = self._alloc_req_id()
        promise = Promise()
        self.pending[req_id] = {
            "promise": promise,
            "deadline": self._now() + timeout,
        }

        self._send_req(target, method, req_id, payload)
        return promise.future

    def on_response(self, req_id, ok, result=None, error=None):
        req = self.pending.pop(req_id, None)
        if not req:
            return

        if ok:
            req["promise"].resolve(result)
        else:
            req["promise"].reject(error)

    def tick(self):
        now = self._now()
        timeout_ids = [
            req_id
            for req_id, req in self.pending.items()
            if req["deadline"] <= now
        ]

        for req_id in timeout_ids:
            req = self.pending.pop(req_id, None)
            if req:
                req["promise"].reject("timeout")
```

业务层再用它：

```python
def requestAttack(self, targetID):
    self.rpc.ask(
        self.cell,
        "CheckCanAttack",
        {"attackerID": self.id, "targetID": targetID},
        timeout=0.5,
    ).then(
        self._onCanAttack
    ).catch(
        self._onAttackRejected
    )
```

这个模板的重点不是“高级”，而是先把几个最关键的工程责任统一起来：

- `reqId` 分配
- `pending` 生命周期
- 超时清理
- 成功/失败双通道
- 业务层不再直接操作 `callbackID`

如果再往上走一步，就可以把 `.then().catch()` 再包成 `async/await` 或 reducer/effect 风格；但第一步先做到这里，收益已经很明显。

### 11.11.13 一个贴近 `BaseApp -> CellApp` 的完整例子

假设有一个典型业务：玩家在 `BaseApp` 发起攻击请求，但真正的实时判定权在 `CellApp`。这时最容易写成下面这种两段式：

```text
BaseApp 问 CellApp：能不能打？
CellApp 回答：可以 / 不可以
BaseApp 再决定是否发“执行攻击”
```

如果只是教学演示，这样没有问题；但在真实项目里，它会暴露两个风险：

1. `check` 与 `use` 之间存在状态窗口，目标可能已经变化
2. `BaseApp` 开始承担本来属于 `CellApp` 的强一致决策

更合理的工程写法一般分两档。

**第一档：保留 ask，但把协议写清楚**

```python
# BaseApp
def requestAttack(self, targetID):
    self.rpc.ask(
        self.cell,
        "CheckCanAttack",
        {
            "attackerID": self.id,
            "targetID": targetID,
            "skillID": self.currentSkillID,
        },
        timeout=0.3,
    ).then(
        lambda result: self._onAttackCheckOK(targetID, result)
    ).catch(
        lambda error: self.client.onAttackFailed(error)
    )


def _onAttackCheckOK(self, targetID, result):
    if not result["ok"]:
        self.client.onAttackFailed(result["reason"])
        return

    self.cell.tell(
        "ApplyAttack",
        {
            "attackerID": self.id,
            "targetID": targetID,
            "skillID": self.currentSkillID,
        }
    )
```

这个版本适合：

- 判定链路不长
- `BaseApp` 确实还需要读这个中间结果
- 项目暂时不准备调整领域边界

但它仍然有 `check -> use` 窗口。

**第二档：把 ask 收缩成“请求执行”，由 CellApp 本地判定并执行**

```python
# BaseApp
def requestAttack(self, targetID):
    self.rpc.ask(
        self.cell,
        "RequestAttack",
        {
            "attackerID": self.id,
            "targetID": targetID,
            "skillID": self.currentSkillID,
        },
        timeout=0.3,
    ).then(
        self._onAttackResult
    ).catch(
        lambda error: self.client.onAttackFailed(error)
    )


def _onAttackResult(self, result):
    if not result["ok"]:
        self.client.onAttackFailed(result["reason"])
        return

    self.client.onAttackStarted(result["combatSnapshot"])
```

```python
# CellApp
def onRequestAttack(self, attackerID, targetID, skillID, reply):
    if not self._canAttack(attackerID, targetID, skillID):
        reply.fail({"reason": "out_of_range"})
        return

    combatSnapshot = self._applyAttack(attackerID, targetID, skillID)
    reply.ok({
        "ok": True,
        "combatSnapshot": combatSnapshot,
    })
```

这个版本更符合 `KBEngine` 的分布式实体模型，因为：

- 强一致判定发生在数据拥有者所在进程
- `BaseApp` 只处理结果，不重复做实时判定
- `CellApp` 内部可以一次性完成校验、扣资源、落战斗态

经验上：

- “先 ask 一个布尔值，再 tell 真正执行”通常是过渡方案
- “ask 一个执行请求，结果里返回执行结果或快照”通常是更稳的最终形态

### 11.11.14 什么时候该用 `tell`、`ask`、读模型，还是直接重构数据归属

下面这张表可以当成实战判断清单：

| 场景特征 | 更推荐 | 原因 |
|------|------|------|
| 只是通知对方做事，不关心结果 | `tell` | 最符合 fire-and-forget 主路径，简单且高吞吐 |
| 需要一次性返回结果，链路短，频率不高 | `ask` | 显式请求/响应最直接 |
| 需要结果，但查询非常高频 | 读模型 / CQRS | 减少跨进程往返，稳定性能 |
| 结果用于强一致决策，且数据权威在对端 | 数据归属重构 | 让决策在数据拥有者本地完成 |
| 多步骤流程、可能失败、需要补偿 | 状态机 / Saga | 统一管理步骤推进、超时与补偿 |
| 只是为了“语法像同步”而发请求 | 不推荐 | 这是把远程调用误当成本地调用 |

再换一个更口语化的判断法：

- **能用 `tell` 就别先上 `ask`**
- **能把判断搬到数据拥有者本地，就别做“问一下再执行”**
- **能用本地快照读，就别高频跨进程查询**
- **流程一长，就别继续堆 callback，要上状态机或 Saga**

### 11.11.15 推荐落地顺序

如果站在 `KBEngine` 项目的工程现实看，最稳妥的演进顺序通常是：

1. 先统一 `Req/Rsp + reqId + pending map`
2. 再在其上封装 `Promise/Future`
3. 对复杂长链路引入状态机或 Saga
4. 对高频查询做读模型或数据归属重构
5. 如果脚本运行时条件成熟，再考虑 `async/await`

不要一开始就上完整协程框架，也不要继续让每个业务自己拼 `callbackID`。

### 11.11.16 一句话原则

- **把 `EntityCall` 当成异步消息投递，不要当成本地函数调用**
- **把“需要结果”的远程调用当成显式的请求/响应协议**
- **把“怎么继续”从散落回调提升成统一抽象**
- **把高频强一致决策放到数据拥有者所在进程**

如果只允许记住一句：

> 在 KBEngine 里，解决“单向消息流下怎么拿结果”的最佳路径，不是伪造同步返回值，而是把请求/响应、异步结果对象、状态推进和数据归属都建模清楚。

## 11.12 与 gRPC / Protobuf / HTTP 的设计对比

| 维度 | gRPC / Protobuf | EntityCall / Mailbox |
|------|----------------|---------------------|
| **服务发现** | 每次调用查询注册中心 | EntityCall 持有实体地址，无需发现 |
| **接口定义** | .proto → 代码生成 | .def → 运行时元数据（MethodDescription） |
| **状态模型** | 无状态服务 | 有状态实体 |
| **调用模式** | Request-Response 默认 | Fire-and-forget 默认 |
| **负载均衡** | 客户端侧 LB | 由 Mgr 组件（BaseappMgr/CellappMgr）集中调度 |
| **连接管理** | 每次新建或连接池 | Channel 长连接，EntityCall 复用 |
| **序列化** | Protobuf 编码 | 手工流编码（MemoryStream） |
| **代码生成** | protoc 自动生成 | 宏 + 模板注册 |
| **错误处理** | status code + retry | Channel 断开 → EntityCall 失效 |

**为什么通用 RPC 框架不适合 MMO 实体系统**：

1. **实体是有状态的**：每次调用需要路由到特定实体所在的特定进程，不是随便找一个实例
2. **高频小消息**：gRPC 的 HTTP/2 帧开销对 MMO 太重
3. **单向调用为主**：gRPC 一元调用是 req-resp，大部分实体方法不需要回复
4. **属性同步**：gRPC 没有内建的"只发变更字段"机制
5. **EntityCall 可序列化**：EntityCall 可以写入 Bundle 传给其他进程，gRPC 的 stub 做不到

## 11.13 两套项目的 RPC 系统对比

| 维度 | KBEngine EntityCall | BigWorld Mailbox |
|------|-------------------|-----------------|
| 概念名 | EntityCall | Mailbox |
| 本质 | 远端实体的 Python 引用 | 远端实体的 Python 引用 |
| 通信模式 | 纯单向 | 单向 + TwoWay |
| 返回值 | 无（Py_None） | 有（PyDeferred + returnValues_） |
| 路由类型 | 7 种 ENTITYCALL_TYPE | 3 种 Component（Cell/Base/Client） |
| 代理对象 | RemoteEntityMethod | PyEntityMailBox 的 tp_call |
| 请求追踪 | CallbackMgr（外层） | RequestManager + ReplyOrder（内置） |
| 错误回调 | 无内置 | errback 链 |
| 超时处理 | 无内置 | RequestManager timeout |
| CellApp TwoWay | 不支持（无此功能） | 不支持（显式报错） |
| Ghost 转接 | RealEntityMethod → onRemoteRealMethodCall | RealEntity::Haunt |
| 暴露方法 | EXPOSED_TYPE 三级 | Exposed 标签 + ExposedMessageRange |

避免误读：`KBEngine 暴露 base/cell` 不是“只有 KBEngine 这样做”。BigWorld 同样要求理解 Cell/Base/Client 执行域；主要差异在 RPC 能力与接口细节（KBEngine `ENTITYCALL_TYPE` 路由变体 vs BigWorld TwoWay/Deferred）以及配套工具链。

## 11.14 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| EntityCall 基类 | `kbe/src/lib/entitydef/entitycallabstract.h` |
| EntityCall 实现 | `kbe/src/lib/entitydef/entity_call.h` |
| newCall_ 路由 | `kbe/src/lib/entitydef/entitycallabstract.cpp` |
| RemoteEntityMethod | `kbe/src/lib/entitydef/remote_entity_method.h` |
| tp_call 实现 | `kbe/src/lib/entitydef/remote_entity_method.cpp` |
| MethodDescription | `kbe/src/lib/entitydef/method.h` |
| ENTITYCALL_TYPE | `kbe/src/lib/common/common.h` |
| CallbackMgr | `kbe/src/lib/server/callbackmgr.h` |
| Entity 接收端 | `kbe/src/server/cellapp/entity.h` |
| Proxy 客户端通信 | `kbe/src/server/baseapp/proxy.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| Mailbox 基类 | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/mailbox.hpp` |
| CellEntityMailBox | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/mailbox.hpp` |
| BaseEntityMailBox | `BigWorld-Engine-14.4.1/programming/bigworld/server/baseapp/mailbox.hpp` |
| getStream (Base) | `BigWorld-Engine-14.4.1/programming/bigworld/server/baseapp/mailbox.cpp` |
| ReturnValuesHandler | `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/return_values_handler.cpp` |
| RequestManager | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/request_manager.hpp` |
| ReliableType | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/bundle.hpp` |
| UDPBundle startRequest | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/udp_bundle.cpp` |
| RealEntity + Haunt | `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp` |
| ExposedMessageRange | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/exposed_message_range.hpp` |
| TwoWay 初始化 | `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/method_description.cpp` |

## 11.15 源码走读路径

### 路径一：跟踪一次 EntityCall RPC 的完整发送

1. `kbe/src/lib/entitydef/entity_call.h` — EntityCall 类声明
2. `kbe/src/lib/entitydef/entitycallabstract.cpp:65` — `newCall_()` 路由逻辑
3. `kbe/src/lib/entitydef/remote_entity_method.cpp:45` — `tp_call()` 序列化+发送
4. `kbe/src/lib/entitydef/method.h` — `checkArgs()` / `addToStream()`

### 路径二：跟踪一次 RPC 的接收处理

1. `kbe/src/server/cellapp/entity.h` — `onRemoteMethodCall()` 入口
2. `kbe/src/lib/entitydef/method.h` — `createFromStream()` 反序列化参数
3. Python 脚本方法被执行

### 路径三：对比 BigWorld 的 TwoWay RPC

1. `BigWorld-Engine-14.4.1/programming/bigworld/server/baseapp/mailbox.cpp:1054` — `getStreamEx()` 单向/TwoWay 分叉
2. `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/udp_bundle.cpp:259` — `startRequest()` 创建 ReplyOrder
3. `BigWorld-Engine-14.4.1/programming/bigworld/lib/entitydef/return_values_handler.cpp` — `handleMessage()` 触发 Deferred 回调

### 路径四：理解 Ghost → Real 的调用转接

1. `kbe/src/server/cellapp/entity.h` — `onRemoteRealMethodCall()`
2. `BigWorld-Engine-14.4.1/programming/bigworld/server/cellapp/real_entity.hpp` — `Haunt` 类
3. 对比：两套项目的 ghost→real 转发机制

## 11.16 小结

- **MMO 的 RPC 以 fire-and-forget 为主**：高频实体通信不需要 req-resp 的同步等待
- **EntityCall / Mailbox 是远端实体的 Python 引用**：持有 ID、地址、类型信息，可以像本地对象一样调用
- **tp_call 是核心桥梁**：把 Python 函数调用 → 参数检查 → 序列化 → 网络发送一步完成
- **newCall_ 路由逻辑**：根据 ENTITYCALL_TYPE / Component 决定消息发往哪个进程
- **BigWorld 多了 TwoWay**：startRequest + ReturnValuesHandler + PyDeferred，提供完整的异步调用链
- **KBEngine 选择更简单**：纯单向 + 外层 CallbackMgr，牺牲了组合能力但降低了复杂度
- **CellApp 都不支持 TwoWay**：两个项目都不允许 CellApp 发起需要回复的远程调用
- **Exposed 是安全边界**：只有标记为 Exposed 的方法才能被客户端调用
- **通用 RPC 框架不适合 MMO**：有状态实体、高频单向调用、EntityCall 可序列化——这些需求 gRPC 都不原生支持
