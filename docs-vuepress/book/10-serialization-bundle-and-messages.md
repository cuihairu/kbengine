# 10. 序列化、Bundle 与网络消息

> 这一章回答：为什么不用 Protobuf 而用手工流编码？MemoryStream 为什么贯穿整个系统？Bundle 真正在做什么？消息是怎么从"一串字节"变成"一次函数调用"的？

## 10.1 本章核心问题

- MemoryStream / BinaryOStream 的设计为什么是手工流编码而不是结构体序列化？
- Bundle 的 newMessage / finiMessage 做了什么？为什么一个 Bundle 能装多条消息？
- Packet 的自动分包怎么实现？
- PacketReader 的半包状态机怎么工作？
- MessageHandler / InterfaceTable 的消息路由怎么实现？
- 坐标压缩（PackXZ / PackY / PackXYZ）为什么省带宽？

## 10.2 为什么不用 Protobuf / FlatBuffers

游戏服务器不用通用序列化框架，原因很具体：

| 维度 | Protobuf / FlatBuffers | 手工流编码（MemoryStream） |
|------|----------------------|--------------------------|
| 编码大小 | 有 schema 元数据开销 | 裸二进制，零开销 |
| 编码速度 | 需要编解码步骤 | 直接内存拷贝 |
| 布局控制 | schema 驱动，不透明 | 完全可控，精确到字节 |
| 流式处理 | 不支持（需要完整消息） | 天然支持（rpos/wpos 游标） |
| 属性同步 | 无法做"只发变更字段" | 按需序列化脏属性 |
| 坐标压缩 | 需要自定义类型 | PackXZ 3 字节编码一个 float |

**核心原因**：MMO 服务器的网络消息不是"序列化一个结构体"，而是：

1. **属性同步**：tick 末只序列化变更的属性，需要精确控制哪些字段进入流
2. **RPC 调用**：参数类型由 EntityDef 的 MethodDescription 在运行时决定，不是编译期 schema
3. **分包/组包**：消息可能跨多个 Packet，需要流式读写游标
4. **带宽敏感**：一个 CellApp 可能有 2000 个客户端，每 tick 同步，每字节都重要

## 10.3 KBEngine MemoryStream：贯穿三条主线的二进制流

### 类结构

```cpp
// 文件：kbe/src/lib/common/memorystream.h（简化）
class MemoryStream : public PoolObject
{
public:
    const static size_t DEFAULT_SIZE = 0x100;
    const static size_t MAX_SIZE = 10000000;

    MemoryStream(): rpos_(0), wpos_(0)
    {
        data_.reserve(DEFAULT_SIZE);   // 默认 256 字节
    }

    // 写入：operator<< 重载
    MemoryStream &operator<<(uint8 value);
    MemoryStream &operator<<(uint16 value);
    MemoryStream &operator<<(uint32 value);
    MemoryStream &operator<<(uint64 value);
    MemoryStream &operator<<(float value);
    MemoryStream &operator<<(const std::string &value);  // null-terminated

    // 读取：operator>> 重载
    MemoryStream &operator>>(uint8 &value);
    MemoryStream &operator>>(uint16 &value);
    // ...

    // 底层操作
    template <typename T> void append(T value)
    {
        EndianConvert(value);       // 字节序转换
        append((uint8 *)&value, sizeof(value));
    }

protected:
    std::vector<uint8> data_;   // 缓冲区
    size_t rpos_;               // 读游标
    size_t wpos_;               // 写游标
};
```

**关键设计决策**：

1. **PoolObject 继承**：MemoryStream 继承自对象池基类，高频分配/释放通过对象池复用
2. **vector 缓冲区**：自动扩容，`reserve(DEFAULT_SIZE)` 预分配
3. **双游标**：rpos/wpos 分别追踪读写位置，支持流式操作
4. **字节序转换**：`EndianConvert` 确保跨平台一致
5. **operator<</>>**：类型安全的流式读写接口

### MemoryStream 贯穿的三条主线

```
MemoryStream 的使用场景：

1. RPC 参数序列化
   Python: entity.cell.onDamage(100)
     → MethodDescription::addToStream(mstream, args)
     → mstream << (int32)100

2. 属性同步
   Entity::addDirtyPropToBundle()
     → PropertyDescription::addToStream(mstream, value)
     → mstream << entity.health

3. 持久化
   EntityTable::addPersistentsDataToStream()
     → mstream << playerName << level << ...

4. DB 查询
   DBTask::execute() → mstream 传参 → 结果也写回 mstream

5. Base-Cell 交接
   Base::giveEntityToCell() → 序列化完整实体状态到 mstream
```

### 坐标压缩

