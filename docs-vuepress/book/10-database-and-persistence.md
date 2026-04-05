# 10. 数据库、DBMgr 与持久化设计

> 本文件是旧稿，已由 [`13-database-dbmgr-and-persistence.md`](./13-database-dbmgr-and-persistence.md) 取代。阅读主线请直接进入新稿。

> 这是理解工程取舍的第三个关键章节。KBEngine 的数据库层不是“顺手加了个 DB 访问模块”，而是把在线实体系统和数据库世界显式隔开。

## 12.1 本章核心问题

- 为什么要有 `DBMgr` 这一层？
- 实体的存储、检出、恢复、删除链路是怎样分工的？
- MySQL / Redis 支持分别体现了怎样的设计考虑？

## 12.2 先给结论：`DBMgr` 不是简单 SQL 代理

如果只看名字，很容易以为 `DBMgr` 只是“统一数据库访问入口”。这不够准确。

在 KBEngine 里，`DBMgr` 至少承担四类职责：

- 数据库访问集中化
- 在线实体 / 离线实体边界管理
- 账号与实体基础状态查询
- 异步 DB 任务与主线程回调收束

换句话说，它不是单纯“谁想查库就来查”，而是：

- 服务器内存世界
- 数据库存储世界

之间的一个仲裁层。

## 12.3 为什么持久化不让 Base 或 Cell 直接写库

从前面的生命周期章节已经能看到，玩家实体的真实状态并不都在一个地方：

- Base 更接近账号、脚本逻辑、持久化组织
- Cell 更接近空间位置、AOI、实时运行态

如果让某一侧直接直写数据库，会立刻遇到几个问题：

- 运行时状态不完整
- 并发写入不好统一控制
- 在线实体日志不好维护
- 数据库连接和错误处理散落各处

所以 KBEngine 选的是：

- Base 发起持久化
- Cell 先收束空间态
- Base 再整理持久化流
- `DBMgr` 最终落库

这是非常典型的 MMO 服务端分层思路。

## 12.4 一次 `writeToDB()` 到底怎么走

关键源码：

- `kbe/src/server/baseapp/entity.cpp`
  `Entity::writeToDB`
  `Entity::onCellWriteToDBCompleted`
  `Entity::onWriteToDBCallback`
- `kbe/src/server/baseapp/baseapp.cpp`
  `Baseapp::onCellWriteToDBCompleted`
  `Baseapp::onWriteToDBCallback`
- `kbe/src/server/cellapp/cellapp.cpp`
  `Cellapp::reqWriteToDBFromBaseapp`
- `kbe/src/server/cellapp/entity.cpp`
  `Entity::writeToDB`
- `kbe/src/server/dbmgr/dbmgr.cpp`
  `Dbmgr::writeEntity`
- `kbe/src/server/dbmgr/dbtasks.cpp`
  `DBTaskWriteEntity::presentMainThread`

完整链路是：

1. Base 侧调用 `Entity::writeToDB`
2. 若实体有 Cell，则消息发到 `Cellapp::reqWriteToDBFromBaseapp`
3. Cell 实体执行自己的 `Entity::writeToDB`
4. Cell 侧先 `onWriteToDB()`，再 `backupCellData()`
5. Cell 回消息给 `Baseapp::onCellWriteToDBCompleted`
6. Base 实体执行 `onPreArchive`
7. Base 侧 `addPersistentsDataToStream(...)`
8. 发往 `Dbmgr::writeEntity`
9. `DBTaskWriteEntity` 在线程里执行真正写库
10. 主线程回包 `BaseappInterface::onWriteToDBCallback`
11. Base 实体 `onWriteToDBCallback` 执行最终回调收束

这条链最重要的含义是：

- 持久化不是一次函数调用
- 而是一条跨 Base / Cell / DBMgr 的状态收束流水线

## 12.5 Base、Cell、DBMgr 在写库里各负责什么

### Cell

Cell 侧负责：

- 把实时空间态收束好
- 调用自己的 `onWriteToDB()`
- 执行 `backupCellData()`

注意：Cell 并不直接完成最终持久化流。

### Base

Base 侧负责：

- 决定何时发起归档
- 调用 `onPreArchive`
- 统一把持久化字段编码进流
- 保存 / 取回 Python 回调
- 最终把结果回给脚本层

所以 Base 更像持久化编排者。

### DBMgr

`Dbmgr::writeEntity` 做的事情非常直接：

- 从流中读出 `componentID / eid / entityDBID / dbInterfaceIndex`
- 找到目标数据库接口对应的 `Buffered_DBTasks`
- 投递 `DBTaskWriteEntity`

也就是说，`DBMgr` 主线程主要负责接单和分发，真正数据库 I/O 在异步任务里完成。

## 12.6 `DBTaskWriteEntity` 暴露了哪些关键设计

源码入口：

