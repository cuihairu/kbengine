# 附录 C 外部参考系统速查

> 本书多处将 BigWorld/KBEngine 的设计与外部系统做对比。本章提供这些系统的独立参考，方便读者理解对比的上下文。

---

## C.1 gRPC / Protobuf：req-resp 模式的工业标准

### 是什么

gRPC 是 Google 开源的高性能 RPC 框架，使用 Protocol Buffers 作为接口定义语言和序列化格式。

### 核心模型

```protobuf
// .proto 文件定义服务接口
service EntityService {
  rpc GetEntity(EntityRequest) returns (EntityResponse);       // 单向
  rpc StreamEntities(EntityRequest) returns (stream Entity);   // 服务端流
  rpc UploadEvents(stream Event) returns (Ack);                // 客户端流
  rpc Chat(stream Message) returns (stream Message);           // 双向流
}
```

### 与 EntityCall 的对比

| 维度 | gRPC | EntityCall |
|------|------|-----------|
| 通信模式 | 4 种（Unary + 3 种 Stream） | 1 种（Fire-and-forget） |
| 接口定义 | .proto 文件 | .def 文件 |
| 服务发现 | DNS / Consul / etcd | EntityDef 注册 |
| 调用目标 | 无状态服务 | 有状态实体 |
| 序列化 | Protobuf（强 schema） | 自定义流编码 |
| 连接模型 | HTTP/2 长连接 | TCP/UDP 长连接 |
| 流控 | HTTP/2 flow control | 应用层控制 |

### 为什么 MMO 不用 gRPC

1. **有状态 vs 无状态**：gRPC 设计目标是微服务（无状态），MMO 的 EntityCall 是有状态的实体间通信
2. **延迟预算**：HTTP/2 的帧复用和流控增加延迟，MMO 需要极致低延迟
3. **序列化开销**：Protobuf 的 tag-length-value 编码比引擎自定义流编码大
4. **连接模型**：gRPC 一个连接上多路复用，游戏服务器更倾向于一个 Channel 对应一个实体对
5. **无实体概念**：gRPC 的 service/method 不适合表达"对某个实体调用某个方法"

### 可以借鉴的

- **接口定义语言**（.proto → .def 的设计灵感来源）
- **代码生成**（protoc → EntityDef 代码生成）
- **流式 RPC**（在特定场景下，如大地图数据流，可借鉴）

---

## C.2 nng：多模式通信

### 是什么

nng (Nanomsg Next Generation) 是一个轻量级消息传递库，提供多种通信模式。

### 通信模式

| 模式 | 拓扑 | 典型用途 |
|------|------|---------|
| **Pair** | 1:1 | 两个组件直接通信 |
| **Req/Rep** | 1:N | 请求-响应（同步） |
| **Pub/Sub** | 1:N | 主题订阅广播 |
| **Push/Pull** | 1:N | 任务分发/收集 |
| **Survey** | N:1 | 一次查询，所有节点回复 |
| **Bus** | N:N | 多方广播 |

### 与游戏服务器的对应

| nng 模式 | 游戏场景 |
|----------|---------|
| Pair | Base ↔ Cell 的 EntityCall 通道 |
| Pub/Sub | 属性同步（一个实体 → N 个客户端） |
| Push/Pull | 任务分发（BaseAppMgr → BaseApp 分配） |
| Survey | ForwardingWatcher（一次查询所有 CellApp） |
| Req/Rep | 客户端登录（Login → DBMgr 查询） |

### Survey 模式的特殊价值

```
Survey 模式 = "一次询问，所有节点回复"
                ↓
BigWorld 的 ForwardingWatcher 本质上就是这个模式：
    BaseAppMgr 发 survey → 所有 BaseApp 回复 watcher 值
    CellAppMgr 发 survey → 所有 CellApp 回复负载信息
```

### 为什么游戏服务器不用 nng

1. **延迟**：nng 是通用库，没有针对游戏场景的优化
2. **无实体抽象**：nng 只有 socket，没有 Entity/EntityCall 的概念
3. **无 AOI 集成**：消息路由不感知空间位置
4. **自定义协议**：引擎需要深度定制消息格式（alias、压缩等）

---

## C.3 Aeron：高性能 UDP 消息传递

### 是什么

Aeron 是一个高效的 UDP 消息传递库，设计目标是从日志复制到消息传递的统一传输。

### 核心设计