MemoryStream 内置了三种坐标压缩方法，直接减少网络带宽：

```cpp
// 文件：kbe/src/lib/common/memorystream.h（简化）

// PackXZ：float x,z → 3 字节（从 8 字节压缩到 3 字节，省 62.5%）
void appendPackXZ(float x, float z)
{
    PackFloatXType xPackData;
    xPackData.fv = x;
    PackFloatXType zPackData;
    zPackData.fv = z;
    // ... 提取尾数位，打包到 3 字节 ...
    uint8 packs[3];
    packs[0] = (uint8)(data >> 16);
    packs[1] = (uint8)(data >> 8);
    packs[2] = (uint8)data;
    (*this).append(packs, 3);
}

// PackY：float y → 2 字节（从 4 字节压缩到 2 字节，省 50%）
void appendPackY(float y);

// PackXYZ：3 个 float → 4 字节（从 12 字节压缩到 4 字节，省 66.7%）
void appendPackXYZ(float x, float y, float z, float minf = -256.f)
{
    x -= minf;
    y -= minf / 2.f;
    z -= minf;
    uint32 packed = 0;
    packed |= ((int)(x / 0.25f) & 0x7FF);          // X: 11 位
    packed |= ((int)(z / 0.25f) & 0x7FF) << 11;     // Z: 11 位
    packed |= ((int)(y / 0.25f) & 0x3FF) << 22;     // Y: 10 位
    *this << packed;
}
```

**压缩原理**：利用浮点数的精度冗余。MMO 中坐标精度到 0.25 单位足够，范围 -256~+256 映射到 11 位整数。这些压缩在 AOI 位置同步中高频使用，每 tick 每 visible 实体都会触发。

## 10.4 BigWorld BinaryOStream / MemoryOStream：抽象流接口

### 两层设计

BigWorld 把流分成了抽象接口和具体实现：

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/cstdmf/binary_stream.hpp（简化）
class BinaryOStream
{
public:
    virtual ~BinaryOStream() {};
    virtual void* reserve(int nBytes) = 0;  // 纯虚：预留空间
    virtual int size() const { return -1; }
    virtual void addBlob(const void* pBlob, int size);

    template <class TYPE>
    void insertRaw(const TYPE& t);           // 模板写入

    void writePackedInt(int len);            // 变长整数编码
    void writeStringLength(int len) { this->writePackedInt(len); }
};

class BinaryIStream
{
public:
    virtual const void* retrieve(int nBytes) = 0;  // 纯虚：取出数据
    virtual int remainingLength() const = 0;
    virtual char peek() = 0;

    int readPackedInt();
    int readStringLength() { return this->readPackedInt(); }
    bool error() const;
};
```

**与 KBEngine 的关键区别**：

1. **抽象接口**：BinaryOStream/BinaryIStream 是纯虚类，KBEngine 的 MemoryStream 是具体类
2. **writePackedInt**：BigWorld 有变长整数编码（类似 VarInt），KBEngine 没有
3. **双向流**：MemoryOStream 同时继承 BinaryOStream 和 BinaryIStream，可读可写

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/cstdmf/memory_stream.hpp（简化）
class MemoryOStream : public BinaryOStream, public BinaryIStream,
    public SafeAllocatable
{
public:
    MemoryOStream(int size = 64);

    // BinaryOStream overrides
    virtual void* reserve(int nBytes);
    virtual int size() const;

    // BinaryIStream overrides
    virtual const void* retrieve(int nBytes);
    virtual int remainingLength() const;

protected:
    char* pBegin_;    // 缓冲区起始
    char* pCurr_;     // 当前写入位置
    char* pEnd_;      // 缓冲区末尾
    char* pRead_;     // 当前读取位置
};
```

**对比**：

| 维度 | KBEngine MemoryStream | BigWorld MemoryOStream |
|------|----------------------|----------------------|
| 继承 | PoolObject（对象池） | SafeAllocatable |
| 缓冲区 | `vector<uint8>` + rpos/wpos | 裸指针 pBegin/pCurr/pEnd/pRead |
| 接口 | 具体 class | 虚基类 BinaryOStream/BinaryIStream |
| 变长编码 | 无 | writePackedInt / readPackedInt |
| 扩容 | vector 自动扩容 | grow() 手动 realloc |
| 坐标压缩 | PackXZ/PackY/PackXYZ 内置 | 无内置 |

## 10.5 Bundle：消息拼装器

### KBEngine Bundle

Bundle 的核心职责：**把多条消息拼装成一组 Packet，管理消息边界和自动分包**。

