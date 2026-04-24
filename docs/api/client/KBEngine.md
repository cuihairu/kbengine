# KBEngine模块

[KBEngine](./KBEngine.md)模块提供了逻辑脚本层访问实体的部分, 以及当前space的数据等等。

## 类

- Entity

## 成员函数

- def [login](#login)( username, password ):
- def [createAccount](#createAccount)( username, password ):
- def [reloginBaseapp](#reloginBaseapp)():
- def [player](#player)( ):
- def [resetPassword](#resetPassword)(username):
- def [bindAccountEmail](#bindAccountEmail)( emailaddress ):
- def [newPassword](#newPassword)( oldpassword, newpassword ):
- def [findEntity](#findEntity)( entityID ):
- def [getSpaceData](#getSpaceData)( key ):

## 回调函数

## 属性

| [component](#component) | 只读string |
| --- | --- |
| [entities](#entities) | Entities[entity_uuid](#entity_uuid)uint64[entity_id](#entity_id)int32[spaceID](#spaceID)int32 |

## 成员函数文档

<a id="login"></a>

### def login(username, password):

功能说明：
登录账号到KBEngine服务端。
注意：如果插件与UI层使用事件交互模式，在UI层不要直接调用，请触发一个"login"事件给插件，事件附带数据username和password。

参数：

| username | string，用户名。 |
| --- | --- |
| password | string，密码。 |

<a id="createAccount"></a>

### def createAccount(username, password):

功能说明：
请求向KBEngine服务端创建一个登录账号。
注意：如果插件与UI层使用事件交互模式，在UI层不要直接调用，请触发一个"createAccount"事件给插件，事件附带数据username和password。

参数：

| username | string，用户名。 |
| --- | --- |
| password | string，密码。 |

<a id="reloginBaseapp"></a>

### def reloginBaseapp():

功能说明：
请求重登录到KBEngine服务端(通常在掉线之后希望更及时的连接到服务端并继续控制服务端角色时使用)。
注意：如果插件与UI层使用事件交互模式，在UI层不要直接调用，请触发一个"reloginBaseapp"事件给插件，事件附带数据为空。

<a id="player"></a>

### def player():

功能说明：
获得本次客户端连接对应的Player实体。
它本质上是按客户端运行时记录的 `entity_id` 从 `entities` 字典中取回实体，不表示该实体当前一定由本地客户端持有移动控制权。

源码解析：

- [网络与消息系统：`isPlayer()`、`player()` 和 `controlledBy()` 的关系](/architecture/source-analysis/networking.html#client-entity-isplayer-control)

返回：

- Entity，返回当前连接对应的Player实体；如果该实体尚未创建或当前未连接到服务端则返回空。

<a id="resetPassword"></a>

### def resetPassword(username):

功能说明：
请求loginapp重置账号的密码, 服务端将会向该账号绑定的邮箱发送一封重置密码邮件(通常是忘记密码功能使用)。

参数：

| username | string，用户名。 |
| --- | --- |

<a id="bindAccountEmail"></a>

### def bindAccountEmail(emailaddress):

功能说明：
请求baseapp绑定账号的email地址。

参数：

| emailaddress | string，邮箱地址。 |
| --- | --- |

<a id="newPassword"></a>

### def newPassword(oldpassword, newpassword):

功能说明：
请求设置账号的新密码。

参数：

| oldpassword | string，旧密码。 |
| --- | --- |
| newpassword | string，新密码。 |

<a id="findEntity"></a>

### def findEntity(entityID):

功能说明：
通过实体ID在客户端运行时维护的实体表中查找实体实例。
这个查找直接针对 `entities` 容器本身，不额外要求目标实体当前已经 `inWorld`；因此“能查到实体”和“实体已经进入客户端世界对象集合”是两件不同的事。

参数：

| entityID | int32，实体ID。 |
| --- | --- |

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

返回：

- Entity，存在返回实体实例，不存在返回空。

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

## 回调函数文档

## 属性文档

<a id="component"></a>

component

说明：
这是正运行在当前脚本环境的组件。（至今为止）可能值有'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' 和 'editor'。

<a id="entities"></a>

entities

说明：
entities 是客户端运行时维护的实体实例表。
它包含当前客户端仍然保留的实体对象，不等于“当前 AOI 可见实体列表”：例如当前连接对应的Player实体可能在收到 `onCreatedProxies()` 后就已经存在于表里，但要等进入世界后 `inWorld` 才会变为 `True`；清理当前空间时，客户端通常还会保留Player实体本身。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- Entities

<a id="entity_uuid"></a>

entity_uuid

说明：
实体的uuid，改ID与实体本次登录绑定。当使用重登陆功能时服务端会与此ID进行比对，判断合法性。

<a id="entity_id"></a>

entity_id

说明：
当前连接对应的 Player 实体 ID。
它是在服务端下发 `onCreatedProxies()` 时写入客户端运行时的连接级身份标记，`player()` 和 `Entity.isPlayer()` 都围绕它判断。
它不等于“当前由本客户端持有移动控制权的实体”。

源码解析：

- [网络与消息系统：`isPlayer()`、`player()` 和 `controlledBy()` 的关系](/architecture/source-analysis/networking.html#client-entity-isplayer-control)

<a id="spaceID"></a>

spaceID

说明：
当前客户端记录的玩家空间上下文 ID。
它会在当前玩家收到 `onEntityEnterSpace()` 或初始化 spaceData 时更新，在离开当前空间或执行 `clearSpace(false)` 后回到 `0`。
它描述的是“当前连接对应玩家现在处于哪个空间”，不等于 `KBEngine.entities` 中所有实体共享的过滤条件，也不等于任意一个实体对象自己的 `spaceID`。

源码解析：

- [网络与消息系统：客户端的 `onEnterWorld()` / `onLeaveWorld()` / `onEnterSpace()` / `onLeaveSpace()` 也不是同一层事件](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)
