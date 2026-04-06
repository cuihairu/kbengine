# 9. 分布式基础：ID、发现、注册与一致性

> BigWorld / KBEngine 都是分布式系统。这一章回答：多个进程怎么互相发现？实体的全局唯一 ID 怎么分配？进程死了怎么办？

## 9.1 本章核心问题

- componentID 和 entityID 怎么生成和分配？
- Machine / bwmachined 如何充当"注册中心"？
- 组件注册完成后，怎么被整个集群看见？
- 在线状态一致性怎么保证？
- BigWorld 的 Reviver 如何自动恢复死亡进程？

## 9.2 分布式 ID 生成

### componentID：进程身份标识

每个组件进程启动时需要一个唯一的 `componentID`。

**KBEngine**：由 Machine 分配或本地计算。

```cpp
// 文件：kbe/src/lib/server/kbemain.h（简化）
// checkComponentID 的逻辑：
// 如果 g_componentID 未设置：
//   向 Machine 进程查询分配（UDP 广播）
//   或本地计算：uid * COMPONENT_ID_MULTIPLE + macMD5 * 10000 + componentType * 100 + 1
```

componentID 不是随机数，而是基于 uid + MAC 地址 + 组件类型的确定性计算。同一台机器上同类型组件的 ID 可预测。

**BigWorld**：由 bwmachined 分配。组件启动时通过 `MachineDaemon::registerWithMachined` 向 bwmachined 注册，bwmachined 返回分配的 ID。

### entityID：实体全局唯一标识

实体 ID 必须在**整个集群内唯一**——跨 BaseApp、CellApp、DBMgr。

**KBEngine 的 IDServer / IDClient 模式**：

```cpp
// 文件：kbe/src/lib/server/idallocate.h（简化）

// IDServer（在 DBMgr 上运行）：分配 ID 区间
template<typename T>
class IDServer
{
    std::pair<T, T> allocRange(void)
    {
        std::pair<T, T> p = std::make_pair(
            last_id_range_begin_,
            last_id_range_begin_ + range_step_);
        last_id_range_begin_ += range_step_;
        return p;    // 返回 [begin, begin + step) 的区间
    }

    T last_id_range_begin_;  // 上次分配的起始位置
    T range_step_;           // 每段长度（如 400）
};

// IDClient（在 BaseApp / CellApp 上运行）：本地分配
template<typename T>
class IDClient
{
    T alloc(void)
    {
        if (last_id_range_begin_ > last_id_range_end_) {
            // 本地段用完，需要向 IDServer 请求新段
            return 0;
        }
        return last_id_range_begin_++;
    }

    void onAddRange(T id_begin, T id_end) {
        // 收到 IDServer 分配的新段
        last_id_range_begin_ = id_begin;
        last_id_range_end_ = id_end;
    }
};
```

**分配流程**：

```
BaseApp / CellApp 需要创建实体
  │
  ├── IDClient::alloc()     ← 本地段内递增分配（O(1)，无网络开销）
  │
  ├── 本地段用完时
  │     └── 向 DBMgr 的 IDServer 请求新区间
  │           └── IDServer::allocRange() 返回 [begin, begin+400)
  │
  └── IDClient::onAddRange() ← 收到新区间，继续本地分配
```

**区间分配的优势**：大部分实体创建只需本地递增，不涉及网络通信。只有段用完时才需要一次网络请求。

### 与 Snowflake / UUID 的对比

| 方案 | 优势 | 劣势 |
|------|------|------|
| KBEngine 区间分配 | 简单、有序、本地分配无网络开销 | 依赖中心化 IDServer（DBMgr） |
| Snowflake | 去中心化、时间有序 | 需要机器 ID 分配、时钟回拨问题 |
| UUID v4 | 完全去中心化 | 无序、128 位、DB 索引性能差 |

MMO 服务器选区间分配是合理的：DBMgr 本身就是单点（数据库仲裁者），不需要额外引入去中心化 ID 方案的复杂性。