```cpp
// 文件：kbe/src/lib/network/bundle.h（简化）
class Bundle : public PoolObject
{
public:
    typedef std::vector<Packet*> Packets;

    Bundle(Channel* pChannel = NULL, ProtocolType pt = PROTOCOL_TCP);

    // 消息边界管理
    void newMessage(const MessageHandler& msgHandler);  // 开始新消息
    void finiMessage(bool isSend = true);               // 结束当前消息

    // 数据写入（自动分包）
    Bundle& operator<<(uint8 value)
    {
        onPacketAppend(sizeof(uint8));     // 检查是否需要新 Packet
        (*pCurrPacket_) << value;          // 写入当前 Packet
        return *this;
    }
    // ... 所有基本类型的 operator<< 重载

    Packet* newPacket();
    int32 packetsLength(bool calccurr = true);

protected:
    int32 onPacketAppend(int32 addsize, bool inseparable = true);

    Packets packets_;               // 已完成的 Packet 列表
    Packet* pCurrPacket_;           // 正在写入的 Packet
    Channel* pChannel_;             // 关联的 Channel
    const MessageHandler* pCurrMsgHandler_;

    MessageID currMsgID_;           // 当前消息 ID
    MessageLength1 currMsgLength_;  // 当前消息长度
    size_t currMsgLengthPos_;       // 长度字段在流中的位置（变长消息用）
    int32 numMessages_;             // Bundle 内消息数
    int32 packetMaxSize_;           // 单 Packet 最大大小
};
```

### newMessage / finiMessage 的完整流程

```cpp
// 文件：kbe/src/lib/network/bundle.cpp:273（简化）
void Bundle::newMessage(const MessageHandler& msgHandler)
{
    pCurrMsgHandler_ = &msgHandler;

    // 如果没有当前 Packet，尝试复用上一个 Packet 的剩余空间
    if (pCurrPacket_ == NULL)
    {
        if (packets_.size() > 0 && packetHaveSpace())
        {
            pCurrPacket_ = packets_.back();    // 复用
            packets_.pop_back();
        }
        else
        {
            this->newPacket();                  // 新建
        }
    }

    finiMessage(false);  // 结束上一条消息（如果有的话）

    // 写入消息 ID
    (*this) << msgHandler.msgID;
    pCurrPacket_->messageID(msgHandler.msgID);

    // 变长消息：预留长度字段
    if (msgHandler.msgLen == NETWORK_VARIABLE_MESSAGE)
    {
        MessageLength msglen = 0;
        currMsgLengthPos_ = pCurrPacket_->wpos();  // 记住位置，finiMessage 时回填
        (*this) << msglen;
    }

    ++numMessages_;
    currMsgID_ = msgHandler.msgID;
}
```

```
Bundle 的消息生命周期：

newMessage(handler1)
  │ 写入 msgID (2 bytes)
  │ 变长消息写入 length 占位 (2 bytes)
  │
  ├── operator<< 写入参数数据
  │     onPacketAppend() 检查空间
  │     空间不足 → 当前 Packet 入 packets_ → 新建 Packet → 继续写
  │
finiMessage()
  │ 回填变长消息的 length 字段
  │ 当前 Packet 关联到 Bundle
  │ 记录统计信息
  │
newMessage(handler2)      ← 可以继续在同一个 Bundle 里拼第二条消息
  │ 写入 msgID
  │ ...
finiMessage()
  │
Channel::send(bundle)     ← 整个 Bundle 的所有 Packet 一起发送
```

### 自动分包：onPacketAppend

```cpp
// 文件：kbe/src/lib/network/bundle.cpp:177（简化）
int32 Bundle::onPacketAppend(int32 addsize, bool inseparable)
{
    if (pCurrPacket_ == NULL)
        newPacket();

    int32 totalsize = (int32)pCurrPacket_->length();
    int32 fwpos = (int32)pCurrPacket_->wpos();

    if (inseparable)
        fwpos += addsize;        // 不可分割的数据，确保完整放入

    // 当前 Packet 放不下了
    if (fwpos >= packetMaxSize_)
    {
        packets_.push_back(pCurrPacket_);   // 当前 Packet 完成
        currMsgPacketCount_++;
        newPacket();                         // 创建新 Packet
        totalsize = 0;
    }

    int32 remainsize = packetMaxSize_ - totalsize;
    int32 taddsize = addsize;

    // 剩余空间不够，先填满当前 Packet
    if (remainsize < addsize)
        taddsize = remainsize;

    currMsgLength_ += taddsize;             // 累加消息长度
    return taddsize;
}
```

**关键**：`inseparable=true` 时，如果当前 Packet 剩余空间放不下这整块数据，直接开新 Packet。这保证了消息头（msgID + length）不会跨 Packet。