- `kbe/src/server/dbmgr/dbtasks.cpp`

`DBTaskWriteEntity` 有几段非常值得注意：

### 先写实体表，再写在线日志

流程里会先：

- `entityTables.writeEntity(...)`

如果这是首次写入，还会继续操作：

- `KBEEntityLogTable::logEntity(...)`

这说明数据库里不只有“实体内容”，还有“在线实体日志”这层元数据。

这层日志的重要用途包括：

- 查询实体是否已被某个 Base 激活
- 做检出 / 恢复语义判断
- 防止同一实体被多处同时当作在线实体

### 结果不是直接回脚本，而是回 Baseapp

`DBTaskWriteEntity::presentMainThread` 会发：

- `BaseappInterface::onWriteToDBCallback`

然后再由 Base 实体最终执行 Python 回调。

这说明 `DBMgr` 明确不持有业务脚本上下文，它只负责：

- DB 任务执行
- 把结果送回原组件

## 12.7 `EntityTables` 说明了“实体定义如何映射到存储结构”

源码入口：

- `kbe/src/lib/db_interface/entity_table.cpp`
- `kbe/src/lib/db_interface/entity_table.h`

`EntityTables::writeEntity / removeEntity / queryEntity` 非常直接：

- 先根据实体模块名找到对应表对象
- 再由具体 `EntityTable` 实现完成读写删

这说明 KBEngine 的数据库层不是手写一堆“玩家表、道具表”专用 SQL，而是：

- 从实体定义出发
- 建出对应的实体表结构
- 再由统一表抽象完成操作

这也是为什么实体定义变化会直接影响数据库层。

## 12.8 为什么要有 `KBEEntityLogTable`

这是很多人第一次读源码时容易忽略，但实际上非常关键的一层。

`KBEEntityLogTable` 记录的不是普通业务数据，而是“实体在线日志”。从职责上看，它解决的问题是：

- 某个 `dbid` 当前是否已被检出
- 检出它的是哪个组件
- 对应的是哪个在线实体 ID

这层表让 `DBMgr` 能区分：

- 这是一个纯离线实体
- 这是一个数据库有记录、但服务器里已在线的实体

没有这层元数据，`createEntityFromDBID`、账号恢复、重检出这些能力都会变得非常脆弱。

## 12.9 检出、恢复、删除三条链也要一起看

如果只看 `writeToDB()`，对 DB 设计的理解还是不完整。还要同时看：

- `createEntityFromDBID`
- `queryAccount`
- `deleteEntityByDBID`

### `queryAccount`

`Dbmgr::queryAccount` 及其对应任务不只是查账号密码，还会把：

- `dbid`
- `entityID`
- `componentID`
- `flags`
- `deadline`

这些和在线状态、账号状态直接相关的信息一起带回。

### `createEntityFromDBID`

它体现的是“检出”语义，而不是简单“读一条记录”。若实体已在线，则可能返回在线信息或远端引用，而不是粗暴地再创建一份。

### `deleteEntityByDBID`

这条链说明删除也不允许随便直删，而是同样经过 Base -> DBMgr -> 回调收束。

## 12.10 MySQL 与 Redis 支持说明了什么

源码入口：

- `kbe/src/lib/db_mysql/*`
- `kbe/src/lib/db_redis/*`

这里最值得注意的不是“支持两种后端”本身，而是它们都复用了同一套更高层抽象：

- `DBInterface`
- `EntityTables`
- `KBEEntityLogTable`
- `KBEAccountTable`

这说明 KBEngine 的数据库层抽象重点不是 SQL 语法，而是：

- 实体存储模型
- 账号模型
- 在线日志模型

也就是说，它先抽象“游戏服务器要存什么”，再适配不同数据库后端。

## 12.11 和 BigWorld 一系设计的关系

这一章如果放回 BigWorld / KBEngine 的演化里看，会更容易理解它为什么这样长：

- BigWorld 一系从很早开始就把“在线实体”和“数据库实体”区别得很严格
- 在线实体在内存里跑逻辑，不把数据库当运行时状态来源
- 数据库更多承担冷状态、恢复状态、元信息记录

KBEngine 继承了这条路线，所以才会有：

- `DBMgr`
- 检出 / 在线日志
- Base / Cell / DB 三段式协作

## 12.12 本章小结

这一章最重要的不是记住多少个数据库接口，而是记住下面这些判断：

- `DBMgr` 不是简单 SQL 代理，而是在线 / 离线边界管理者
- `writeToDB()` 不是本地写库，而是跨组件状态收束流水线
- `EntityTables` 体现的是“实体定义到存储结构”的映射
- `KBEEntityLogTable` 解决的是在线实体检出与恢复问题
- MySQL / Redis 只是后端差异，更上层的实体存储模型是一致的

把这些看清楚，后面再看账号恢复、自动加载、备份归档时就不会把它们误读成零散工具函数。