## 9.3 Machine：游戏服务器的"注册中心"

### 为什么需要注册中心

一个 MMO 集群有 10+ 个进程组件（多个 BaseApp、多个 CellApp、LoginApp、DBMgr、Mgr 类）。没有注册中心：
- CellApp 怎么知道有哪些 BaseApp？
- LoginApp 怎么知道 BaseApp 的地址？
- 新启动的组件怎么让其他组件发现自己？

### KBEngine Machine

```cpp
// 文件：kbe/src/server/machine/machine.h（简化）
class Machine : public ServerApp, public Singleton<Machine>
{
    // 三个核心接口：

    // 1. 组件广播注册
    void onBroadcastInterface(Network::Channel* pChannel,
        int32 uid, COMPONENT_TYPE componentType, COMPONENT_ID componentID,
        uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, ...);

    // 2. 查询特定组件地址
    void onFindInterfaceAddr(Network::Channel* pChannel,
        COMPONENT_TYPE findComponentType, ...);

    // 3. 查询所有组件信息
    void onQueryAllInterfaceInfos(Network::Channel* pChannel, ...);
};
```

**注册流程**：

```
新组件启动
  │
  ├── 向 Machine 广播 onBroadcastInterface
  │     携带：componentType / componentID / 内网地址 / 外网地址
  │
  ├── Machine 收录到组件表
  │     cidMap_: componentID → ComponentInfos
  │     pidMD5Map_: uid → ComponentInfos
  │
  └── 其他组件通过 onFindInterfaceAddr 查询到新组件
```

### BigWorld bwmachined

```cpp
// 文件：server/tools/bwmachined/bwmachined.cpp（核心功能）
// bwmachined 比 KBEngine Machine 复杂得多：

// 1. Cluster：跨机器集群管理
//    - chooseBuddy()：伙伴节点选择
//    - FloodTriggerHandler：洪流发现算法
//    - BirthReplyHandler：新节点加入确认

// 2. Listeners：组件生命周期通知
//    - birthListeners_：组件上线的通知
//    - deathListeners_：组件下线的通知
//    - broadcastToListeners()：向所有监听者广播

// 3. 进程守护
//    - 检查 /proc 或进程 PID 确认存活
//    - 死亡进程自动清理

// 4. 状态持久化
//    - save()：注册表写入磁盘
//    - load()：重启后恢复集群视图
```

### 两者对比

| 维度 | KBEngine Machine | BigWorld bwmachined |
|------|-----------------|-------------------|
| 发现方式 | UDP 广播 + 查询 | 洪流发现 + 广播 |
| 组件表 | `cidMap_` / `pidMD5Map_` | `procs_` vector + PID 检查 |
| 上线通知 | 无（查询式） | `birthListeners_`（推送式） |
| 下线通知 | 无（超时检测） | `deathListeners_`（推送式） |
| 状态持久化 | 无 | `save()` / `load()` |
| 跨机器管理 | 无 | `Cluster` + `chooseBuddy()` |
| 进程存活检查 | 无 | `/proc` PID 检查 |

### 与现代服务注册中心的对比

| 维度 | Machine / bwmachined | Consul / etcd / ZooKeeper |
|------|---------------------|--------------------------|
| 一致性 | 最终一致（广播/查询） | 强一致（Raft/Paxos） |
| 健康检查 | 无 / PID 检查 | TTL 心跳 + HTTP/TCP 探测 |
| 自动摘除 | 无 / 死亡通知 | 无心跳自动摘除 |
| 数据存储 | 内存 / 可选持久化 | 磁盘 + 内存 |
| 选主 | 无 / buddy 选举 | 内置选主 |

**游戏服务器为什么不用 Consul/etcd**：
- 额外依赖，部署复杂度增加
- 游戏服务器进程数量有限（几十个），不需要 KV 存储
- 组件发现频率低（只在启动和崩溃时），不需要实时 watch