### BigWorld Bundle

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/network/bundle.hpp（简化）
enum ReliableTypeEnum
{
    RELIABLE_NO       = 0,   // 完全不可靠（位置更新）
    RELIABLE_DRIVER   = 1,   // 可靠驱动消息
    RELIABLE_PASSENGER = 2,  // 搭便车（随驱动消息一起可靠传输）
    RELIABLE_CRITICAL  = 3   // 关键消息（必须可靠）
};

class Bundle : public BinaryOStream
{
public:
    // 三种消息启动方式
    virtual void startMessage(const InterfaceElement& ie,
        ReliableType reliable = RELIABLE_DRIVER) = 0;

    virtual void startRequest(const InterfaceElement& ie,
        ReplyMessageHandler* handler,
        void* arg = NULL,
        int timeout = DEFAULT_REQUEST_TIMEOUT,
        ReliableType reliable = RELIABLE_DRIVER) = 0;

    virtual void startReply(ReplyID id,
        ReliableType reliable = RELIABLE_DRIVER) = 0;

    void finalise();   // 定稿：写 footer、checksum 等

    // 取消请求
    void cancelRequests(RequestManager* pRequestManager, Reason reason);

protected:
    Channel* pChannel_;
    bool isFinalised_;
    int numMessages_;
    ReplyOrders replyOrders_;   // 请求-回复的回调队列
};
```

**与 KBEngine 的关键区别**：

1. **startMessage / startRequest / startReply**：BigWorld 区分三种消息类型——单向消息、请求（期望回复）、回复。KBEngine 只有 newMessage/finiMessage（纯单向）
2. **ReliableType 参数**：每条消息可以指定可靠性级别。KBEngine 用 TCP，所有消息天然可靠，不需要这个
3. **finalise()**：BigWorld 有明确的定稿步骤，会写入 Packet footer（ACK、序列号、校验和等）
4. **ReplyOrders**：BigWorld 的 Bundle 内置请求-回复追踪，KBEngine 由外层 CallbackMgr 管理

### UDPBundle：BigWorld 的 UDP 特化

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/network/udp_bundle.hpp（简化）
class UDPBundle : public Bundle
{
public:
    UDPBundle(uint8 spareSize = 0, UDPChannel* pChannel = NULL);

    // Bundle overrides
    virtual void startMessage(const InterfaceElement& ie,
        ReliableType reliable = RELIABLE_DRIVER);
    virtual void startRequest(const InterfaceElement& ie,
        ReplyMessageHandler* handler, void* arg = NULL,
        int timeout = DEFAULT_REQUEST_TIMEOUT,
        ReliableType reliable = RELIABLE_DRIVER);
    virtual void startReply(ReplyID id, ReliableType reliable = RELIABLE_DRIVER);

    // BinaryOStream overrides
    virtual void* reserve(int nBytes);
    virtual void addBlob(const void* pBlob, int size);

    virtual void doFinalise();   // 写 UDP footer
};
```

## 10.6 Packet：网络传输单元

### KBEngine Packet

```cpp
// 文件：kbe/src/lib/network/packet.h（简化）
class Packet : public MemoryStream, public RefCountable
{
public:
    Packet(MessageID msgID = 0, bool isTCPPacket = true, size_t res = 200):
        MemoryStream(res),
        msgID_(msgID),
        isTCPPacket_(isTCPPacket),
        encrypted_(false),
        pBundle_(NULL),
        sentSize(0)
    {}

    virtual void onReclaimObject()    // 对象池回收时重置
    {
        MemoryStream::onReclaimObject();
        resetPacket();
    }

    virtual int recvFromEndPoint(EndPoint& ep, Address* pAddr = NULL) = 0;

protected:
    MessageID msgID_;
    bool isTCPPacket_;
    bool encrypted_;
    Bundle* pBundle_;     // 所属的 Bundle

public:
    uint32 sentSize;      // 已发送字节数
};

typedef SmartPointer<Packet> PacketPtr;
```

**Packet = MemoryStream + RefCountable + 网络元数据**。它既是二进制流（继承 MemoryStream），又是网络传输单元（有 msgID、sentSize），还通过引用计数管理生命周期。

TCP 和 UDP 有各自子类：

```cpp
// TCPPacket 和 UDPPacket 主要区别在 recvFromEndPoint 实现
// TCP：从 socket 读取到缓冲区
// UDP：recvfrom 获取一个完整数据报
```

**Packet 最大尺寸**：

```cpp
// 文件：kbe/src/lib/network/common.h
PACKET_MAX_SIZE_TCP = 1460;    // TCP MSS 1500 - 40(IP+TCP header)
PACKET_MAX_SIZE_UDP = 1472;    // UDP 1500 - 28(IP+UDP header)
```

