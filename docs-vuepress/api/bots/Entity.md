# Entity类

[KBEngine模块]

Entity是KBEngine模块的一部分。[更多...](#detailed_description)

```python
import KBEngine
```

## 成员函数

- def [moveToPoint](#moveToPoint)( self, destination, velocity, distance, userData, faceMovement, moveVertically ):
- def [cancelController](#cancelController)( self, controllerID ):
- def [isPlayer](#isPlayer)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## 回调函数

- def [onEnterWorld](#onEnterWorld)( self ):
- def [onLeaveWorld](#onLeaveWorld)( self ):
- def [onEnterSpace](#onEnterSpace)( self ):
- def [onLeaveSpace](#onLeaveSpace)( self ):

## 属性

| [base](#base) | 只读ENTITYCALL |
| --- | --- |
| [cell](#cell) | 只读ENTITYCALL |
| [className](#className) | 只读string |
| [clientapp](#clientapp) | 只读PyClientApp |
| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| [id](#id) | 只读 Integer |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | 只读 uint32 |
| [isOnGround](#isOnGround) | bool |

<a id="detailed_description"></a>

## 详细描述

类Entity的实例代表着在client上的游戏对象。
一个Entity可以通过ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。

## 成员函数文档

<a id="moveToPoint"></a>

### def moveToPoint(self, destination, velocity, distance, userData, faceMovement, moveVertically):

功能说明：
在 bots 客户端本地创建一个直线移动控制器，使该实体朝给定坐标点持续推进。
任何[实体](./Entity.md)在任意时刻只能有一个本地移动控制器，重复调用任何移动函数会终止之前的本地移动控制器。
返回一个可用于取消这次本地移动的控制器ID。
例如：
Entity.[cancelController](./Entity.md#cancelController)( movementID )。
移动取消还可以调用Entity.[cancelController](./Entity.md#cancelController)( "Movement" )。当移动被取消之后通知方法将
不被调用。
回调函数如下定义：

```python
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):
```

参看：

- Entity.[cancelController](./Entity.md#cancelController)

源码解析：

- [网络与消息系统：客户端 `moveToPoint()`、`cancelController()` 与 `isOnGround` 同步](/architecture/source-analysis/networking.html#client-entity-move-ground-sync)

参数：

| destination | Vector3，Entity要移动到的目标位置点 |
| --- | --- |
| velocity | float，Entity的移动速度，单位m/s |
| distance | float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。 |
| userData | object，传给通知函数的数据 |
| faceMovement | bool，如果实体面向移动方向则为true。如果是其它机制则为false。 |
| moveVertically | bool，设为true指移动为直线移动，设为false指贴着地面移动。 |

返回：

- int，新创建的控制器ID。

<a id="cancelController"></a>

### def cancelController(self, controllerID):

功能说明：
停止一个 bots 客户端本地控制器对该实体的影响。
当前客户端侧主要对应的是本地移动控制器，支持传入控制器ID，也支持传入字符串 `"Movement"` 取消当前移动。

源码解析：

- [网络与消息系统：客户端 `moveToPoint()`、`cancelController()` 与 `isOnGround` 同步](/architecture/source-analysis/networking.html#client-entity-move-ground-sync)

参数：

| controllerID | 要取消的本地控制器ID。也可以传入专用控制器类型字符串 `"Movement"`，用于取消当前 bots 客户端上的本地移动控制器。 |
| --- | --- |

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

<a id="onEnterWorld"></a>

### def onEnterWorld(self):

如果当前实体不是本次连接对应的Player实体，则表示该实体进入了当前 bots 客户端维护的世界对象集合，通常也就是进入了玩家的 AOI / View，可开始对 bots 客户端可见。
如果当前实体就是本次连接对应的Player实体，则表示该玩家实体的 cell 侧已经就绪并进入了当前 bots 客户端的世界对象集合；底层会先补齐 `cellEntityCall`，然后再触发该回调。

源码解析：

- [网络与消息系统：客户端实体进入/离开世界与空间回调](/architecture/source-analysis/networking.html#client-entity-world-space-callbacks)

<a id="onLeaveWorld"></a>

### def onLeaveWorld(self):

如果当前实体不是本次连接对应的Player实体，则表示该实体离开了当前 bots 客户端维护的世界对象集合，通常意味着离开 AOI / View；底层通常会在该回调后继续销毁这个客户端实体。
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
base 是当前 bots 客户端实体持有的 `base entityCall`。它是否存在，取决于 bots 运行时是否已经为该实体建立了 base 侧句柄。
常规流程下通常只有当前连接对应的Player实体会在 `onCreatedProxies()` 后拿到它，普通可见实体通常为 `None`。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

其他参考：

- Entity.[clientEntity](./Entity.md#clientEntity) Entity.[allClients](./Entity.md#allClients) Entity.[otherClients](./Entity.md#otherClients)

类型：

- 只读的，ENTITYCALL

<a id="cell"></a>

cell

说明：
cell 是当前 bots 客户端实体持有的 `cell entityCall`。它通常在实体进入 bots 客户端世界对象集合时建立，因此并不保证在实体创建后立即可用。
当当前连接对应的Player实体离开世界时，该句柄也会被移除。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- 只读ENTITYCALL

<a id="cellData"></a>

cellData

说明：
cellData是一个字典属性。每当base实体没有创建它的cell实体时，cell实体的属性会保存在这里。
如果cell实体被创建，这些用到的值和[cellData](#cellData)属性将被删除。除了cell实体在实体定义文件里指定的属性外，它还包含position, direction and
spaceID。

类型：

- CELLDATADICT

<a id="className"></a>

className

说明：
实体的类名。

类型：

- 只读，string

<a id="clientapp"></a>

clientapp

说明：
当前实体所属的 bots 客户端运行时对象。
这个属性指向拥有该实体实例的本地客户端上下文对象，而不是某个“玩家实体”。

源码解析：

- [网络与消息系统：客户端的句柄表与实体容器语义](/architecture/source-analysis/networking.html#client-entity-handles-table)

类型：

- 只读，PyClientApp

<a id="id"></a>

id

说明：
实体对象ID。这个ID在 base、cell、client 三侧关联的同一实体之间保持一致。

类型：

- 只读，int32

<a id="position"></a>

position

这个实体在世界空间中的坐标(x, y, z)，数据由服务端同步到客户端。

类型：

- Vector3

<a id="direction"></a>

direction

这个属性描述的是Entity在世界空间中的朝向，数据由服务端同步到客户端。

类型：

- Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。

<a id="isOnGround"></a>

isOnGround

如果这个属性的值为True，表示引擎当前认为该实体在地面上，否则为False。
这个属性既可能由服务端同步到 bots 客户端，也可能在本地移动控制时被 bots 客户端改写；真正决定它是否会上行到服务端的，是当前这个实体是否由当前 bots 客户端持有移动控制权。
例如本地 `moveToPoint()` 一类非导航式移动过程中，底层会主动把它设为 `false`。

源码解析：

- [网络与消息系统：客户端 `moveToPoint()`、`cancelController()` 与 `isOnGround` 同步](/architecture/source-analysis/networking.html#client-entity-move-ground-sync)

类型：

- bool

<a id="spaceID"></a>

spaceID

说明：
实体当前已知的空间ID。
对当前连接对应的Player实体来说，这个值会跟随进入/离开空间或离开世界被更新；当 bots 客户端清理当前空间上下文时，该值可能被置为 `0`。

类型：

- 只读，uint32
