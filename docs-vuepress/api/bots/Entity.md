# Entity类

| KBEngine |

KBEngine
---

# Entity类
Entity类[KBEngine模块]

[[KBEngine模块](../Modules/KBEngine.html)KBEngine模块]Entity是KBEngine模块的一部分。 更多...

Entity是[KBEngine](../Modules/KBEngine.html)KBEngine模块的一部分。[更多...](#detailed_description)更多...
```
import KBEngine
```
import KBEngine
## 成员函数
[成员函数](#)成员函数| def moveToPoint( self, destination, velocity, distance, userData, faceMovement, moveVertically ): |
| --- |
| def cancelController( self, controllerID ): |
| def isPlayer( self ): |
| def getComponent( self, componentName, all ): |
| def fireEvent( self, eventName, *args ): |
| def registerEvent( self, eventName, callback ): |
| def deregisterEvent( self, eventName, callback ): |

def[moveToPoint](#moveToPoint)moveToPoint( self, destination, velocity, distance, userData, faceMovement, moveVertically ):def[cancelController](#cancelController)cancelController( self, controllerID ):def[isPlayer](#isPlayer)isPlayer( self ):def[getComponent](#getComponent)getComponent( self, componentName, all ):def[fireEvent](#fireEvent)fireEvent( self, eventName, *args ):def[registerEvent](#registerEvent)registerEvent( self, eventName, callback ):def[deregisterEvent](#deregisterEvent)deregisterEvent( self, eventName, callback ):
## 回调函数
[回调函数](#)回调函数| def onEnterWorld( self ): |
| --- |
| def onLeaveWorld( self ): |
| def onEnterSpace( self ): |
| def onLeaveSpace( self ): |

def[onEnterWorld](#onEnterWorld)onEnterWorld( self ):def[onLeaveWorld](#onLeaveWorld)onLeaveWorld( self ):def[onEnterSpace](#onEnterSpace)onEnterSpace( self ):def[onLeaveSpace](#onLeaveSpace)onLeaveSpace( self ):
## 属性
[属性](#)属性| base | 只读 ENTITYCALL |
| --- | --- |
| cell | 只读 ENTITYCALL |
| className | 只读 string |
| clientapp | 只读 PyClientApp |
| direction | Tuple of 3 floats as (roll, pitch, yaw) |
| id | 只读 Integer |
| position | Vector3 |
| spaceID | 只读 uint32 |
| isOnGround | 只读 bool |

[base](#base)base只读[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALL[cell](#client)cell只读[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALL[className](#className)className只读[string](STRING.html)string[clientapp](#clientapp)clientapp只读[PyClientApp](../Classes/PyClientApp.html)PyClientApp[direction](#direction)directionTuple of 3 floats as (roll, pitch, yaw)[id](#id)id只读 Integer[position](#position)position[Vector3](../../keywords.html#vector3)Vector3[spaceID](#spaceID)spaceID只读 uint32[isOnGround](#isOnGround)isOnGround只读 bool
---

## 详细描述
详细描述类[Entity](../../keywords.html#entity)Entity的实例代表着在client上的游戏对象。一个[Entity](../../keywords.html#entity)Entity可以通过[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。
---

## 成员函数文档
成员函数文档def moveToPoint( self, destination, velocity, distance, userData, faceMovement, moveVertically ):

def moveToPoint(self, destination, velocity, distance, userData, faceMovement, moveVertically):功能说明：直线移动[Entity](../../keywords.html#entity)Entity到给定的坐标点，成功或失败会调用回调函数。任何[实体](Entity.html)实体，在任意时刻只能有一个移动控制器，重复调用任何移动函数将终止之前的移动控制器。返回一个可以用于取消这次移动的控制器ID。例如：[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController( movementID )。
移动取消还可以调用[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController( "Movement" )。当移动被取消之后通知方法将
不被调用。回调函数如下定义：
```
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):

```
def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):参看：

Entity.cancelController

参看：| Entity.cancelController |

[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController参数：

destination
Vector3，Entity要移动到的目标位置点
velocity
float，Entity的移动速度，单位m/s
distance
float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。

userData
object，传给通知函数的数据

faceMovement
bool，如果实体面向移动方向则为true。如果是其它机制则为false。

moveVertically
bool，设为true指移动为直线移动，设为false指贴着地面移动。

参数：| destination | Vector3，Entity要移动到的目标位置点 |
| --- | --- |
| velocity | float，Entity的移动速度，单位m/s |
| distance | float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。 |
| userData | object，传给通知函数的数据 |
| faceMovement | bool，如果实体面向移动方向则为true。如果是其它机制则为false。 |
| moveVertically | bool，设为true指移动为直线移动，设为false指贴着地面移动。 |

destinationVector3，[Entity](../../keywords.html#entity)Entity要移动到的目标位置点velocityfloat，[Entity](../../keywords.html#entity)Entity的移动速度，单位m/sdistancefloat，距离目标小于该值停止移动，如果该值为0则移动到目标位置。userDataobject，传给通知函数的数据faceMovementbool，如果实体面向移动方向则为true。如果是其它机制则为false。moveVerticallybool，设为true指移动为直线移动，设为false指贴着地面移动。返回：


int，新创建的控制器ID。

返回：| int，新创建的控制器ID。 |

int，新创建的控制器ID。def cancelController( self, controllerID ):

def cancelController(self, controllerID):功能说明：函数cancelController停止一个控制器对[Entity](../../keywords.html#entity)Entity的影响。它只能在一个[real](../../keywords.html#real)real实体上被调用。参数：

controllerID
controllerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。
例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。

参数：| controllerID | controllerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。
例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。 |

controllerIDcontrollerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。
例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。def isPlayer( self ):

def isPlayer(self):功能说明：这个函数返回这个[Entity](../../keywords.html#entity)Entity是否为当前客户端所控制的Player。返回：

bool， 如果是当前客户端所控制的Player返回True，否则返回False。

返回：| bool， 如果是当前客户端所控制的Player返回True，否则返回False。 |

bool， 如果是当前客户端所控制的Player返回True，否则返回False。def getComponent( self, componentName, all ):

def getComponent(self, componentName, all):功能说明：该函数用于获取实体所绑定的某一类组件实例。参数：

componentName
string，组件类型名称，组件的模块名称。

all
bool，如果为True，返回所有同类组件实例，否则只返回第一个或空列表。

参数：| componentName | string，组件类型名称，组件的模块名称。 |
| --- | --- |
| all | bool，如果为True，返回所有同类组件实例，否则只返回第一个或空列表。 |

componentNamestring，组件类型名称，组件的模块名称。allbool，如果为True，返回所有同类组件实例，否则只返回第一个或空列表。def fireEvent( self, eventName, *args ):

def fireEvent(self, eventName, *args):功能说明：该函数用于触发实体事件。参数：

eventName
string，要触发的事件名称。

args
要附带的事件数据，可变参数。

参数：| eventName | string，要触发的事件名称。 |
| --- | --- |
| args | 要附带的事件数据，可变参数。 |

eventNamestring，要触发的事件名称。args要附带的事件数据，可变参数。def registerEvent( self, eventName, callback ):

def registerEvent(self, eventName, callback):功能说明：该函数用于注册实体事件。参数：

eventName
string，要注册监听的事件名称。

callback
当事件触发时，用于响应该事件的回调方法。

参数：| eventName | string，要注册监听的事件名称。 |
| --- | --- |
| callback | 当事件触发时，用于响应该事件的回调方法。 |

eventNamestring，要注册监听的事件名称。callback当事件触发时，用于响应该事件的回调方法。def deregisterEvent( self, eventName, callback ):

def deregisterEvent(self, eventName, callback):功能说明：该函数用于注销监听实体事件。参数：

eventName
string，要注销监听的事件名称。

callback
要注销监听的回调方法。

参数：| eventName | string，要注销监听的事件名称。 |
| --- | --- |
| callback | 要注销监听的回调方法。 |

eventNamestring，要注销监听的事件名称。callback要注销监听的回调方法。
---

## 回调函数文档
回调函数文档def onEnterWorld( self ):

def onEnterWorld(self):如果实体非客户端控制实体，则表明实体进入了服务端上客户端控制的实体的View范围，此时客户端可以看见这个实体了。如果实体是客户端控制的实体则表明该实体已经在服务端创建了cell并进入了space。def onLeaveWorld( self ):

def onLeaveWorld(self):如果实体非客户端控制实体，则表明实体离开了服务端上客户端控制的实体的View范围，此时客户端看不见这个实体了。如果实体是客户端控制的实体则表明该实体已经在服务端销毁了cell并离开了space。def onEnterSpace( self ):

def onEnterSpace(self):客户端控制的实体进入了一个新的space。def onLeaveSpace( self ):

def onLeaveSpace(self):客户端控制的实体离开了当前的space。
---

## 属性文档
属性文档base

basebase是用于联系[Entity](../../baseapp/classes/Entity.html)Entity实体的entityCall。这个属性是只读的，且如果这个实体没有关联的[Entity](../../baseapp/classes/Entity.html)Entity实体时属性是None。其他参考：

Entity.clientEntityEntity.allClientsEntity.otherClients

其他参考：| Entity.clientEntityEntity.allClientsEntity.otherClients |

[Entity](../../keywords.html#entity)Entity.[clientEntity](Entity.html#clientEntity)clientEntity[Entity](../../keywords.html#entity)Entity.[allClients](Entity.html#allClients)allClients[Entity](../../keywords.html#entity)Entity.[otherClients](Entity.html#otherClients)otherClients类型：

只读的，ENTITYCALL

类型：| 只读的，ENTITYCALL |

只读的，[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALLcell

cell说明：cell是用于联系cell实体的[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALL。这个属性是只读的，且如果这个base实体没有关联的cell时属性是None。类型：

只读 ENTITYCALL

类型：| 只读 ENTITYCALL |

只读[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALLcellData

cellData说明：cellData是一个字典属性。每当base实体没有创建它的cell实体时，cell实体的属性会保存在这里。如果cell实体被创建，这些用到的值和[cellData](#cellData)cellData属性将被删除。除了cell实体在实体定义文件里指定的属性外，它还包含position, direction and
spaceID。类型：

CELLDATADICT

类型：| CELLDATADICT |

[CELLDATADICT](CELLDATADICT.html)CELLDATADICTclassName

className说明：实体的类名。类型：

只读，string

类型：| 只读，string |

只读，stringclientapp

clientapp说明：当前实体所属的客户端（对象）。类型：

只读，PyClientApp

类型：| 只读，PyClientApp |

只读，[PyClientApp](../Classes/PyClientApp.html)PyClientAppposition

position这个实体在世界空间中的坐标(x, y, z)，数据由服务端同步到客户端。类型：

Vector3

类型：| Vector3 |

[Vector3](../../keywords.html#vector3)Vector3direction

direction这个属性描述的是[Entity](../../keywords.html#entity)Entity在世界空间中的朝向，数据由服务端同步到客户端。类型：

Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。

类型：| Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。 |

Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。isOnGround

isOnGround如果这个属性的值为True，[Entity](../../keywords.html#entity)Entity在地面上，否则为False。如果是客户端控制的实体该属性将会在改变时同步到服务端，其他实体则由服务端同步到客户端，客户端可以判断这个值来强制贴地避免精度带来的影响。类型：

可读写， bool

类型：| 可读写， bool |

可读写， bool
---
版权归KBEngine所有。

版权归KBEngine所有。