### BigWorld Packet

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/network/packet.hpp（简化）
typedef uint16 Flags;

enum
{
    FLAG_HAS_REQUESTS         = 0x0001,
    FLAG_HAS_PIGGYBACKS       = 0x0002,
    FLAG_HAS_ACKS             = 0x0004,
    FLAG_ON_CHANNEL           = 0x0008,
    FLAG_IS_RELIABLE          = 0x0010,
    FLAG_IS_FRAGMENT          = 0x0020,
    FLAG_HAS_SEQUENCE_NUMBER  = 0x0040,
    FLAG_INDEXED_CHANNEL      = 0x0080,
    FLAG_HAS_CHECKSUM         = 0x0100,
    FLAG_CREATE_CHANNEL       = 0x0200,
    FLAG_HAS_CUMULATIVE_ACK   = 0x0400,
};

class Packet : public ReferenceCount
{
public:
    static const int HEADER_SIZE = sizeof(Flags);  // 2 字节 flags 头
    static const int RESERVED_FOOTER_SIZE = 27;    // 预留 footer 空间
    static const int MAX_SIZE = 1472;              // UDP MTU

    char* data() { return data_; }
    const char* body() const { return data_ + HEADER_SIZE; }

    Flags flags() const { return BW_NTOHS(*(Flags*)data_); }
    void setFlags(Flags flags) { *(Flags*)data_ = BW_HTONS(flags); }

    void chain(Packet* pPacket) { next_ = pPacket; }  // 链表连接（分片）
    int freeSpace() const
    {
        return MAX_SIZE - RESERVED_FOOTER_SIZE
            - msgEndOffset_ - footerSize_ - extraFilterSize_;
    }

private:
    char data_[MAX_SIZE];        // 固定大小缓冲区
    PacketPtr next_;             // 链表下一个（分片）
    int msgEndOffset_;           // 消息数据末尾
    int footerSize_;             // footer 大小
    int extraFilterSize_;        // 过滤器预留
    bool isPiggyback_;           // 是否搭便车
    AckCount nAcks_;             // ACK 数量
    SeqNum seq_;                 // 序列号
};
```

**BigWorld Packet 比 KBEngine 复杂得多**：

1. **Flags 头部**：2 字节标志位，描述 Packet 内包含哪些数据（请求/ACK/分片/校验和等）
2. **预分配固定大小**：`char data_[1472]`，不像 KBEngine 用 vector 动态扩容
3. **链表分片**：`next_` 指针连接分片 Packet，KBEngine 用 vector 管理
4. **Footer 预留**：27 字节 footer 空间预分配（ACK、序列号、校验和等）
5. **Piggyback**：小消息可以"搭便车"附在其他 Packet 上
6. **校验和**：`FLAG_HAS_CHECKSUM` 启用数据校验

**对比**：

| 维度 | KBEngine Packet | BigWorld Packet |
|------|----------------|-----------------|
| 继承 | MemoryStream + RefCountable | ReferenceCount |
| 缓冲区 | vector（动态） | char[1472]（固定） |
| 头部 | 无特殊头部 | 2 字节 Flags |
| Footer | 无 | 预留 27 字节（ACK/seq/checksum） |
| 分片连接 | Bundle 的 vector 管理 | 链表 next_ |
| 可靠性 | TCP 天然可靠 | Flags 中的 IS_RELIABLE |
| Piggyback | 无 | FLAG_HAS_PIGGYBACKS |
| 校验和 | 无 | FLAG_HAS_CHECKSUM |

## 10.7 PacketReader：半包状态机

网络层收到的原始字节流需要解析成完整的消息。TCP 是字节流协议，消息边界不保证对齐——一次 recv 可能包含半条消息、一条完整消息、或一条半消息。

### KBEngine PacketReader

```cpp
// 文件：kbe/src/lib/network/packet_reader.h（简化）
class PacketReader
{
public:
    PacketReader(Channel* pChannel);
    virtual void processMessages(MessageHandlers* pMsgHandlers, Packet* pPacket);

protected:
    // 半包状态的 5 种类型
    enum FragmentDataTypes
    {
        FRAGMENT_DATA_UNKNOW,           // 未知（初始状态）
        FRAGMENT_DATA_MESSAGE_ID,       // 正在接收消息 ID
        FRAGMENT_DATA_MESSAGE_LENGTH,   // 正在接收消息长度
        FRAGMENT_DATA_MESSAGE_LENGTH1,  // 扩展长度（4 字节）
        FRAGMENT_DATA_MESSAGE_BODY      // 正在接收消息体
    };