```
Publisher ──→ Media Driver ──→ Subscriber
              (共享内存/UDP)

关键特性：
1. 单播 (Unicast) + 多播 (Multicast)
2. 背压感知 (Back-pressure aware)
3. 零拷贝 (通过内存映射文件)
4. 可靠传输 (重传 + 去重)
5. 流控 (流量控制)
```

### 与 BigWorld Mercury 的对比

| 维度 | Aeron | BigWorld Mercury |
|------|-------|-----------------|
| 传输层 | UDP | UDP |
| 可靠性 | 内置（重传 + 去重） | 自建（PacketSender 重传） |
| 背压 | 显式流控 | 隐式（Bundle 缓冲） |
| 零拷贝 | 内存映射文件 | 无（标准内存拷贝） |
| 多播 | 原生支持 | 无 |
| 定位 | 通用消息总线 | 游戏专用网络层 |

### 可以借鉴的

1. **零拷贝**：Aeron 通过内存映射避免内核态拷贝，BigWorld/KBEngine 都没用
2. **背压感知**：当消费者跟不上时，Aeron 显式通知生产者减速，游戏服务器通常只是丢包
3. **多播**：AOI 广播场景天然适合多播，但两套项目都没用

---

## C.4 Twisted Deferred：BigWorld 的异步回调模型来源

### 是什么

Twisted 是 Python 的异步网络框架，其 `Deferred` 类是一种管理异步操作回调链的模式。

### Deferred 的核心模型

```python
from twisted.internet import defer

@defer.inlineCallbacks
def do_something():
    try:
        result1 = yield async_operation_1()
        result2 = yield async_operation_2(result1)
        defer.returnValue(result2)
    except Exception as e:
        handle_error(e)

# 等价于回调链：
d = async_operation_1()
d.addCallback(lambda r1: async_operation_2(r1))
d.addCallback(lambda r2: use_result(r2))
d.addErrback(handle_error)
```

### Deferred 的内部结构

```
Deferred:
    callback chain: [cb1, cb2, cb3, ...]
    errback chain:  [eb1, eb2, eb3, ...]
    result:         当前值（初始为 None）

当异步操作完成时：
    → d.callback(result)
    → 依次执行 cb1(result) → cb2(cb1_result) → cb3(cb2_result)
    → 任何一步抛异常 → 跳到 errback chain
```

### BigWorld 的 PyDeferred

```cpp
// BigWorld: lib/entitydef/py_deferred.hpp

class PyDeferred : public PyObjectPlus
{
public:
    PyDeferred();

    // Python 接口
    PyObject* py_addCallback(PyObject* args);   // 添加回调
    PyObject* py_addErrback(PyObject* args);     // 添加错误回调
    PyObject* py_callback(PyObject* args);       // 触发成功
    PyObject* py_errback(PyObject* args);        // 触发失败

private:
    BW::vector<PyObject*> callbacks_;
    BW::vector<PyObject*> errbacks_;
    bool started_;
};
```

### 为什么 BigWorld 需要 Deferred

```
MMO 中的异步操作链：

1. 客户端发起 TwoWay EntityCall
   → d = entity.remoteMethod.twoWay(args)

2. 消息跨进程发送到目标 Entity
   → 目标处理完成，回复

3. 回复到达，Deferred 触发 callback
   → d.addCallback(onResult)
   → onResult 处理结果，可能触发下一个异步操作

4. 任何一步失败，走 errback
   → d.addErrback(onError)
```

### KBEngine 为什么不需要 Deferred

KBEngine 用 `CallbackMgr` 替代：

```python
# KBEngine 的方式
callbackID = KBEngine.addCallback(self.onResult)
entity.remoteMethod(args, callbackID)

# 回调直接执行，不支持链式
def onResult(self, result):
    # 处理结果
    pass
```

### 对比

| 维度 | Twisted Deferred | CallbackMgr |
|------|-----------------|-------------|
| 链式调用 | `d.addCallback().addCallback()` | 无 |
| 错误处理 | 内置 errback 链 | 手动 try/catch |
| 组合性 | 可组合多个异步操作 | 扁平回调 |
| 学习曲线 | 较陡 | 平缓 |
| 代码量 | 更多 | 更少 |
| 适用场景 | 复杂异步编排 | 简单回调 |

**BigWorld 选 Deferred**：因为 MMO 的很多操作是异步链式的（DB 查询 → 验证 → 创建实体 → 同步），Deferred 让这种链式代码更可读。

**KBEngine 选 CallbackMgr**：因为大部分操作不需要链式——发起一个请求，收到结果直接处理。简单场景不需要 Deferred 的复杂性。
