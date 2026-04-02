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

| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| --- | --- |
| [id](#id) | 只读 Integer |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | 只读 uint32 |
| [isOnGround](#isOnGround) | 只读 bool |
| [inWorld](#inWorld) | 只读 bool |
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
调用该实体的base部分的方法。
注意：实体在服务端必须有base部分，在客户端只有客户端控制的玩家实体才可以访问该方法。
例子：
js插件: entity.baseCall("reqCreateAvatar", roleType, name);
c#插件: entity.baseCall("reqCreateAvatar", new object[]{roleType, name});

参数：

| methodName | string，方法名称。 |
| --- | --- |
| methodArgs | objects，方法参数列表。 |

返回：

- 由于是远程调用，不可能阻塞等待返回，因此无返回值。

<a id="cellCall"></a>

### def cellCall(self, methodName, methodArgs):

功能说明：
调用该实体的cell部分的方法。
注意：实体在服务端必须有cell部分，在客户端只有客户端控制的玩家实体才可以访问该方法。
例子：
js插件: entity.cellCall("xxx", roleType, name);
c#插件: entity.cellCall("xxx", new object[]{roleType, name});

参数：

| methodName | string，方法名称。 |
| --- | --- |
| methodArgs | objects，方法参数列表。 |

返回：

- 由于是远程调用，不可能阻塞等待返回，因此无返回值。

<a id="isPlayer"></a>

### def isPlayer(self):

功能说明：
这个函数返回这个Entity是否为当前客户端所控制的Player。

返回：

- bool， 如果是当前客户端所控制的Player返回True，否则返回False。

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

<a id="className"></a>

className

实体的类名。

类型：

- 只读，string

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