## 9.4 onRegisterNewApp：注册完成后的第二层收束

组件在 Machine 注册后，还要向目标组件发送 `onRegisterNewApp`：

```
新 BaseApp 启动
  │
  ├── 第一步：向 Machine 广播 onBroadcastInterface
  │     Machine 知道"你存在"
  │
  ├── 第二步：向 DBMgr 发送 onRegisterNewApp
  │     DBMgr 把你加入自己的组件表
  │
  ├── 第三步：DBMgr 通知其他已注册的 BaseApp
  │     onGetEntityAppFromDbmgr → 其他 BaseApp 主动与你建立 Channel
  │
  └── 第四步：向 BaseappMgr 注册
        BaseappMgr 把你加入负载均衡候选
```

**从"Machine 知道你存在"到"业务组件愿意和你协作"**，这是两层收束。

## 9.5 进程间通信模型

### KBEngine：TCP 点对点

组件间通过 `Channel` 直接通信。每个组件维护一个 `Components` 表（已知组件列表），通过 `NetworkInterface` 建立和维护 Channel。

```
BaseApp ──TCP Channel──▶ CellApp
BaseApp ──TCP Channel──▶ DBMgr
CellApp ──TCP Channel──▶ BaseApp
LoginApp ──TCP Channel──▶ DBMgr
```

### BigWorld：UDP 为主 + 自建可靠性

```
BaseApp ──UDP Channel──▶ CellApp    (ReliableType 可选)
BaseApp ──UDP Channel──▶ DBApp
CellApp ──UDP Channel──▶ BaseApp
```

### 与 nng / Aeron 通信模式的对比

| 模式 | nng | Aeron | BigWorld/KBEngine |
|------|-----|-------|------------------|
| Pair | ✓ 1:1 双向 | — | Channel 点对点 |
| Req-Rep | ✓ 请求-回复 | — | KBEngine: CallbackMgr; BW: TwoWay |
| Pub-Sub | ✓ 多播 | ✓ 多播 + 背压 | CellAppMgr → CellApp 广播 |
| Push-Pull | ✓ 管道 | — | 无直接对应 |
| Survey | ✓ 一查多答 | — | Machine 查询组件 |

BigWorld/KBEngine 本质上用 **Pair 模式**（点对点双向 Channel），辅以组件管理器的 **Survey 模式**（查询所有组件状态）。

## 9.6 一致性保证

### 在线状态一致性

`KBEEntityLogTable` / `EntityLogTable`：在线实体的"检出记录"。

```
实体上线时：
  DBMgr 在 EntityLogTable 写入记录
  { dbid, componentID, entityID, flags, deadline }

实体下线时：
  DBMgr 删除记录

实体恢复/重连时：
  DBMgr 检查 EntityLogTable
  如果有记录 → 上次非正常下线 → 走恢复流程
  如果无记录 → 正常流程
```

没有这层，`createEntityFromDBID` / 账号恢复 / 重检出都会变得脆弱——你不知道这个实体是"纯离线"还是"在线上某个进程里"。

### 实体状态一致性：real vs ghost 的权威模型

```
real entity（权威）
  │
  ├── 拥有完整状态
  ├── 所有写操作必须经过 real
  └── ghost 变更需要转发到 real

ghost entity（副本）
  │
  ├── 只有部分只读状态
  ├── 修改请求转发到 real（RealEntityMethod）
  └── 由 real 定期同步更新
```

这保证了**单一写权限**——同一时刻只有一个 real 拥有修改权。

### 最终一致性 vs 强一致性

MMO 选**最终一致性**：
- 属性同步是 tick 末批量发送，不是即时一致
- Ghost 状态由 real 定期推送，有延迟
- 客户端看到的可能是几帧前的状态

如果选强一致性（每次操作等所有副本确认），10Hz tick 内根本完不成。

