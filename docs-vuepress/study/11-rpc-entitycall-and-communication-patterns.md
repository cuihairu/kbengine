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

**一句话**：BigWorld 的 Deferred 是可组合的异步原语（类似 Promise），KBEngine 的 CallbackMgr 是简单的回调注册表。

## 11.11 与 gRPC / Protobuf / HTTP 的设计对比

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

## 11.12 两套项目的 RPC 系统对比

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

## 11.13 关键源码入口

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

## 11.14 源码走读路径

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

## 11.15 小结

- **MMO 的 RPC 以 fire-and-forget 为主**：高频实体通信不需要 req-resp 的同步等待
- **EntityCall / Mailbox 是远端实体的 Python 引用**：持有 ID、地址、类型信息，可以像本地对象一样调用
- **tp_call 是核心桥梁**：把 Python 函数调用 → 参数检查 → 序列化 → 网络发送一步完成
- **newCall_ 路由逻辑**：根据 ENTITYCALL_TYPE / Component 决定消息发往哪个进程
- **BigWorld 多了 TwoWay**：startRequest + ReturnValuesHandler + PyDeferred，提供完整的异步调用链
- **KBEngine 选择更简单**：纯单向 + 外层 CallbackMgr，牺牲了组合能力但降低了复杂度
- **CellApp 都不支持 TwoWay**：两个项目都不允许 CellApp 发起需要回复的远程调用
- **Exposed 是安全边界**：只有标记为 Exposed 的方法才能被客户端调用
- **通用 RPC 框架不适合 MMO**：有状态实体、高频单向调用、EntityCall 可序列化——这些需求 gRPC 都不原生支持