    virtual void writeFragmentMessage(FragmentDataTypes flag,
        Packet* pPacket, uint32 datasize);
    virtual void mergeFragmentMessage(Packet* pPacket);

    uint8* pFragmentDatas_;           // 半包数据缓冲区
    uint32 pFragmentDatasWpos_;
    uint32 pFragmentDatasRemain_;
    FragmentDataTypes fragmentDatasFlag_;  // 当前半包状态
    MemoryStream* pFragmentStream_;

    MessageID currMsgID_;
    MessageLength1 currMsgLen_;
    Channel* pChannel_;
};
```

### processMessages 的状态机

```cpp
// 文件：kbe/src/lib/network/packet_reader.cpp:48（简化）
void PacketReader::processMessages(MessageHandlers* pMsgHandlers, Packet* pPacket)
{
    while (pPacket->length() > 0 || pFragmentStream_ != NULL)
    {
        if (fragmentDatasFlag_ == FRAGMENT_DATA_UNKNOW)
        {
            // 状态 1：读取消息 ID
            if (currMsgID_ == 0)
            {
                if (pPacket->length() < NETWORK_MESSAGE_ID_SIZE)
                {
                    // 半包：ID 都不完整，等下一个 Packet
                    writeFragmentMessage(FRAGMENT_DATA_MESSAGE_ID,
                        pPacket, NETWORK_MESSAGE_ID_SIZE);
                    break;
                }
                (*pPacket) >> currMsgID_;
            }

            // 查找消息处理器
            MessageHandler* pMsgHandler = pMsgHandlers->find(currMsgID_);

            if (pMsgHandler == NULL)
            {
                // 未知消息 ID → condemn（谴责）通道
                pChannel_->condemn("not found msgID");
                break;
            }

            // 状态 2：读取消息长度
            if (currMsgLen_ == 0)
            {
                if (pMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE)
                {
                    // 变长消息：从流中读长度字段
                    if (pPacket->length() < NETWORK_MESSAGE_LENGTH_SIZE)
                    {
                        writeFragmentMessage(FRAGMENT_DATA_MESSAGE_LENGTH,
                            pPacket, NETWORK_MESSAGE_LENGTH_SIZE);
                        break;
                    }
                    (*pPacket) >> currMsgLen_;
                }
                else
                {
                    // 定长消息：长度从 handler 获取
                    currMsgLen_ = pMsgHandler->msgLen;
                }
            }

            // 状态 3：读取消息体
            if (pPacket->length() < currMsgLen_)
            {
                // 半包：消息体不完整
                writeFragmentMessage(FRAGMENT_DATA_MESSAGE_BODY,
                    pPacket, currMsgLen_);
                break;
            }

            // 完整消息，交给 handler 处理
            pMsgHandler->handle(pChannel_, *pPacket);

            // 重置状态，处理下一条消息
            currMsgID_ = 0;
            currMsgLen_ = 0;
        }
        else
        {
            // 有未完成的半包，先合并
            mergeFragmentMessage(pPacket);
        }
    }
}
```

**状态机流转**：

```
收到 Packet
  │
  ├── 有未完成半包？
  │     YES → mergeFragmentMessage() → 合并后重新进入主流程
  │
  ├── 读取 MessageID（2 bytes）
  │     不够 → writeFragmentMessage(ID) → 等下一个 Packet
  │
  ├── 查找 MessageHandler
  │     找不到 → condemn 通道（防御措施）
  │
  ├── 读取 MessageLength（变长消息）
  │     不够 → writeFragmentMessage(LENGTH) → 等下一个 Packet
  │
  ├── 读取 MessageBody
  │     不够 → writeFragmentMessage(BODY) → 等下一个 Packet
  │
  └── 完整 → handler->handle() → 重置状态 → 处理下一条消息
```

**condemn 通道**：如果收到未知 MessageID，直接谴责（关闭）通道。这是防御措施——正常通信不可能出现未知 ID，出现意味着协议不匹配或恶意数据。

## 10.8 MessageHandler / InterfaceTable：消息路由

### KBEngine MessageHandlers

```cpp
// 文件：kbe/src/lib/network/message_handler.h（简化）
class MessageHandler
{
public:
    std::string name;
    MessageID msgID;
    MessageArgs* pArgs;
    int32 msgLen;                    // -1 = 变长消息
    bool exposed;                    // 是否客户端可调用
    MessageHandlers* pMessageHandlers;

    // 统计
    volatile mutable uint32 send_size, send_count;
    volatile mutable uint32 recv_size, recv_count;

