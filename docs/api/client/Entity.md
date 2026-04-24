# Entity类

[KBEngine模块]

Entity是KBEngine模块的一部分。[更多...](#detailed_description)

```python
import KBEngine
```

## 成员函数

- def [baseCall](#baseCall)( self, methodName, methodArgs ):
- def [cellCall](#cellCall)( self, methodName, methodArgs ):
- def [isPlayer](#isPlayer)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## 回调函数

- def [onDestroy](#onDestroy)( self ):
- def [onEnterWorld](#onEnterWorld)( self ):
- def [onLeaveWorld](#onLeaveWorld)( self ):
- def [onEnterSpace](#onEnterSpace)( self ):
- def [onLeaveSpace](#onLeaveSpace)( self ):

## 属性

| [base](#base) | 只读 ENTITYCALL |
| --- | --- |
| [cell](#cell) | 只读 ENTITYCALL |
| [clientapp](#clientapp) | 只读 PyClientApp |
| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| [id](#id) | 只读 Integer |
| [inWorld](#inWorld) | 只读 bool |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | 只读 uint32 |
| [isOnGround](#isOnGround) | bool |
| [className](#className) | 只读 string |

<a id="detailed_description"></a>

## 详细描述

类Entity的实例代表着在client上的游戏对象。
一个Entity可以通过ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。

## 成员函数文档

<a id="baseCall"></a>

### def baseCall(self, methodName, methodArgs):

功能说明：
调用当前实体持有的 `base entityCall` 对应的服务端 base 方法。
该接口依赖 `.def` 中声明的客户端可调用 base 方法；底层会按生成的方法表校验方法名、参数个数和参数类型。
常规流程下通常只有当前连接对应的Player实体会在 `onCreatedProxies()` 后拿到 `base entityCall`，因此通常只有它能成功调用该接口。
例子：
js插件: entity.baseCall("reqCreateAvatar", roleType, name);
c#插件: entity.baseCall("reqCreateAvatar", new object[]{roleType, name});

源码解析：

- [网络与消息系统：`baseCall()` 与 `cellCall()` 的真实限制条件](/architecture/source-analysis/networking.html#client-entity-remote-calls)

参数：

| methodName | string，方法名称。 |
| --- | --- |
| methodArgs | objects，方法参数列表。 |

返回：

- 由于是远程调用，不可能阻塞等待返回，因此无返回值。

<a id="cellCall"></a>

### def cellCall(self, methodName, methodArgs):

功能说明：
调用当前实体持有的 `cell entityCall` 对应的服务端 cell 方法。
该接口依赖 `.def` 中声明的客户端可调用 cell 方法；底层会按生成的方法表校验方法名、参数个数和参数类型。
与 `baseCall()` 不同，`cell entityCall` 通常会在实体进入世界对象集合时建立，因此只要当前实体已经拿到 `cell entityCall`，通常不限于Player实体。
例子：
js插件: entity.cellCall("xxx", roleType, name);
c#插件: entity.cellCall("xxx", new object[]{roleType, name});

源码解析：

- [网络与消息系统：`baseCall()` 与 `cellCall()` 的真实限制条件](/architecture/source-analysis/networking.html#client-entity-remote-calls)

参数：

| methodName | string，方法名称。 |
| --- | --- |
| methodArgs | objects，方法参数列表。 |

返回：

- 由于是远程调用，不可能阻塞等待返回，因此无返回值。

<a id="isPlayer"></a>

### def isPlayer(self):

功能说明：
这个函数返回当前这个Entity是否为本次客户端连接对应的Player实体。
它本质上比较的是当前实体的 `id` 与客户端运行时记录的 `entity_id`，不是检查该实体当前是否处于 `controlled` 状态。

源码解析：

- [网络与消息系统：`isPlayer()`、`player()` 和 `controlledBy()` 的关系](/architecture/source-analysis/networking.html#client-entity-isplayer-control)

返回：

- bool，如果该实体就是当前连接对应的Player实体返回True，否则返回False。

<a id="getComponent"></a>

### def getComponent(self, componentName, all):

功能说明：
该函数用于获取实体所绑定的某一类组件实例。

参数：

| componentName | string，组件类型名称，组件的模块名称。 |
| --- | --- |
| all | bool，如果为True，返回所有同类组件实例，否则只返回第一个或空列表。 |

<a id="fireEvent"></a>

### def fireEvent(self, eventName, *args):

功能说明：
该函数用于触发实体事件。

参数：

| eventName | string，要触发的事件名称。 |
| --- | --- |
| args | 要附带的事件数据，可变参数。 |

<a id="registerEvent"></a>

### def registerEvent(self, eventName, callback):

功能说明：
该函数用于注册实体事件。

参数：

| eventName | string，要注册监听的事件名称。 |
| --- | --- |
| callback | 当事件触发时，用于响应该事件的回调方法。 |

<a id="deregisterEvent"></a>

### def deregisterEvent(self, eventName, callback):

功能说明：
该函数用于注销监听实体事件。

参数：

| eventName | string，要注销监听的事件名称。 |
| --- | --- |
| callback | 要注销监听的回调方法。 |

## 回调函数文档

<a id="onDestroy"></a>

### def onDestroy(self):

实体被销毁时调用。

<a id="onEnterWorld"></a>

### def onEnterWorld(self):

如果当前实体不是本次连接对应的Player实体，则表示该实体进入了当前客户端维护的世界对象集合，通常也就是进入了玩家的 AOI / View，可开始对客户端可见。
如果当前实体就是本次连接对应的Player实体，则表示该玩家实体的 cell 侧已经就绪并进入了客户端当前世界对象集合；底层会先补齐 `cellEntityCall`，然后再触发该回调。

源码解析：

- [网络与消息系统：客户端实体进入/离开世界与空间回调](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)

<a id="onLeaveWorld"></a>

### def onLeaveWorld(self):

如果当前实体不是本次连接对应的Player实体，则表示该实体离开了当前客户端维护的世界对象集合，通常意味着离开 AOI / View；底层通常会在该回调后继续销毁这个客户端实体。
如果当前实体就是本次连接对应的Player实体，则表示客户端将清理当前空间上下文，并移除该实体对应的 `cellEntityCall`。

源码解析：

- [网络与消息系统：客户端实体进入/离开世界与空间回调](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)

<a id="onEnterSpace"></a>

### def onEnterSpace(self):

当前连接对应的Player实体进入了一个新的 space。
这个回调对应的是客户端收到 `onEntityEnterSpace` 消息后的处理，重点是更新当前空间上下文，而不是普通可见实体进入 AOI。

源码解析：

- [网络与消息系统：客户端实体进入/离开世界与空间回调](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)

<a id="onLeaveSpace"></a>

### def onLeaveSpace(self):

当前连接对应的Player实体离开了当前 space。
底层会在该回调后清理当前客户端的空间上下文。

源码解析：

- [网络与消息系统：客户端实体进入/离开世界与空间回调](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)

## 属性文档

<a id="base"></a>

base

说明：
base 是当前实体持有的 `base entityCall`。它是否存在，取决于客户端运行时是否已经为该实体建立了 base 侧句柄。
常规流程下通常只有当前连接对应的Player实体会在 `onCreatedProxies()` 后拿到它，普通可见实体通常为 `None`。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- 只读，ENTITYCALL

<a id="cell"></a>

cell

说明：
cell 是当前实体持有的 `cell entityCall`。它通常在实体进入客户端世界对象集合时建立，因此并不保证在实体创建后立即可用。
当当前连接对应的Player实体离开世界时，该句柄也会被移除。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- 只读，ENTITYCALL

<a id="clientapp"></a>

clientapp

说明：
当前实体所属的本地客户端运行时对象。
这个属性指向拥有该实体实例的客户端上下文对象，而不是某个“玩家实体”。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- 只读，PyClientApp

<a id="className"></a>

className

实体的类名。

类型：

- 只读，string

<a id="id"></a>

id

说明：
实体对象ID。这个ID在 base、cell、client 三侧关联的同一实体之间保持一致。
`KBEngine.player()`、`KBEngine.findEntity()` 和 `KBEngine.entities` 都是围绕这个 ID 做查找。

类型：

- 只读，int32

<a id="inWorld"></a>

inWorld

说明：
表示该实体当前是否属于客户端维护的世界对象集合。
需要注意，这个状态和“实体是否存在于 `KBEngine.entities` 中”不是一回事：实体可以先被创建并加入实体表，但要等收到进入世界消息后才会变成 `True`。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- 只读，bool

<a id="position"></a>

position

这个实体在世界空间中的坐标(x, y, z)，数据由服务端同步到客户端。

类型：

- Vector3

<a id="spaceID"></a>

spaceID

说明：
实体当前已知的空间ID。
对当前连接对应的Player实体来说，这个值会跟随进入/离开空间或离开世界被更新；当客户端清理当前空间上下文时，该值可能被置为 `0`。

类型：

- 只读，uint32

<a id="direction"></a>

direction

这个属性描述的是Entity在世界空间中的朝向，数据由服务端同步到客户端。

类型：

- Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。

<a id="isOnGround"></a>

isOnGround

如果这个属性的值为True，表示引擎当前认为该实体在地面上，否则为False。
这个属性既可能由服务端同步到客户端，也可能在本地移动控制时被客户端改写；真正决定它是否会上行到服务端的，不是简单的 `isPlayer()`，而是当前这个实体是否由本客户端持有移动控制权。
例如本地移动控制流程中，底层可能会主动把它设为 `false`；当当前玩家实体已经处于 `isControlled == true` 时，本地对它的改动则不会继续上行。

源码解析：

- [网络与消息系统：客户端 `moveToPoint()`、`cancelController()` 与 `isOnGround` 同步](/architecture/source-analysis/networking.html#client-entity-move-ground-sync)

类型：

- bool
