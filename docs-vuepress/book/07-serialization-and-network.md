# 7. 序列化、Bundle 与网络消息系统

> 本文件是旧稿，内容已拆分并扩写到 [`10-serialization-bundle-and-messages.md`](./10-serialization-bundle-and-messages.md)。阅读主线请以新稿为准。

> 这是理解 KBEngine 设计取舍的第二个关键章节。很多机制最终都会落到这一层，所以这一章不是“网络实现细节”，而是“为什么上层会长成那样”。

## 9.1 本章核心问题

- 为什么是 `Bundle / MemoryStream / MessageHandler` 这一套？
- 序列化设计想解决什么问题？
- 这套设计的优势、代价和包袱分别是什么？

## 9.2 先把三层职责分开

如果只看名字，很容易把 `Bundle` 和 `MemoryStream` 混成一层。实际上它们职责不同：

- `MemoryStream`
  面向数据编码，负责把参数、属性、实体状态写成连续字节流
- `Bundle`
  面向消息拼装，负责把一个或多个消息头 + 消息体组织进待发送 packet
- `MessageHandler / MessageHandlers`
  面向协议分发，负责“某个消息 ID 应该交给谁处理”

所以一次完整消息通常是：

1. 上层先把业务参数写进 `MemoryStream`
2. 再由 `Bundle::newMessage(...)` 写入消息头
3. 最后接收侧由 `PacketReader` 拆包，再按 `MessageHandler` 分发

## 9.3 为什么它不用“通用结构体序列化”而偏向手工流编码

这套设计背后的核心诉求不是优雅，而是：

- 高吞吐
- 小包体
- 可控的协议布局
- 能直接服务于实体同步和远程调用

从前面几章已经能看到，KBEngine 很多关键路径都需要非常明确的字节布局：

- 远程方法调用
- 属性更新
- AOI 进入 / 离开
- Base / Cell 跨进程状态交接
- DB 持久化流

如果每次都走一层高抽象通用协议，会让：

- 包更大
- 控制更弱
- 调优更难

所以它选的是更“引擎式”的方案：大量手工 `<<`、`>>`，大量基于 `Utype / alias` 的短字段编码。

## 9.4 `Bundle` 真正在做什么

源码入口：

- `kbe/src/lib/network/bundle.cpp`
  `Bundle::newMessage`
  `Bundle::finiMessage`

`Bundle::newMessage(const MessageHandler& msgHandler)` 的核心动作非常清楚：

- 记录当前消息处理器 `pCurrMsgHandler_`
- 确保当前 packet 可写
- 先结束上一个消息 `finiMessage(false)`
- 写入当前消息 ID
- 若是变长消息，则预留长度字段
- 记录当前消息长度信息

也就是说，`Bundle` 的核心不是“存业务对象”，而是：

- 在一个发送缓冲里连续拼多条协议消息
- 正确维护每条消息的 ID、长度、packet 边界

`finiMessage` 则负责补齐变长消息长度、做统计、把当前 packet 归档。

这说明 KBEngine 不是“一个调用 = 一个包”的思路，而是：

- 一个 bundle 里可以拼多个消息
- 真正的网络发送单位和业务调用单位并不严格一一对应

这对高频同步非常重要。

## 9.5 `MessageHandler` 解决的是“消息如何落地”

源码入口：

- `kbe/src/lib/network/message_handler.h`
- 各组件的 `*_interface.h`
- 各组件的 `*_interface_macros.h`

`MessageHandler` 里至少包含：

- `name`
- `msgID`
- `msgLen`
- `pArgs`
- `handle(...)`

`MessageHandlers` 则维护：

- 组件级消息表
- `msgID -> handler` 的查找关系

这意味着 KBEngine 的消息系统本质上是：

- 每个组件维护自己的一套接口表
- 每个接口在初始化阶段获得稳定的消息 ID
- 收包时按消息 ID 找处理器，再把流参数交给它

所以所谓 `BaseappInterface::onWriteToDBCallback` 这些“接口”，本质不是普通函数，而是：