## 9.7 稳定性保证：BigWorld Reviver

KBEngine 没有自动进程恢复机制。BigWorld 有 **Reviver**：

```
Reviver 运行流程：

1. 注册 birth/death 监听器到 bwmachined
   │
2. 收到 death 通知
   │
   ├── 检查死亡进程是否在自己负责的组件列表中
   │
   ├── 是 → revive("componentName")
   │     └── 向 bwmachined 发送 CREATE 消息
   │           └── bwmachined 启动新进程
   │
   └── 否 → 忽略
```

**Reviver 的 buddy 选举**：多台机器上的 Reviver 互相监控，通过 bwmachined 的 Cluster 机制选举 buddy。当一台机器宕机，buddy 上的 Reviver 检测到并拉起死亡进程。

**bwmachined 的状态持久化**：`save()` 将进程列表写入磁盘。bwmachined 重启后 `load()` 恢复集群视图。

## 9.8 关键源码入口

### KBEngine

| 概念 | 文件 |
|------|------|
| IDServer/IDClient | `kbe/src/lib/server/idallocate.h` |
| Machine | `kbe/src/server/machine/machine.h` |
| Components 表 | `kbe/src/lib/server/components.h` |
| 负载均衡 | `kbe/src/server/baseappmgr/baseappmgr.h`（`findFreeBaseapp`）|
| 实体日志 | DBMgr 中的 EntityLogTable 相关代码 |

### BigWorld

| 概念 | 文件 |
|------|------|
| bwmachined | `server/tools/bwmachined/bwmachined.cpp` |
| Cluster | `server/tools/bwmachined/cluster.hpp` |
| Listeners | `server/tools/bwmachined/listeners.hpp` |
| MachineDaemon | `lib/network/machine_guard.hpp` |
| Reviver | `server/reviver/reviver.hpp` / `reviver.cpp` |
| IDClient | `lib/server/id_client.hpp` |

## 9.9 源码走读路径

### 路径一：理解实体 ID 分配

1. `kbe/src/lib/server/idallocate.h` — `IDServer::allocRange()` + `IDClient::alloc()`
2. `kbe/src/server/dbmgr/dbmgr.h` — `idServer_` 成员
3. `kbe/src/server/cellapp/cellapp.h` — `idClient_` 成员

### 路径二：理解组件注册流程

1. `kbe/src/server/machine/machine.h` — `onBroadcastInterface()` 三个核心接口
2. `kbe/src/server/baseappmgr/baseappmgr.h` — `onAddComponent()` 收录 BaseApp
3. `kbe/src/server/baseapp/baseapp.h` — `onGetEntityAppFromDbmgr()` 互相建立连接

### 路径三：理解 BigWorld 的分布式注册（对比）

1. `server/tools/bwmachined/bwmachined.cpp` — 进程消息处理
2. `server/tools/bwmachined/cluster.hpp` — 跨机器集群管理 + buddy 选举
3. `server/reviver/reviver.cpp` — 进程恢复逻辑

## 9.10 小结

- **实体 ID 用区间分配**：DBMgr 的 IDServer 分配区间，BaseApp/CellApp 的 IDClient 本地递增，大部分创建操作无网络开销
- **componentID 由 Machine 分配**：基于 uid + MAC + 组件类型的确定性计算
- **注册中心分两层**：Machine 负责地址发现，业务组件（Mgr 类）负责负载和协作
- **onRegisterNewApp 是第二层收束**：从"Machine 知道你存在"到"业务组件愿意和你协作"
- **选最终一致性**而非强一致性：tick 末批量同步、ghost 由 real 定期推送、客户端看到几帧前状态
- **BigWorld 基础设施更完善**：bwmachined 有洪流发现、birth/death 监听器、状态持久化、跨机器 Cluster；Reviver 自动恢复死亡进程
- **KBEngine 更简单**：UDP 广播 + 查询式发现，无自动进程恢复