    virtual void handle(Channel* pChannel, MemoryStream& s)
    {
        pArgs->createFromStream(s);  // 反序列化参数
    }
};

class MessageHandlers
{
public:
    typedef std::map<MessageID, MessageHandler*> MessageHandlerMap;

    MessageHandler* find(MessageID msgID);   // msgID → handler 查找

private:
    MessageHandlerMap msgHandlers_;          // map 存储
    MessageID msgID_;                        // 下一个可分配的 msgID
};
```

**消息路由 = map 查找**：`msgHandlers_[msgID]` 返回对应的 MessageHandler，调用其 `handle()` 方法。组件启动时通过宏注册所有消息处理器。

### BigWorld InterfaceTable + InterfaceElement

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/network/interface_element.hpp（简化）
const char FIXED_LENGTH_MESSAGE    = 0;   // 定长消息
const char VARIABLE_LENGTH_MESSAGE = 1;   // 变长消息
const char CALLBACK_LENGTH_MESSAGE = 2;   // 回调决定长度

class InterfaceElement
{
public:
    InterfaceElement(const char* name = "", MessageID id = 0,
        int8 lengthStyle = INVALID_MESSAGE, int lengthParam = 0,
        InputMessageHandler* pHandler = NULL);

    int headerSize() const;
    int nominalBodySize() const;
    int compressLength(void* header, int length,
        UDPBundle* pBundle, bool isRequest) const;
    int expandLength(void* header, Packet* pPacket, bool isRequest) const;

    MessageID id() const { return id_; }
    int8 lengthStyle() const { return lengthStyle_; }

protected:
    MessageID id_;
    int8 lengthStyle_;
    int32 lengthParam_;
    const char* name_;
    InputMessageHandler* pHandler_;
};
```

```cpp
// 文件：BigWorld-Engine-14.4.1/programming/bigworld/lib/network/interface_table.hpp（简化）
class InterfaceTable : public TimerHandler
{
public:
    InterfaceTable(EventDispatcher& dispatcher);

    void serve(const InterfaceElement& ie, InputMessageHandler* pHandler);
    InterfaceElementWithStats& operator[](int id);

    const char* msgName(MessageID msgID) const { return table_[msgID].name(); }

private:
    typedef BW::vector<InterfaceElementWithStats> Table;
    Table table_;    // vector，msgID 直接作为索引
};
```

**关键区别**：

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 存储结构 | `std::map<MessageID, Handler*>` | `vector<InterfaceElement>`，ID 直接索引 |
| 查找效率 | O(log n) | O(1) |
| 长度风格 | msgLen == -1 表示变长 | lengthStyle: FIXED / VARIABLE / CALLBACK |
| 统计 | 内置在 MessageHandler | InterfaceElementWithStats 子类 |
| 注册 | 宏 + add() | serve() |
| 与注册中心集成 | 无 | registerWithMachined() |

BigWorld 用 vector + 直接索引更高效（消息 ID 从 0 连续分配），但要求 ID 空间紧凑。

## 10.9 FixedMessages：外部协议的固定约定

KBEngine 有一个 `FixedMessages` 机制，用于与不使用 KBE 内部协议的外部客户端约定固定的消息 ID：

```cpp
// 文件：kbe/src/lib/network/fixed_messages.h（简化）
class FixedMessages : public Singleton<FixedMessages>
{
public:
    struct MSGInfo
    {
        MessageID msgid;
        std::string msgname;
    };

    bool loadConfig(std::string fileName, bool notFoundError = true);
    MSGInfo* isFixed(const char* msgName);

private:
    typedef KBEUnordered_map<std::string, MSGInfo> MSGINFO_MAP;
    MSGINFO_MAP _infomap;
};
```

外部客户端（如 Unity/Unreal）通过 XML 配置文件固定消息 ID，不需要依赖 KBE 的自动 ID 分配机制。这确保了前后端协议兼容性。

## 10.10 两套项目的消息系统架构对比

| 维度 | KBEngine | BigWorld |
|------|----------|----------|
| 序列化基类 | MemoryStream（具体类） | BinaryOStream/BinaryIStream（虚基类） |
| 缓冲区 | vector 动态 | 指针 / 固定数组 |
| 消息拼装 | Bundle::newMessage/finiMessage | Bundle::startMessage/startRequest/startReply |
| 可靠性 | TCP 天然可靠 | ReliableType 四级 |
| Packet 继承 | MemoryStream + RefCountable | ReferenceCount |
| Packet 大小 | 动态（vector） | 固定 1472 字节 |
| Packet 头部 | 无特殊头部 | 2 字节 Flags |
| Packet Footer | 无 | 预留 27 字节 |
| 分片连接 | vector | 链表 |
| 半包处理 | PacketReader 5 态状态机 | PacketReceiver + 有序处理 |
| 消息路由 | map 查找 | vector 直接索引 |
| 变长编码 | 无 | writePackedInt |
| 坐标压缩 | PackXZ/PackY/PackXYZ | 无内置 |
| 外部协议 | FixedMessages XML | InterfaceElement 长度风格 |
| 校验和 | 无 | FLAG_HAS_CHECKSUM |
| 请求-回复 | 无内置（外层 CallbackMgr） | Bundle 内置 ReplyOrders |