- 一组静态注册好的消息描述与 handler

## 9.6 `PacketReader` 如何拆包和防御异常包

源码入口：

- `kbe/src/lib/network/packet_reader.cpp`
  `PacketReader::processMessages`

这段代码是理解整个收包模型的关键。它在循环里做了这些事：

1. 先读 `msgID`
2. 通过 `pMsgHandlers->find(currMsgID_)` 找 handler
3. 再判断消息是定长还是变长
4. 若是变长，继续读长度字段
5. 若包体未收全，则进入 fragment 合并逻辑
6. 收全后调用 `pMsgHandler->handle(pChannel_, *pPacket)`
7. 最后检查 handler 是否把流读完

这里的几个设计点非常重要：

- 未知消息 ID 会直接 `condemn` 通道
- 超长消息会被拒绝
- 半包会进入 fragment 流程，而不是假设一次收全
- handler 读偏了还会告警

这说明 KBEngine 的网络层并不是“裸流随便读”，而是有比较强的协议边界防御。

## 9.7 `MemoryStream` 为什么贯穿了 RPC、同步、持久化三条主线

如果只把 `MemoryStream` 理解为“一个字节数组”，会低估它的重要性。实际上它是 KBEngine 的统一中间表示：

- RPC 参数序列化进它
- 属性同步数据写进它
- Base / Cell 状态交接写进它
- 持久化字段写进它
- DB 查询回包也用它恢复对象

这意味着 KBEngine 在协议层做了一件很统一的事：

- 不试图区分“网络数据流”和“持久化数据流”的编码体系
- 尽量复用同一种对象级序列化机制

这样做的好处是：

- 同一套 `DataType / MethodDescription / PropertyDescription` 可以复用
- 上层逻辑更容易做流转

代价是：

- 流格式和引擎内部结构绑定很深
- 调试时你经常看到的是字节流，不是高层对象

## 9.8 这套消息系统为什么特别适合实体同步

以客户端属性同步为例，系统并不是发送“字段名 + JSON 值”，而是更偏底层：

- 先发消息 ID
- 再发实体 ID 或 alias
- 再发属性 `Utype / alias`
- 最后发编码后的值

在一些路径里还会借助 `FixedMessages` 做进一步压缩，例如位置、朝向、`spaceID` 等高频字段会有固定消息优化。

这说明 KBEngine 的网络层不是为“业务服务接口”优先设计的，而是为：

- 高频状态同步
- 大量小消息
- 实体视野广播

优先设计的。

## 9.9 一个建议的源码走读路径

如果要真正把这一层看懂，我建议按下面顺序：

1. `kbe/src/lib/network/message_handler.h`
   先看消息表与 handler 抽象
2. 任一组件接口头
   例如 `baseapp_interface.h`
3. `kbe/src/lib/network/bundle.cpp`
   看 `newMessage / finiMessage`
4. `kbe/src/lib/network/packet_reader.cpp`
   看 `processMessages`
5. 再回到上层某条具体链
   例如 `EntityCall` 或 `writeToDB`

只有这样你才会建立正确心智模型：

- 上层不是直接“调函数”
- 而是在不停地往 `MemoryStream + Bundle` 写结构化字节

## 9.10 这套设计的优势与包袱

### 优势

- 性能可控，适合引擎级高频消息
- 包体紧凑，适合 MMO 大量同步
- 消息边界清晰，便于统计与防御
- 与实体定义体系天然兼容

### 包袱

- 可读性差，很多逻辑要对着字节流看
- 手工流编码较容易出错
- 版本演进和协议兼容性维护成本高
- 新人上手会觉得“到处都在 `<<` / `>>`”

## 9.11 本章小结

理解 KBEngine 的序列化与网络系统，最重要的是记住：

- `MemoryStream` 是统一的数据编码层
- `Bundle` 是消息拼装层
- `MessageHandler` 是消息分发表
- `PacketReader` 是收包与边界校验层

再往上一层看，RPC、属性同步、AOI、持久化并不是四套完全不同的机制，而是都建立在这一套流式协议基座上。
