# 1. 导读与阅读方法

> 这一章不展开源码机制分析。它只回答两个问题：这本书怎么组织的？你应该怎么读？

## 1.1 这本书是什么

这是一本**从源码出发**的书，不是 API 手册，不是架构图集。

它的目标读者：

- 需要理解 KBEngine 或 BigWorld 引擎内部机制的**引擎程序员**
- 需要在源码层面做二次开发或深度调优的**技术负责人**
- 需要在面试中讲清楚"MMO 服务器为什么这样设计"的**求职者**

它的核心假设：你已经会 C++ 和 Python，你不需要解释什么是虚函数或元表。

## 1.2 两套源码仓库

本书同时分析两个项目：

| 项目 | 源码位置 | 定位 |
|------|---------|------|
| **KBEngine** | 本书工作目录下 `kbe/` | 开源 MMO 引擎，BigWorld 思想的 Python-driven 实现 |
| **BigWorld** | https://github.com/cuihairu/BigWorld-Engine-14.4.1 | 商业 MMO 中间件，KBEngine 的思想源头 |

为什么同时看两个？因为 KBEngine 的很多设计只有在对比 BigWorld 原版时才能看清"为什么这样做"以及"哪些地方做了取舍"。比如：

- BigWorld 有 `TwoWay RPC + Twisted Deferred`，KBEngine 砍掉了，改用 `CallbackMgr`
- BigWorld 用 `BSP 树` 做空间动态分割，KBEngine 简化了
- BigWorld 有 `Reviver` 自动拉起死亡进程，KBEngine 没有
- BigWorld 有 `bwmachined` 分布式注册中心，KBEngine 的 `Machine` 更简单

反过来，KBEngine 也有 BigWorld 没有的东西：

- `lib/db_redis/` — Redis 作为持久化后端
- `KCP` 协议支持
- `guiconsole` — 集成式 GUI 管理工具

## 1.3 源码目录地图

开始阅读前，先建立目录直觉。

### KBEngine（`kbe/src/`）

```
kbe/src/
├── lib/                    ← 通用库（不分组件）
│   ├── server/             ← 服务器基础设施：ServerApp / EntityApp / PythonApp
│   ├── network/            ← 网络：Bundle / Channel / EventDispatcher / epoll
│   ├── entitydef/          ← 实体定义系统：EntityDef / ScriptDefModule / EntityCall
│   ├── pyscript/           ← Python 桥接：PyType / tp_call
│   ├── db_interface/       ← 数据库抽象接口
│   ├── db_mysql/           ← MySQL 实现
│   ├── db_redis/           ← Redis 实现
│   ├── thread/             ← 线程池 / 互斥锁
│   ├── common/             ← 工具：ObjectPool / MemoryStream / tasks
│   └── math/               ← 数学库
│
├── server/                 ← 组件进程
│   ├── baseapp/            ← 玩家长期逻辑主体（Entity / Proxy / Backuper / Archiver）
│   ├── cellapp/            ← 空间实时世界（SpaceMemory / Witness / View_trigger / CoordinateSystem）
│   ├── cellappmgr/         ← CellApp 调度
│   ├── dbmgr/              ← 持久化仲裁（DBMgr / DBTaskWriteEntity）
│   ├── baseappmgr/         ← BaseApp 调度
│   ├── loginapp/           ← 第一跳接入
│   ├── machine/            ← 服务注册中心
│   └── tools/
│       ├── bots/           ← 压测机器人
│       ├── guiconsole/     ← GUI 管理工具
│       ├── interfaces/     ← 外部接口网关
│       └── logger/         ← 集中日志
│
├── client/                 ← 客户端 SDK
└── res/                    ← 资源定义（.def / messages_fixed_defaults.xml）
```

### BigWorld（`programming/bigworld/`）

```
programming/bigworld/
├── lib/                    ← 通用库
│   ├── server/             ← 服务器基础设施（ServerApp / EntityApp / ScriptApp / PythonServer）
│   ├── network/            ← 网络（Bundle / Channel / EventPoller / EventDispatcher）
│   ├── entitydef/          ← 实体定义（EntityDescription / MethodDescription / Mailbox）
│   ├── entitydef_script/   ← 实体定义脚本支持
│   ├── pyscript/           ← Python 桥接（50+ 文件，比 KBEngine 更完整）
│   ├── script/             ← 脚本核心
│   ├── space/              ← 空间管理
│   ├── db/                  ← 数据库接口
│   ├── db_storage/          ← 存储抽象
│   ├── db_storage_mysql/    ← MySQL 实现（含 EntityMapping / PropertyMapping / tasks/）
│   ├── db_storage_xml/      ← XML 存储
│   ├── cstdmf/              ← 通用基础设施（concurrency / object_pool / profiler / allocator）
│   └── appmgr/              ← 应用框架
│
├── server/                 ← 组件进程
│   ├── baseapp/            ← 玩家长期逻辑（Base / Mailbox / WorkerThread / BackupSender / Archiver）
│   ├── cellapp/            ← 空间世界（Cell / Space / Witness / RangeTrigger / GhostManager / Controller）
│   ├── cellappmgr/         ← Cell 调度 + **BSP 树拓扑 + 动态负载均衡**
│   ├── dbapp/              ← 持久化（DBApp + EntityAutoLoader + BillingSystem）
│   ├── dbappmgr/           ← DB 集群协调
│   ├── baseappmgr/         ← Base 调度
│   ├── loginapp/           ← 第一跳接入
│   ├── reviver/            ← 进程恢复（BigWorld 独有）
│   └── tools/
│       ├── bots/           ← 压测机器人（更丰富：beeline / zigzag / patrol）
│       ├── bwmachined/     ← 分布式注册中心（Cluster + Flood + Listeners）
│       ├── message_logger/ ← 集中日志（binary + text 双格式）
│       └── bw_profile/     ← 性能分析
│
├── client/                 ← 客户端
└── tools/
    ├── worldeditor/        ← 世界编辑器
    ├── modeleditor/        ← 模型编辑器
    ├── particle_editor/    ← 粒子编辑器
    └── navgen/             ← 导航网格生成
```

