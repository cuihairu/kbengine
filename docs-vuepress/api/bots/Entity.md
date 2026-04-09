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
| [cell](#client) | 只读ENTITYCALL |
| [className](#className) | 只读string |
| [clientapp](#clientapp) | 只读PyClientApp |
| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| [id](#id) | 只读 Integer |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | 只读 uint32 |
| [isOnGround](#isOnGround) | 只读 bool |

<a id="detailed_description"></a>

## 详细描述

类Entity的实例代表着在client上的游戏对象。
一个Entity可以通过ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。

## 成员函数文档

<a id="moveToPoint"></a>

### def moveToPoint(self, destination, velocity, distance, userData, faceMovement, moveVertically):

功能说明：
直线移动Entity到给定的坐标点，成功或失败会调用回调函数。
任何[实体](./Entity.md)，在任意时刻只能有一个移动控制器，重复调用任何移动函数将终止之前的移动控制器。
返回一个可以用于取消这次移动的控制器ID。
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
函数cancelController停止一个控制器对Entity的影响。它只能在一个real实体上被调用。

参数：

| controllerID | controllerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。 例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。 |
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

如果实体非客户端控制实体，则表明实体进入了服务端上客户端控制的实体的View范围，此时客户端可以看见这个实体了。
如果实体是客户端控制的实体则表明该实体已经在服务端创建了cell并进入了space。

<a id="onLeaveWorld"></a>

### def onLeaveWorld(self):

如果实体非客户端控制实体，则表明实体离开了服务端上客户端控制的实体的View范围，此时客户端看不见这个实体了。
如果实体是客户端控制的实体则表明该实体已经在服务端销毁了cell并离开了space。

<a id="onEnterSpace"></a>

### def onEnterSpace(self):

客户端控制的实体进入了一个新的space。

<a id="onLeaveSpace"></a>

### def onLeaveSpace(self):

客户端控制的实体离开了当前的space。

## 属性文档

<a id="base"></a>

base

base是用于联系Entity实体的entityCall。这个属性是只读的，且如果这个实体没有关联的Entity实体时属性是None。

其他参考：

- Entity.[clientEntity](./Entity.md#clientEntity) Entity.[allClients](./Entity.md#allClients) Entity.[otherClients](./Entity.md#otherClients)

类型：

- 只读的，ENTITYCALL

<a id="cell"></a>

cell

说明：
cell是用于联系cell实体的ENTITYCALL。这个属性是只读的，且如果这个base实体没有关联的cell时属性是None。

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
当前实体所属的客户端（对象）。

类型：

- 只读，PyClientApp

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

如果这个属性的值为True，Entity在地面上，否则为False。
如果是客户端控制的实体该属性将会在改变时同步到服务端，其他实体则由服务端同步到客户端，客户端可以判断这个值来强制贴地避免精度带来的影响。

类型：

- 可读写， bool
