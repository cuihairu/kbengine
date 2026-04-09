# PyClientApp类

[KBEngine模块]

PyClientApp是KBEngine模块的一部分，由C++底层模拟一个客户端时创建的客户端对象，脚本层不可直接创建。

## 成员函数

- def [getSpaceData](#getSpaceData)( key ):
- def [player](#player)( ):

## 回调函数

## 属性

| [id](#id) | 只读 Integer |
| --- | --- |
| [entities](#entities) | Entities |

<a id="detailed_description"></a>

## 详细描述

类Entity的实例代表着在client上的游戏对象。
一个Entity可以通过ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。

## 成员函数文档

<a id="getSpaceData"></a>

### def getSpaceData(key):

功能说明：
获取指定key的space数据。
space数据由用户在服务端通过setSpaceData设置。

参数：

| key | string，一个字符串关键字。 |
| --- | --- |

返回：

- string，指定key的字符串数据。

<a id="player"></a>

### def player():

功能说明：
获得当前这个 bots 客户端连接对应的Player实体。

源码解析：

- [网络与消息系统：`isPlayer()`、`player()` 和 `controlledBy()` 的关系](/architecture/source-analysis/networking.html#client-entity-isplayer-control)

返回：

- Entity，返回当前连接对应的Player实体；如果该实体尚未创建则返回空。

## 回调函数文档

## 属性文档

<a id="id"></a>

id

说明：
当前这个 bots 客户端运行时对象的ID。
它表示本地客户端上下文本身，不是玩家实体ID。

类型：

- 只读，Integer

<a id="entities"></a>

entities

说明：
entities 是当前这个 bots 客户端运行时维护的实体实例表。
它包含当前 bots 客户端仍然保留的实体对象，不等于“当前 AOI 可见实体列表”：当前连接对应的Player实体可能先进入实体表，再在后续世界/空间消息里更新自己的 `inWorld` 与 `spaceID` 状态。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- Entities