### 关键对应关系

读源码时，随时对照这张表：

| 概念 | KBEngine 文件 | BigWorld 文件 |
|------|-------------|-------------|
| 主循环 | `lib/network/event_dispatcher.h` | `lib/network/event_dispatcher.hpp` |
| EntityCall | `lib/entitydef/entity_call.cpp` | `lib/entitydef/mailbox_base.cpp` |
| 远程方法 | `lib/entitydef/remote_entity_method.cpp` | `lib/entitydef/remote_entity_method.cpp` |
| 实体定义 | `lib/entitydef/scriptdef_module.cpp` | `lib/entitydef/entity_description.cpp` |
| 空间 | `server/cellapp/spacememory.cpp` | `server/cellapp/space.cpp` |
| AOI | `server/cellapp/range_trigger.cpp` | `server/cellapp/range_trigger.cpp` |
| Witness | `server/cellapp/witness.cpp` | `server/cellapp/witness.cpp` |
| 持久化 | `server/dbmgr/dbtasks.cpp` | `lib/db_storage_mysql/tasks/put_entity_task.cpp` |
| 注册中心 | `server/machine/machine.cpp` | `server/tools/bwmachined/bwmachined.cpp` |

## 1.4 每章的统一结构

每一章都尽量遵循这个结构：

```
1. 核心问题：这一章要回答什么？
2. BigWorld 背景：BigWorld 是怎么做的？（如果相关）
3. KBEngine 实现：源码入口 + 关键代码片段 + 调用链
4. 设计对比：与替代方案的对比（gRPC / nng / Aeron / Redis 等）
5. 关键源码入口：可以直接在 IDE 里打开的文件列表
6. 源码走读路径：建议的阅读顺序
7. 小结：这一章最重要的结论
```

不是每章都有全部 7 节，但原则上每章都会先交代核心问题，并给出可直接落到 IDE 的源码入口。

## 1.5 建议的阅读方式

### 第一遍：按顺序通读（建立全景）

从头到尾读一遍。这一遍的目标不是记住细节，而是建立一张心理地图：

- 哪些组件在哪
- 谁调谁
- 数据从哪来、到哪去

### 第二遍：IDE 跟读（验证地图）

打开 IDE，按每章给出的"源码走读路径"逐个跳转。重点看：

- 函数签名和注释
- 关键分支（if/switch）
- 成员变量列表

这一遍的目标是验证第一遍建立的心理地图是否正确。

### 第三遍：专题深挖

选一条链路走到底。比如"从 `writeToDB` 到数据库"，在 IDE 里逐函数跟踪，不要跳过任何一个间接调用。

推荐先跟这三条链：

1. **登录主链（中后段）**：`Loginapp::onLoginAccountQueryResultFromDbmgr → Baseapp::loginBaseapp → Baseapp::onQueryAccountCBFromDbmgr`
2. **RPC 链**：`RemoteEntityMethod::tp_call → EntityCallAbstract::newCall_ → Channel::send`
3. **持久化链**：`Entity::writeToDB → Cellapp::reqWriteToDBFromBaseapp → Dbmgr::writeEntity → DBTaskWriteEntity::presentMainThread`

跟完这三条链，KBEngine 的整体骨架就立住了。

## 1.6 源码链接约定

本书中引用源码时使用以下格式：

- **KBEngine**：直接写相对路径 `kbe/src/server/baseapp/proxy.cpp:42`
  - 读者可以在 IDE 中直接搜索 `proxy.cpp` 并跳到对应行
- **BigWorld**：使用 GitHub 链接
  ```
  https://github.com/cuihairu/BigWorld-Engine-14.4.1/blob/main/programming/bigworld/server/baseapp/proxy.cpp#L42
  ```

代码片段的格式：

```cpp
// 文件：kbe/src/server/baseapp/proxy.cpp:42
// 直接贴出关键代码，不做伪代码改写
uint64 rndUUID() const { return rndUUID_; }
```

## 1.7 这本书不做什么

- **不按函数逐个解释 API。** 有 API 手册做这件事。
- **不把背景资料拆成低密度铺垫。** BigWorld 的历史和概念压缩在一章里讲完。
- **不把源码分析和接口手册混在一起。** 每章关注"为什么"和"怎么做到的"，不是"有哪些参数"。