## 10.11 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| MemoryStream | `kbe/src/lib/common/memorystream.h` |
| Bundle | `kbe/src/lib/network/bundle.h` |
| Bundle 实现 | `kbe/src/lib/network/bundle.cpp` |
| Packet | `kbe/src/lib/network/packet.h` |
| TCPPacket | `kbe/src/lib/network/tcp_packet.h` |
| UDPPacket | `kbe/src/lib/network/udp_packet.h` |
| PacketReader | `kbe/src/lib/network/packet_reader.h` |
| PacketReader 实现 | `kbe/src/lib/network/packet_reader.cpp` |
| MessageHandler | `kbe/src/lib/network/message_handler.h` |
| FixedMessages | `kbe/src/lib/network/fixed_messages.h` |
| 网络常量 | `kbe/src/lib/network/common.h` |

### BigWorld

| 概念 | 文件 |
|------|------|
| BinaryOStream/BinaryIStream | `BigWorld-Engine-14.4.1/programming/bigworld/lib/cstdmf/binary_stream.hpp` |
| MemoryOStream | `BigWorld-Engine-14.4.1/programming/bigworld/lib/cstdmf/memory_stream.hpp` |
| Bundle | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/bundle.hpp` |
| UDPBundle | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/udp_bundle.hpp` |
| Packet | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/packet.hpp` |
| PacketReceiver | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/packet_receiver.hpp` |
| InterfaceElement | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/interface_element.hpp` |
| InterfaceTable | `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/interface_table.hpp` |

## 10.12 源码走读路径

### 路径一：跟踪一次完整的消息发送

1. `kbe/src/lib/network/bundle.cpp:273` — `newMessage()` 写入 msgID + 长度占位
2. `kbe/src/lib/network/bundle.cpp:177` — `onPacketAppend()` 自动分包检查
3. `kbe/src/lib/network/bundle.cpp:318` — `finiMessage()` 回填长度 + 记录统计
4. `kbe/src/lib/network/channel.h` — `Channel::send()` 发送 Bundle 的所有 Packet

### 路径二：跟踪消息的接收和分发

1. `kbe/src/lib/network/packet_reader.cpp:48` — `processMessages()` 状态机入口
2. `kbe/src/lib/network/message_handler.h` — `MessageHandlers::find()` 查找 handler
3. `kbe/src/lib/network/message_handler.h:93` — `MessageHandler::handle()` 反序列化并回调

### 路径三：对比 BigWorld 的 UDP Bundle

1. `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/bundle.hpp` — ReliableType 枚举
2. `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/udp_bundle.hpp` — UDPBundle 特化
3. `BigWorld-Engine-14.4.1/programming/bigworld/lib/network/packet.hpp` — Packet flags + footer + 链表

### 路径四：理解坐标压缩

1. `kbe/src/lib/common/memorystream.h` — `appendPackXZ()` / `appendPackY()` / `appendPackXYZ()`
2. `kbe/src/lib/common/memorystream.h` — `readPackXZ()` / `readPackY()` / `readPackXYZ()` 对应的解码

## 10.13 小结

- **手工流编码优于通用序列化**：MMO 需要精确控制布局、按需序列化脏属性、带宽敏感——Protobuf 都做不到
- **MemoryStream 贯穿整个系统**：RPC 参数、属性同步、持久化、DB 查询、Base-Cell 交接全部复用
- **Bundle = 消息拼装器**：管理消息边界、自动分包、一条 Bundle 可拼多条消息
- **PacketReader 是半包状态机**：5 种 fragment 状态处理 TCP 字节流的不对齐问题
- **消息路由是静态 map/vector 查找**：msgID → handler，组件启动时注册完成
- **BigWorld 比 KBEngine 复杂得多**：可靠性分级、Packet flags/footer/校验和、请求-回复内置、piggyback 搭便车、链表分片
- **KBEngine 更简单直接**：TCP 天然可靠，不需要可靠性分级和 footer 机制
- **坐标压缩是带宽优化的关键**：PackXZ 将 8 字节压到 3 字节，在 AOI 同步中高频使用
