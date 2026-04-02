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
[成员函数](#)成员函数| def accelerate( self, accelerateType, acceleration ): |
| --- |
| def addYawRotator( self, targetYaw, velocity, userArg ): |
| def addProximity( self, range, userArg ): |
| def addTimer( self, start, interval=0.0, userData=0 ): |
| def cancelController( self, controllerID ): |
| def clientEntity( self, destID  ): |
| def canNavigate( self ): |
| def debugView( self ): |
| def delTimer( self, id ): |
| def destroy( self ): |
| def destroySpace( self ): |
| def entitiesInView( self, pending): |
| def entitiesInRange( self, range, entityType=None, position=None ): |
| def isReal( self ): |
| def moveToEntity( self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos ): |
| def moveToPoint( self, destination, velocity, distance, userData, faceMovement, moveVertically ): |
| def getViewRadius( self ): |
| def getViewHystArea( self ): |
| def getRandomPoints( self, centerPos, maxRadius, maxPoints, layer ): |
| def navigate( self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData ): |
| def navigatePathPoints( self,  destination, maxSearchDistance, layer ): |
| def setViewRadius( self, radius, hyst=5 ): |
| def teleport( self, nearbyMBRef, position, direction ): |
| def writeToDB( self, shouldAutoLoad, dbInterfaceName ): |
| def getWitnesses( self ): |
| def getComponent( self, componentName, all ): |
| def fireEvent( self, eventName, *args ): |
| def registerEvent( self, eventName, callback ): |
| def deregisterEvent( self, eventName, callback ): |

def[accelerate](#accelerate)accelerate( self, accelerateType, acceleration ):def[addYawRotator](#addYawRotator)addYawRotator( self, targetYaw, velocity, userArg ):def[addProximity](#addProximity)addProximity( self, range, userArg ):def[addTimer](#addTimer)addTimer( self, start, interval=0.0, userData=0 ):def[cancelController](#cancelController)cancelController( self, controllerID ):def[clientEntity](#clientEntity)clientEntity( self, destID  ):def[canNavigate](#canNavigate)canNavigate( self ):def[debugView](#debugView)debugView( self ):def[delTimer](#delTimer)delTimer( self, id ):def[destroy](#destroy)destroy( self ):def[destroySpace](#destroySpace)destroySpace( self ):def[entitiesInView](#entitiesInView)entitiesInView( self, pending):def[entitiesInRange](#entitiesInRange)entitiesInRange( self, range, entityType=None, position=None ):def[isReal](#isReal)isReal( self ):def[moveToEntity](#moveToEntity)moveToEntity( self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos ):def[moveToPoint](#moveToPoint)moveToPoint( self, destination, velocity, distance, userData, faceMovement, moveVertically ):def[getViewRadius](#getViewRadius)getViewRadius( self ):def[getViewHystArea](#getViewHystArea)getViewHystArea( self ):def[getRandomPoints](#getRandomPoints)getRandomPoints( self, centerPos, maxRadius, maxPoints, layer ):def[navigate](#navigate)navigate( self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData ):def[navigatePathPoints](#navigatePathPoints)navigatePathPoints( self,  destination, maxSearchDistance, layer ):def[setViewRadius](#setViewRadius)setViewRadius( self, radius, hyst=5 ):def[teleport](#teleport)teleport( self, nearbyMBRef, position, direction ):def[writeToDB](#writeToDB)writeToDB( self, shouldAutoLoad, dbInterfaceName ):def[getWitnesses](#getWitnesses)getWitnesses( self ):def[getComponent](#getComponent)getComponent( self, componentName, all ):def[fireEvent](#fireEvent)fireEvent( self, eventName, *args ):def[registerEvent](#registerEvent)registerEvent( self, eventName, callback ):def[deregisterEvent](#deregisterEvent)deregisterEvent( self, eventName, callback ):
## 回调函数
[回调函数](#)回调函数| def onDestroy( self ): |
| --- |
| def onEnterTrap( self, entity, rangeXZ, rangeY, controllerID, userArg ): |
| def onEnteredView( self, entity ): |
| def onGetWitness( self ): |
| def onLeaveTrap( self, entity, rangeXZ, rangeY, controllerID, userArg ): |
| def onLoseControlledBy( self, id ): |
| def onLoseWitness( self ): |
| def onMove( self, controllerID, userData ): |
| def onMoveOver( self, controllerID, userData ): |
| def onMoveFailure( self, controllerID, userData ): |
| def onRestore( self ): |
| def onSpaceGone( self ): |
| def onTurn( self, controllerID, userData ): |
| def onTeleport( self ): |
| def onTeleportFailure( self ): |
| def onTeleportSuccess( self, nearbyEntity ): |
| def onTimer( self, timerHandle, userData ): |
| def onUpdateBegin( self ): |
| def onUpdateEnd( self ): |
| def onWitnessed( self, isWitnessed ): |
| def onWriteToDB( self ): |

def[onDestroy](#onDestroy)onDestroy( self ):def[onEnterTrap](#onEnterTrap)onEnterTrap( self, entity, rangeXZ, rangeY, controllerID, userArg ):def[onEnteredView](#onEnteredView)onEnteredView( self, entity ):def[onGetWitness](#onGetWitness)onGetWitness( self ):def[onLeaveTrap](#onLeaveTrap)onLeaveTrap( self, entity, rangeXZ, rangeY, controllerID, userArg ):def[onLoseControlledBy](#onLoseControlledBy)onLoseControlledBy( self, id ):def[onLoseWitness](#onLoseWitness)onLoseWitness( self ):def[onMove](#onMove)onMove( self, controllerID, userData ):def[onMoveOver](#onMoveOver)onMoveOver( self, controllerID, userData ):def[onMoveFailure](#onMoveFailure)onMoveFailure( self, controllerID, userData ):def[onRestore](#onRestore)onRestore( self ):def[onSpaceGone](#onSpaceGone)onSpaceGone( self ):def[onTurn](#onTurn)onTurn( self, controllerID, userData ):def[onTeleport](#onTeleport)onTeleport( self ):def[onTeleportFailure](#onTeleportFailure)onTeleportFailure( self ):def[onTeleportSuccess](#onTeleportSuccess)onTeleportSuccess( self, nearbyEntity ):def[onTimer](#onTimer)onTimer( self, timerHandle, userData ):def[onUpdateBegin](#onUpdateBegin)onUpdateBegin( self ):def[onUpdateEnd](#onUpdateEnd)onUpdateEnd( self ):def[onWitnessed](#onWitnessed)onWitnessed( self, isWitnessed ):def[onWriteToDB](#onWriteToDB)onWriteToDB( self ):
## 属性
[属性](#)属性| allClients | 只读 PyClient |
| --- | --- |
| base | 只读 BaseEntityCall |
| client | 只读 ClientEntityCall |
| controlledBy | BaseEntityCall |
| className | 只读 string |
| direction | Tuple of 3 floats as (roll, pitch, yaw) |
| hasWitness | boolean |
| id | 只读 Integer |
| isDestroyed | 只读 bool |
| isWitnessed | 只读 bool |
| layer | int8 |
| otherClients | 只读 PyClient |
| position | Vector3 |
| spaceID | 只读 uint32 |
| topSpeed | float |
| topSpeedY | float |
| volatileInfo | float |

[allClients](#allClients)allClients只读 PyClient[base](#base)base只读[BaseEntityCall](../../keywords.html#EntityCall)BaseEntityCall[client](#client)client只读[ClientEntityCall](../../keywords.html#EntityCall)ClientEntityCall[controlledBy](#controlledBy)controlledBy[BaseEntityCall](../../keywords.html#EntityCall)BaseEntityCall[className](#className)className只读[string](STRING.html)string[direction](#direction)directionTuple of 3 floats as (roll, pitch, yaw)[hasWitness](#hasWitness)hasWitnessboolean[id](#id)id只读 Integer[isDestroyed](#isDestroyed)isDestroyed只读 bool[isWitnessed](#isWitnessed)isWitnessed只读 bool[layer](#layer)layerint8[otherClients](#otherClients)otherClients只读 PyClient[position](#position)position[Vector3](../../keywords.html#vector3)Vector3[spaceID](#spaceID)spaceID只读 uint32[topSpeed](#topSpeed)topSpeedfloat[topSpeedY](#topSpeedY)topSpeedYfloat[volatileInfo](#volatileInfo)volatileInfofloat
---

## 详细描述
详细描述类[Entity](../../keywords.html#entity)Entity的实例代表着在cell上的游戏对象。一个[Entity](../../keywords.html#entity)Entity可以是"real"或者"ghosted"的，一个"ghost"[Entity](../../keywords.html#entity)Entity是一个存活在邻近的cell上
的"real"[Entity](../../keywords.html#entity)Entity的拷贝。对于每一个实体来说有一个唯一的"real"[Entity](../../keywords.html#entity)Entity实例，和有0个
或者更多的"ghost"[Entity](../../keywords.html#entity)Entity实例。一个[Entity](../../keywords.html#entity)Entity实例操控着实体的位置数据，包括他的空间和旋转。它还控制着这些数据发给
客户端的频率（如果可以）。位置的数据可以被唯一的客户端所更新，被控制器对象，被teleport成员函数修改。
控制器是非python对象，可以适用在cell实体上随着时间的过去来改变它们的位置数据，它们通过成员函数如"trackEntity"和"turnToYaw"来
添加到[Entity](../../keywords.html#entity)Entity，可以通过"cancelController"移除。感兴趣的范围，或"View"对于所有属于客户端的[KBEngine](KBEngine.html)KBEngine实体来说是一个重要的概念。
一个实体的View是围绕这个实体的客户端（如果它有）所能感知的区域。这用于选择发给客户端的数据量。View的实际
形状由x轴和z轴上的距离范围定义，还有一个类似形状向外延伸的滞后区域。一个[Entity](../../keywords.html#entity)Entity进入
另一个[Entity](../../keywords.html#entity)Entity的View，但不会离开它直到它离开滞后区域。一个[Entity](../../keywords.html#entity)Entity可以
通过"setViewRadius"修改它的View大小。
可以通过"entitiesInRange"找到一个具体距离之内的所有实体，通过"addProximity"设置一个陷阱捕获进入陷阱的所有实体。cellApp上新的[Entity](../../keywords.html#entity)Entity可以使用[KBEngine](KBEngine.html)KBEngine.[createEntity](KBEngine.html#createEntity)createEntity创建。一个实体还可以通过baseApp远程调用[KBEngine](KBEngine.html)KBEngine.[createCellEntity](KBEngine.html#createCellEntity)createCellEntity函数
来创建。一个[Entity](../../keywords.html#entity)Entity可以通过[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALL访问在base和client应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。
---

## 成员函数文档
成员函数文档def accelerate( self, accelerateType, acceleration ):

def accelerate(self, accelerateType, acceleration):功能说明：加速实体当前运动。可影响的运动包括：

Entity.moveToEntity
Entity.moveToPoint
Entity.navigate
Entity.addYawRotator

可影响的运动包括：| Entity.moveToEntity |
| --- |
| Entity.moveToPoint |
| Entity.navigate |
| Entity.addYawRotator |

[Entity](../../keywords.html#entity)Entity.[moveToEntity](Entity.html#moveToEntity)moveToEntity[Entity](../../keywords.html#entity)Entity.[moveToPoint](Entity.html#moveToPoint)moveToPoint[Entity](../../keywords.html#entity)Entity.[navigate](Entity.html#navigate)navigate[Entity](../../keywords.html#entity)Entity.[addYawRotator](Entity.html#addYawRotator)addYawRotator参数：

accelerateType
string，影响的运动类型，如：Movement、Turn。

velocity
float，每秒的加速度，如果传入的是负值则表示减速度。

参数：| accelerateType | string，影响的运动类型，如：Movement、Turn。 |
| --- | --- |
| velocity | float，每秒的加速度，如果传入的是负值则表示减速度。 |

accelerateTypestring，影响的运动类型，如：Movement、Turn。velocityfloat，每秒的加速度，如果传入的是负值则表示减速度。返回：


影响后的实体当前速度。

返回：| 影响后的实体当前速度。 |

影响后的实体当前速度。def addYawRotator( self, targetYaw, velocity, userArg ):

def addYawRotator(self, targetYaw, velocity, userArg):功能说明：控制实体绕yaw旋转，旋转完成将会通过[Entity](../../keywords.html#entity)Entity.[onTurn](Entity.html#onTurn)onTurn通知。使用[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController带上控制器ID或者使用[Entity](../../keywords.html#entity)Entity.[cancelController("Movement")](Entity.html#cancelController)cancelController("Movement")来删除它。参看：

Entity.cancelController

参看：| Entity.cancelController |

[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController参数：

targetYaw
float，给定的目标yaw弧度。

velocity
float，旋转时每秒的弧度。

userArg
是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议
在回调原型里设置默认值为0。

参数：| targetYaw | float，给定的目标yaw弧度。 |
| --- | --- |
| velocity | float，旋转时每秒的弧度。 |
| userArg | 是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议
在回调原型里设置默认值为0。 |

targetYawfloat，给定的目标yaw弧度。velocityfloat，旋转时每秒的弧度。userArg是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议
在回调原型里设置默认值为0。def addProximity( self, rangeXZ, rangeY, userArg ):

def addProximity(self, rangeXZ, rangeY, userArg):功能说明：创建一个范围触发器，当有其它实体进入或离开这个触发器区域的时候会通知这个[Entity](../../keywords.html#entity)Entity。这个区域是一个方形（为了效率）。如果其它实体在x轴和z轴上均在给定的距离里面，则实体被视为在这个范围里面。这个[Entity](../../keywords.html#entity)Entity通过onEnterTrap和onLeaveTrap函数被通知，这两个函数可以如下定义：
```
	def onEnterTrap( self, entityEntering, rangeXZ, rangeY, controllerID, userArg = 0 ):
	def onLeaveTrap( self, entityLeaving, rangeXZ, rangeY, controllerID, userArg = 0 ):

```
def onEnterTrap( self, entityEntering, rangeXZ, rangeY, controllerID, userArg = 0 ):
	def onLeaveTrap( self, entityLeaving, rangeXZ, rangeY, controllerID, userArg = 0 ):由于这个范围触发器是一个控制器，使用[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController带上控制器ID来删除它。需要注意的是回调有可能会立刻被触发，即使在addProximity()调用返回之前。参看：

Entity.cancelController

参看：| Entity.cancelController |

[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController参数：

rangeXZ
float，给定触发器xz轴区域的大小，必须大于等于0。

rangeY
float，给定触发器y轴高度，必须大于等于0。
需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y

开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。
3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。

userArg
是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议
在回调原型里设置默认值为0。

参数：| rangeXZ | float，给定触发器xz轴区域的大小，必须大于等于0。 |
| --- | --- |
| rangeY | float，给定触发器y轴高度，必须大于等于0。
需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y

开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。
3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。 |
| userArg | 是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议
在回调原型里设置默认值为0。 |

rangeXZfloat，给定触发器xz轴区域的大小，必须大于等于0。rangeYfloat，给定触发器y轴高度，必须大于等于0。需要注意的是，这个参数要生效必须开放[kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。userArg是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议
在回调原型里设置默认值为0。返回：


返回创建控制器的id。

返回：| 返回创建控制器的id。 |

返回创建控制器的id。def addTimer( self, start, interval=0.0, userData=0 ):

def addTimer(self, start, interval=0.0, userData=0):功能说明：注册一个定时器，定时器由回调函数onTimer[onTimer](#onTimer)onTimer触发，回调函数将在"initialOffset"秒后被执行第1次，而后将每间隔"repeatOffset"秒执行1次，可设定一个用户参数"userArg"（仅限integer类型）。onTimer[onTimer](#onTimer)onTimer函数必须在entity的cell部分被定义，且带有2个参数，第1个integer类型的是timer的id（可用于移除timer的"[delTimer](#delTimer)delTimer"函数），第2个是用户参数"userArg"。例子:
```
# 这里是使用addTimer的一个例子
import KBEngine
 
class MyCellEntity( KBEngine.Entity ):
 
    def __init__( self ):
        KBEngine.Entity.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    # Entity的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyCellEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if 这是不断重复的定时器，当不再需要该定时器的时候，调用下面函数移除:
        #     self.delTimer( id )

```

```
# 这里是使用addTimer的一个例子
import KBEngine
 
class MyCellEntity( KBEngine.Entity ):
 
    def __init__( self ):
        KBEngine.Entity.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    # Entity的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyCellEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if 这是不断重复的定时器，当不再需要该定时器的时候，调用下面函数移除:
        #     self.delTimer( id )
```
# 这里是使用addTimer的一个例子
import[KBEngine](../Modules/KBEngine.html)KBEngineclass MyCellEntity([KBEngine](../Modules/KBEngine.html)KBEngine.[Entity](../../keywords.html#entity)Entity):
 
    def __init__( self ):[KBEngine](../Modules/KBEngine.html)KBEngine.[Entity](../../keywords.html#entity)Entity.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    #[Entity](../../keywords.html#entity)Entity的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyCellEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if 这是不断重复的定时器，当不再需要该定时器的时候，调用下面函数移除:
        #     self.delTimer( id )参数：

initialOffset
float，指定定时器从注册到第一次回调的时间间隔（秒）。

repeatOffset
float，指定第一次回调执行后每次执行的时间间隔（秒）。必须用函数delTimer移除定时器，否则它会一直重复下去。值小于等于0将被忽略。

userArg
integer，指定底层回调"onTimer"时的userArg参数值。

参数：| initialOffset | float，指定定时器从注册到第一次回调的时间间隔（秒）。 |
| --- | --- |
| repeatOffset | float，指定第一次回调执行后每次执行的时间间隔（秒）。必须用函数delTimer移除定时器，否则它会一直重复下去。值小于等于0将被忽略。 |
| userArg | integer，指定底层回调"onTimer"时的userArg参数值。 |

initialOffsetfloat，指定定时器从注册到第一次回调的时间间隔（秒）。repeatOffsetfloat，指定第一次回调执行后每次执行的时间间隔（秒）。必须用函数[delTimer](#delTimer)delTimer移除定时器，否则它会一直重复下去。值小于等于0将被忽略。userArginteger，指定底层回调"[onTimer](#onTimer)onTimer"时的userArg参数值。返回:

integer，该函数返回timer的内部id，这个id可用于delTimer移除定时器。

返回:| integer，该函数返回timer的内部id，这个id可用于delTimer移除定时器。 |

integer，该函数返回timer的内部id，这个id可用于[delTimer](#delTimer)delTimer移除定时器。def cancelController( self, controllerID ):

def cancelController(self, controllerID):功能说明：函数cancelController停止一个控制器对[Entity](../../keywords.html#entity)Entity的影响。它只能在一个[real](../../keywords.html#real)real实体上被调用。参数：

controllerID
controllerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。
例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。

参数：| controllerID | controllerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。
例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。 |

controllerIDcontrollerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。
例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。def clientEntity( self, destID ):

def clientEntity(self, destID):功能说明：通过这个方法可以访问自己客户端（当前实体必须绑定了客户端）中某个实体的方法，只有在View范围内的实体才会同步到客户端。它只能在一个[real](../../keywords.html#real)real实体上被调用。参数：

destID
目标实体的ID。

参数：| destID | 目标实体的ID。 |

destID目标实体的ID。def canNavigate( self ):

def canNavigate(self):功能说明：通过这个方法判断当前实体是否可以使用导航（[Entity](../../keywords.html#entity)Entity.[navigate](Entity.html#navigate)navigate）功能。它只能在一个[real](../../keywords.html#real)real实体上被调用。通常当实体所在Space使用[Entity](../../keywords.html#entity)Entity.[addSpaceGeometryMapping](Entity.html#addSpaceGeometryMapping)addSpaceGeometryMapping加载过有效的导航用的碰撞数据（Navmesh或者2D的tile数据）并且实体在有效导航区域该功能可用。返回：

bool， 如果实体可在当前Space中使用导航功能返回True，否则返回False。

返回：| bool， 如果实体可在当前Space中使用导航功能返回True，否则返回False。 |

bool， 如果实体可在当前Space中使用导航功能返回True，否则返回False。def debugView( self ):

def debugView(self):功能说明：debugView输出[Entity](../../keywords.html#entity)Entity的View的详细信息到cell的调试日志。
一份View系统工作的描述可以在[Entity](../../keywords.html#entity)Entity类文档中找到。一份样品信息如下：
```
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 size=4, Seen=4, Pending=0, ViewRadius=50.000, ViewHyst=5.000
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(102), position(771.586.211.002.776.55), dist=0
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Monster(1028), position(820.834.211.635.768.749), dist=49.8659
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 NPC(1025), position(784.024.210.95.782.273), dist=13.6915
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(106), position(771.586.211.002.776.55), dist=0

```
INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 size=4, Seen=4, Pending=0, ViewRadius=50.000, ViewHyst=5.000
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(102), position(771.586.211.002.776.55), dist=0
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Monster(1028), position(820.834.211.635.768.749), dist=49.8659
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 NPC(1025), position(784.024.210.95.782.273), dist=13.6915
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(106), position(771.586.211.002.776.55), dist=0信息的第一行告诉我们：
- 实体#1000的数据
实体#1000的数据- 有4个实体在它的View区域并且已经同步给客户端。
有4个实体在它的View区域并且已经同步给客户端。- 有0个实体在它的View区域，正在等待同步到客户端。
有0个实体在它的View区域，正在等待同步到客户端。- View的半径是 50.000
View的半径是 50.000- View的滞后区域向外延伸了5.000
View的滞后区域向外延伸了5.000def delTimer( self, id ):

def delTimer(self, id):功能说明：函数delTimer用于移除一个注册的定时器，移除后的定时器不再执行。只执行1次的定时器在执行回调后自动移除，不必要使用delTimer移除。
如果delTimer函数使用一个无效的id（例如已经移除），将会产生错误。参数：

id
integer，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。

参数：| id | integer，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。 |

idinteger，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。def destroy( self ):

def destroy(self):功能说明：这个函数销毁它的本地[Entity](../../keywords.html#entity)Entity实例，如果实体在其他进程上存在[ghost](../../keywords.html#ghost)ghost部分也会同时通知销毁。
这个函数最好由实体自己调用，如果这个实体是一个[ghost](../../keywords.html#ghost)ghost则会抛出一个异常。如果回调函数onDestroy()被实现则被调用。def destroySpace( self ):

def destroySpace(self):功能说明：销毁这个实体所在的[空间](../../keywords.html#cn_Space)空间。def entitiesInView( self, pending ):

def entitiesInView(self, pending):功能说明：获得这个实体的[View](../../keywords.html#View)View范围内的实体列表。参数：

pending
bool，可选参数，默认值为False，只返回客户端可见的所有实体，否则返回服务端可见但未同步到客户端的所有实体。

参数：| pending | bool，可选参数，默认值为False，只返回客户端可见的所有实体，否则返回服务端可见但未同步到客户端的所有实体。 |

pendingbool，可选参数，默认值为False，只返回客户端可见的所有实体，否则返回服务端可见但未同步到客户端的所有实体。def entitiesInRange( self, range, entityType=None, position=None ):

def entitiesInRange(self, range, entityType=None, position=None):功能说明：在给定的距离内搜索[实体](Entity.html)实体。这是一个球形的搜索，3个轴的距离
都要测量。这可以找到在这个[实体](Entity.html)实体的[View](../../keywords.html#View)View范围之外的实体，但不能找到其他[cell](../../keywords.html#cell)cell的实体。例子：
```
  self.entitiesInRange( 100, 'Creature', (100, 0, 100) )

```
self.entitiesInRange( 100, 'Creature', (100, 0, 100) )搜索到‘Creature’类型的实体列表（‘Creature’的子类实例化的实体）。中心点是(100, 0, 100)，搜索半径是100米。
```
  [ e for e in self.entitiesInRange( 100, None, (100,0,100) ) if isinstance( e, BaseType ) ]

```
[ e for e in self.entitiesInRange( 100, None, (100,0,100) ) if isinstance( e, BaseType ) ]将给出一个来自‘BaseType’或‘BaseType’的子类实例化的实体列表。参数：

range
围绕这个实体搜索的距离，float类型

entityType
一个可选的字符串参数，实体的类型名称，用于匹配实体。如果实体类型是一个有效的类名（ 有效的实体类型在/scripts/entities.xml列出 ），
则只有这个类型的实体会被返回，否则将这个范围的所有实体都返回。

position
一个可选的Vector3类型参数，作为搜索半径的中心, 默认以实体自身为中心。

参数：| range | 围绕这个实体搜索的距离，float类型 |
| --- | --- |
| entityType | 一个可选的字符串参数，实体的类型名称，用于匹配实体。如果实体类型是一个有效的类名（ 有效的实体类型在/scripts/entities.xml列出 ），
则只有这个类型的实体会被返回，否则将这个范围的所有实体都返回。 |
| position | 一个可选的Vector3类型参数，作为搜索半径的中心, 默认以实体自身为中心。 |

range围绕这个实体搜索的距离，float类型entityType一个可选的字符串参数，实体的类型名称，用于匹配实体。如果实体类型是一个有效的类名（ 有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出 ），则只有这个类型的实体会被返回，否则将这个范围的所有实体都返回。position一个可选的[Vector3](../../keywords.html#vector3)Vector3类型参数，作为搜索半径的中心, 默认以实体自身为中心。返回：


在给定范围内的Entity对象列表。

返回：| 在给定范围内的Entity对象列表。 |

在给定范围内的[Entity](../../keywords.html#entity)Entity对象列表。def isReal( self ):

def isReal(self):功能说明：这个函数返回这个[Entity](../../keywords.html#entity)Entity是[real](../../keywords.html#real)real的还是一个[ghost](../../keywords.html#ghost)ghost的。这个函数很少被用到但对调试很有用。返回：

bool， 如果是real实体返回True，否则返回False。

返回：| bool， 如果是real实体返回True，否则返回False。 |

bool， 如果是real实体返回True，否则返回False。def moveToEntity( self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos ):

def moveToEntity(self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos):功能说明：直线移动[实体](Entity.html)实体到另一个[Entity](../../keywords.html#entity)Entity位置。任何[实体](Entity.html)实体，在任意时刻只能有一个移动控制器，重复调用任何移动函数将终止之前的移动控制器。调用后函数将返回一个可以用于取消这次移动的控制器ID。例如，[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController( movementID )。
移动取消还可以调用[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController( "Movement" )。当移动被取消之后回调通知方法将不被调用。
```
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):

```
def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):参考:

Entity.cancelController

参考:| Entity.cancelController |

[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController参数：

destEntityID
int，目标Entity的ID

velocity
float，Entity移动的速度，单位m/s

distance
float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。

userData
object，可选参数，回调通知被调用时其中userData参数将为此值。

faceMovement
bool，可选参数，如果实体面向移动方向则为true。如果是其它机制则为false。

moveVertically
bool，可选参数，设为True指移动为直线移动，设为False指贴着地面直线移动。

offsetPos
Vector3，可选参数，设置一定的偏移值，例如使移动目标位置位于实体的左侧。

参数：| destEntityID | int，目标Entity的ID |
| --- | --- |
| velocity | float，Entity移动的速度，单位m/s |
| distance | float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。 |
| userData | object，可选参数，回调通知被调用时其中userData参数将为此值。 |
| faceMovement | bool，可选参数，如果实体面向移动方向则为true。如果是其它机制则为false。 |
| moveVertically | bool，可选参数，设为True指移动为直线移动，设为False指贴着地面直线移动。 |
| offsetPos | Vector3，可选参数，设置一定的偏移值，例如使移动目标位置位于实体的左侧。 |

destEntityIDint，目标[Entity](../../keywords.html#entity)Entity的IDvelocityfloat，[Entity](../../keywords.html#entity)Entity移动的速度，单位m/sdistancefloat，距离目标小于该值停止移动，如果该值为0则移动到目标位置。userDataobject，可选参数，回调通知被调用时其中userData参数将为此值。faceMovementbool，可选参数，如果实体面向移动方向则为true。如果是其它机制则为false。moveVerticallybool，可选参数，设为True指移动为直线移动，设为False指贴着地面直线移动。offsetPosVector3，可选参数，设置一定的偏移值，例如使移动目标位置位于实体的左侧。返回：


int，新创建的控制器ID。

返回：| int，新创建的控制器ID。 |

int，新创建的控制器ID。def moveToPoint( self, destination, velocity, distance, userData, faceMovement, moveVertically ):

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

int，新创建的控制器ID。def getViewRadius( self ):

def getViewRadius(self):功能说明：这个函数返回这个[Entity](../../keywords.html#entity)Entity当前的View半径值。数据可以通过[Entity](../../keywords.html#entity)Entity.[setViewRadius](Entity.html#setViewRadius)setViewRadius( radius, hyst )设置。返回：

float， View半径。

返回：| float， View半径。 |

float， View半径。def getViewHystArea( self ):

def getViewHystArea(self):功能说明：这个函数返回这个[Entity](../../keywords.html#entity)Entity的View当前滞后区域值。数据可以通过[Entity](../../keywords.html#entity)Entity.[setViewRadius](Entity.html#setViewRadius)setViewRadius( radius, hyst )设置。返回：

float， View当前滞后区域值。

返回：| float， View当前滞后区域值。 |

float， View当前滞后区域值。def getRandomPoints( self, centerPos, maxRadius, maxPoints, layer ):

def getRandomPoints(self, centerPos, maxRadius, maxPoints, layer):功能说明：这个函数用于获取以某个坐标点为中心一定区域内[Entity](../../keywords.html#entity)Entity.[navigate](Entity.html#navigate)navigate可到达的随机坐标点。参数：

centerPos
Vector3，Entity中心坐标点

maxRadius
float，最大的搜索半径

maxPoints
uint32，最多返回的随机坐标点数量。

layer
int8，使用某个层的navmesh来搜索。

参数：| centerPos | Vector3，Entity中心坐标点 |
| --- | --- |
| maxRadius | float，最大的搜索半径 |
| maxPoints | uint32，最多返回的随机坐标点数量。 |
| layer | int8，使用某个层的navmesh来搜索。 |

centerPosVector3，[Entity](../../keywords.html#entity)Entity中心坐标点maxRadiusfloat，最大的搜索半径maxPointsuint32，最多返回的随机坐标点数量。layerint8，使用某个层的navmesh来搜索。返回：


tuple，包含一个或者多个坐标的数组。

返回：| tuple，包含一个或者多个坐标的数组。 |

tuple，包含一个或者多个坐标的数组。def navigate( self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData ):

def navigate(self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData):功能说明：使用导航系统来使这个[Entity](../../keywords.html#entity)Entity向一个目标点移动，成功或失败会调用回调函数。[KBEngine](KBEngine.html)KBEngine可以有数个预先生成好的导航网格，不同的网格大小（会导致不同的导航路径）。任何[实体](Entity.html)实体，在任意时刻只能有一个移动控制器，重复调用任何移动函数将终止之前的移动控制器。返回一个可以用于取消这次移动的控制器ID。例如：[Entity](../../keywords.html#entity)Entity.[cancelController](Entity.html#cancelController)cancelController( movementID )。
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
Vector3，Entity移向的目标点。

velocity
float，Entity的移动速度，单位m/s

distance
float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。

maxMoveDistance
float，最大的移动距离。

maxSearchDistance
float，从导航数据中最大搜索距离。

faceMovement
bool，如果实体面向移动方向则为true（默认）。如果是其它机制则为false。

layer
int8，使用某个层的navmesh来寻路。

userData
object，传给通知函数的数据。

参数：| destination | Vector3，Entity移向的目标点。 |
| --- | --- |
| velocity | float，Entity的移动速度，单位m/s |
| distance | float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。 |
| maxMoveDistance | float，最大的移动距离。 |
| maxSearchDistance | float，从导航数据中最大搜索距离。 |
| faceMovement | bool，如果实体面向移动方向则为true（默认）。如果是其它机制则为false。 |
| layer | int8，使用某个层的navmesh来寻路。 |
| userData | object，传给通知函数的数据。 |

destinationVector3，[Entity](../../keywords.html#entity)Entity移向的目标点。velocityfloat，[Entity](../../keywords.html#entity)Entity的移动速度，单位m/sdistancefloat，距离目标小于该值停止移动，如果该值为0则移动到目标位置。maxMoveDistancefloat，最大的移动距离。maxSearchDistancefloat，从导航数据中最大搜索距离。faceMovementbool，如果实体面向移动方向则为true（默认）。如果是其它机制则为false。layerint8，使用某个层的navmesh来寻路。userDataobject，传给通知函数的数据。返回：


int，新创建的控制器ID。

返回：| int，新创建的控制器ID。 |

int，新创建的控制器ID。def navigatePathPoints( self, destination, maxSearchDistance, layer ):

def navigatePathPoints(self, destination, maxSearchDistance, layer):功能说明：这个函数返回有当前[Entity](../../keywords.html#entity)Entity的位置到destination位置所经过的路径点列表。参数：

destination
Vector3，Entity移向的目标点

maxSearchDistance
float，最大的搜索距离

layer
int8，使用某个层的navmesh来查找路径列表。

参数：| destination | Vector3，Entity移向的目标点 |
| --- | --- |
| maxSearchDistance | float，最大的搜索距离 |
| layer | int8，使用某个层的navmesh来查找路径列表。 |

destinationVector3，[Entity](../../keywords.html#entity)Entity移向的目标点maxSearchDistancefloat，最大的搜索距离layerint8，使用某个层的navmesh来查找路径列表。def setViewRadius( self, radius, hyst=5 ):

def setViewRadius(self, radius, hyst=5):功能说明：指定[Entity](../../keywords.html#entity)Entity的[感兴趣的区域](../../keywords.html#View)感兴趣的区域大小。这个函数只能用于有[Witness](../../keywords.html#Witness)Witness关联的实体。注意：
你可以通过设置[kbengine.xml](../../keywords.html#kbengine.xml)kbengine.xml配置选项'cellapp/defaultViewRadius'来设置默认的View半径。View半径默认是不能大于500.0的。详情请看[kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml配置选项'cellapp/entity_posdir_updates'。数据可以通过[Entity](../../keywords.html#entity)Entity.[getViewRadius](Entity.html#getViewRadius)getViewRadius( )与[Entity](../../keywords.html#entity)Entity.[getViewHystArea](Entity.html#getViewHystArea)getViewHystArea( )获得。参数：

radius
float，radius指定View区域的半径

hyst
float，指定超过View区域的滞后区域的大小。合理的设定滞后区域将能够降低View碰撞的敏感度从而提高CPU执行效率。
一个实体进入另一个实体的View必须跨越View半径区域，但实体离开View区域则需要移出View半径区域包括滞后区域。

参数：| radius | float，radius指定View区域的半径 |
| --- | --- |
| hyst | float，指定超过View区域的滞后区域的大小。合理的设定滞后区域将能够降低View碰撞的敏感度从而提高CPU执行效率。
一个实体进入另一个实体的View必须跨越View半径区域，但实体离开View区域则需要移出View半径区域包括滞后区域。 |

radiusfloat，radius指定View区域的半径hystfloat，指定超过View区域的滞后区域的大小。合理的设定滞后区域将能够降低View碰撞的敏感度从而提高CPU执行效率。一个实体进入另一个实体的View必须跨越View半径区域，但实体离开View区域则需要移出View半径区域包括滞后区域。返回：


None

返回：| None |

Nonedef teleport( self, nearbyMBRef, position, direction ):

def teleport(self, nearbyMBRef, position, direction):功能说明：瞬间移动一个[Entity](../../keywords.html#entity)Entity到一个指定的空间。这个函数允许指定实体移动后的位置与朝向。如果需要在不同[空间](../../keywords.html#cn_Space)空间跳转（ 通常用于不同场景或者房间跳转 ），可以传一个[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall给这个函数（ 这个entityCall所对应的实体必须在目的[空间](../../keywords.html#cn_Space)空间中 ）。这个函数只能在[real](../../keywords.html#real)real的实体上被调用。参数：

nearbyMBRef
一个决定Entity跳往哪个Space的CellEntityCall（ 这个entityCall所对应的实体必须在目的Space中 ），它被认为是传送目的地。
这个可以设为None，在这种情形下它会在当前的cell完成瞬移。

position
Entity瞬移后的坐标，是一个有3个float(x, y, z)组成的序列。

direction
Entity瞬移后的朝向，是一个由3个float组成的序列(roll,pitch, yaw)。

参数：| nearbyMBRef | 一个决定Entity跳往哪个Space的CellEntityCall（ 这个entityCall所对应的实体必须在目的Space中 ），它被认为是传送目的地。
这个可以设为None，在这种情形下它会在当前的cell完成瞬移。 |
| --- | --- |
| position | Entity瞬移后的坐标，是一个有3个float(x, y, z)组成的序列。 |
| direction | Entity瞬移后的朝向，是一个由3个float组成的序列(roll,pitch, yaw)。 |

nearbyMBRef一个决定[Entity](../../keywords.html#entity)Entity跳往哪个[Space](../../keywords.html#cSpace)Space的[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall（ 这个entityCall所对应的实体必须在目的[Space](../../keywords.html#cSpace)Space中 ），它被认为是传送目的地。
这个可以设为None，在这种情形下它会在当前的cell完成瞬移。position[Entity](../../keywords.html#entity)Entity瞬移后的坐标，是一个有3个float(x, y, z)组成的序列。direction[Entity](../../keywords.html#entity)Entity瞬移后的朝向，是一个由3个float组成的序列(roll,pitch, yaw)。def writeToDB( self, shouldAutoLoad, dbInterfaceName ):

def writeToDB(self, shouldAutoLoad, dbInterfaceName):功能说明：这个函数保存与这个实体相关的数据到数据库，包括base实体的数据。在数据确认传到数据库之前base实体的onWriteToDB函数会被调用。cell实体的数据同时备份在base实体，确保遇到灾难恢复数据时数据是最新的。这个函数只能在[real](../../keywords.html#real)real实体且实体必须存在base部分时才允许被调用。参数：

shouldAutoLoad
这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。
注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。
脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如：
def onAutoLoadEntityCreate(entityType, dbid): 
          KBEngine.createEntityFromDBID(entityType, dbid)

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| shouldAutoLoad | 这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。
注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。
脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如：
def onAutoLoadEntityCreate(entityType, dbid): 
          KBEngine.createEntityFromDBID(entityType, dbid) |
| --- | --- |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

shouldAutoLoad这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如：def onAutoLoadEntityCreate(entityType, dbid):KBEngine.createEntityFromDBID(entityType, dbid)dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。def getWitnesses( self ):

def getWitnesses(self):功能说明：这个函数返回观察该[Entity](../../keywords.html#entity)Entity的所有观察者(玩家)。返回：

tuple， 包含零个或者多个Entity的数组。

返回：| tuple， 包含零个或者多个Entity的数组。 |

tuple， 包含零个或者多个[Entity](../../keywords.html#entity)Entity的数组。def getComponent( self, componentName, all ):

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
回调函数文档def onDestroy( self ):

def onDestroy(self):如果这个函数在脚本中有实现，这个函数在调用[Entity](Entity.html)Entity.[destroy](Entity.html#destroy)destroy()后，在实际销毁之前被调用。
这个函数没有参数。def onEnterTrap( self, entity, rangeXZ, rangeY, controllerID, userArg ):

def onEnterTrap(self, entity, rangeXZ, rangeY, controllerID, userArg):当注册了使用[Entity](../../keywords.html#entity)Entity.[addProximity](Entity.html#addProximity)addProximity注册了一个范围触发器，有其他实体进入触发器时，该回调函数被调用。参数：

entity 已经进入了范围的实体。

rangeXZ
float，给定触发器xz轴区域的大小，必须大于等于0。

rangeY
float，给定触发器y轴高度，必须大于等于0。
需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y

开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。
3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。

controllerID 这个触发器的控制器id。

userArg 这个参数的值由用户调用addProximity时给出，用户可以根据此参数对当前行为做一些判断。

参数：| entity | 已经进入了范围的实体。 |
| --- | --- |
| rangeXZ | float，给定触发器xz轴区域的大小，必须大于等于0。 |
| rangeY | float，给定触发器y轴高度，必须大于等于0。
需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y

开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。
3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。 |
| controllerID | 这个触发器的控制器id。 |
| userArg | 这个参数的值由用户调用addProximity时给出，用户可以根据此参数对当前行为做一些判断。 |

entity已经进入了范围的实体。rangeXZfloat，给定触发器xz轴区域的大小，必须大于等于0。rangeYfloat，给定触发器y轴高度，必须大于等于0。需要注意的是，这个参数要生效必须开放[kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。controllerID这个触发器的控制器id。userArg这个参数的值由用户调用[addProximity](Entity.html#addProximity)addProximity时给出，用户可以根据此参数对当前行为做一些判断。def onEnteredView( self, entity ):

def onEnteredView(self, entity):如果这个函数在脚本中有实现，当一个实体进入了当前实体的[View](../../keywords.html#View)View范围，该回调被触发。参数：

entity 进入View范围的实体。

参数：| entity | 进入View范围的实体。 |

entity进入View范围的实体。def onGetWitness( self ):

def onGetWitness(self):如果这个函数在脚本中有实现，当实体绑定了[Witness](../../keywords.html#Witness)Witness时被调用。也可以访问实体属性[Entity.hasWitness](#hasWitness)Entity.hasWitness得到实体当前的状态。def onLeaveTrap( self, entity, rangeXZ, rangeY, controllerID, userArg ):

def onLeaveTrap(self, entity, rangeXZ, rangeY, controllerID, userArg):如果这个函数在脚本中有实现，当实体离开了当前实体注册的范围触发器区域时被触发，范围触发器由[Entity](../../keywords.html#entity)Entity.[addProximity](#addProximity)addProximity注册。参数：

entity 已经离开触发器区域的实体。

rangeXZ
float，给定触发器xz轴区域的大小，必须大于等于0。

rangeY
float，给定触发器y轴高度，必须大于等于0。
需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y

开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。
3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。

controllerID 这个触发器的控制器ID。

userArg 这个参数的值由用户调用addProximity时给出，用户可以根据此参数对当前行为做一些判断。

参数：| entity | 已经离开触发器区域的实体。 |
| --- | --- |
| rangeXZ | float，给定触发器xz轴区域的大小，必须大于等于0。 |
| rangeY | float，给定触发器y轴高度，必须大于等于0。
需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y

开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。
3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。 |
| controllerID | 这个触发器的控制器ID。 |
| userArg | 这个参数的值由用户调用addProximity时给出，用户可以根据此参数对当前行为做一些判断。 |

entity已经离开触发器区域的实体。rangeXZfloat，给定触发器xz轴区域的大小，必须大于等于0。rangeYfloat，给定触发器y轴高度，必须大于等于0。需要注意的是，这个参数要生效必须开放[kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。controllerID这个触发器的控制器ID。userArg这个参数的值由用户调用[addProximity](Entity.html#addProximity)addProximity时给出，用户可以根据此参数对当前行为做一些判断。def onLoseControlledBy( self, id ):

def onLoseControlledBy(self, id):如果这个函数在脚本中有实现，当[Entity](../../keywords.html#entity)Entity.[controlledBy](Entity.html#controlledBy)controlledBy所关联的实体丢失时改回调方法被调用。参数：

id controlledBy实体的ID。

参数：| id | controlledBy实体的ID。 |

idcontrolledBy实体的ID。def onLoseWitness( self ):

def onLoseWitness(self):如果这个函数在脚本中有实现，当实体解除[Witness](../../keywords.html#Witness)Witness时，该回调被触发。也可以访问实体属性[Entity.hasWitness](#hasWitness)Entity.hasWitness得到实体当前的状态。def onMove( self, controllerID, userData ):

def onMove(self, controllerID, userData):如果这个函数在脚本中有实现，相关的[Entity](../../keywords.html#entity)Entity.[moveToPoint](Entity.html#moveToPoint)moveToPoint与[Entity](../../keywords.html#entity)Entity.[moveToEntity](Entity.html#moveToEntity)moveToEntity还有[Entity](../../keywords.html#entity)Entity.[navigate](Entity.html#navigate)navigate方法被调用并且成功后底层每帧移动都会回调此函数。参数：

controllerID 与某个移动相关的控制器ID。

userData 这个参数值由用户在请求移动相关接口时给出。

参数：| controllerID | 与某个移动相关的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

controllerID与某个移动相关的控制器ID。userData这个参数值由用户在请求移动相关接口时给出。def onMoveOver( self, controllerID, userData ):

def onMoveOver(self, controllerID, userData):如果这个函数在脚本中有实现，相关的[Entity](../../keywords.html#entity)Entity.[moveToPoint](Entity.html#moveToPoint)moveToPoint与[Entity](../../keywords.html#entity)Entity.[moveToEntity](Entity.html#moveToEntity)moveToEntity还有[Entity](../../keywords.html#entity)Entity.[navigate](Entity.html#navigate)navigate方法被调用后到达指定目标点时会回调此函数。参数：

controllerID 与某个移动相关的控制器ID。

userData 这个参数值由用户在请求移动相关接口时给出。

参数：| controllerID | 与某个移动相关的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

controllerID与某个移动相关的控制器ID。userData这个参数值由用户在请求移动相关接口时给出。def onMoveFailure( self, controllerID, userData ):

def onMoveFailure(self, controllerID, userData):如果这个函数在脚本中有实现，相关的[Entity](../../keywords.html#entity)Entity.[moveToPoint](Entity.html#moveToPoint)moveToPoint与[Entity](../../keywords.html#entity)Entity.[moveToEntity](Entity.html#moveToEntity)moveToEntity还有[Entity](../../keywords.html#entity)Entity.[navigate](Entity.html#navigate)navigate方法被调用并且失败时会回调此函数。参数：

controllerID 与某个移动相关的控制器ID。

userData 这个参数值由用户在请求移动相关接口时给出。

参数：| controllerID | 与某个移动相关的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

controllerID与某个移动相关的控制器ID。userData这个参数值由用户在请求移动相关接口时给出。def onRestore( self ):

def onRestore(self):如果这个函数在脚本中有实现，这个函数在Cell应用程序崩溃后在其它Cell应用程序上重新创建该实体时被调用。这个函数没有参数。def onSpaceGone( self ):

def onSpaceGone(self):如果这个函数在脚本中有实现，当前实体所在的[Space](../../keywords.html#cSpace)Space将要销毁时触发 。这个函数没有参数。def onTurn( self, controllerID, userData ):

def onTurn(self, controllerID, userData):如果这个函数在脚本中有实现，相关的[Entity](../../keywords.html#entity)Entity.[addYawRotator](Entity.html#addYawRotator)addYawRotator方法被调用后到达指定yaw时会回调此函数。参数：

controllerID Entity.addYawRotator返回的控制器ID。

userData 这个参数值由用户在请求移动相关接口时给出。

参数：| controllerID | Entity.addYawRotator返回的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

controllerID[Entity](../../keywords.html#entity)Entity.[addYawRotator](Entity.html#addYawRotator)addYawRotator返回的控制器ID。userData这个参数值由用户在请求移动相关接口时给出。def onTeleport( self ):

def onTeleport(self):如果这个函数在脚本中有实现，在通过baseapp的Entity.teleport调用发生的实体传送中，实体(Real entity)被传送之前的时刻此方法会被调用。请注意，在cell上调用实体的teleport并不会回调此接口，如果你需要这个功能请在[Entity](../../keywords.html#entity)Entity.[teleport](Entity.html#teleport)teleport之后调用此回调。def onTimer( self, timerHandle, userData ):

def onTimer(self, timerHandle, userData):功能说明：这个函数当一个与此实体关联的定时器触发的时候被调用。
一个定时器可以使用[Entity](../../keywords.html#entity)Entity.[addTimer](#addTimer)addTimer函数添加。参数：

timerHandle
定时器的id。

userData
传进Entity.addTimer的integer用户数据。

参数：| timerHandle | 定时器的id。 |
| --- | --- |
| userData | 传进Entity.addTimer的integer用户数据。 |

timerHandle定时器的id。userData传进[Entity](../../keywords.html#entity)Entity.[addTimer](#addTimer)addTimer的integer用户数据。def onTeleportFailure( self ):

def onTeleportFailure(self):如果这个函数在脚本中有实现，当用户调用[Entity](../../keywords.html#entity)Entity.[teleport](Entity.html#teleport)teleport失败时该回调被调用。def onTeleportSuccess( self, nearbyEntity ):

def onTeleportSuccess(self, nearbyEntity):如果这个函数在脚本中有实现，当用户调用[Entity](../../keywords.html#entity)Entity.[teleport](Entity.html#teleport)teleport成功时该回调被调用。参数：

nearbyEntity 这个参数由用户调用
    Entity.teleport时给出。这是一个real实体。

参数：| nearbyEntity | 这个参数由用户调用
    Entity.teleport时给出。这是一个real实体。 |

nearbyEntity这个参数由用户调用[Entity](../../keywords.html#entity)Entity.[teleport](Entity.html#teleport)teleport时给出。这是一个real实体。def onUpdateBegin( self ):

def onUpdateBegin(self):当同步一帧开始时被调用。def onUpdateEnd( self ):

def onUpdateEnd(self):当同步一帧结束时被调用。def onWitnessed( self, isWitnessed ):

def onWitnessed(self, isWitnessed):如果这个函数在脚本中有实现，如果当前实体进入了另一个绑定了Witness的实体的View范围（也可以理解为一个实体被观察者观察到了），则该实体的回调函数被调用。可以利用这个函数在实体被观察时激活实体的AI，实体停止被观察时可以停止AI的执行，这样可以降低服务端的计算量从而提升效率。参数：

isWitnessed bool，实体被观察时为True，实体被停止观察时是False。
也可以访问实体属性Entity.isWitnessed得到实体当前的状态。

参数：| isWitnessed | bool，实体被观察时为True，实体被停止观察时是False。
也可以访问实体属性Entity.isWitnessed得到实体当前的状态。 |

isWitnessedbool，实体被观察时为True，实体被停止观察时是False。也可以访问实体属性[Entity.isWitnessed](#isWitnessed)Entity.isWitnessed得到实体当前的状态。def onWriteToDB( self ):

def onWriteToDB(self):如果这个函数在脚本中有实现，这个函数在实体将要存档到数据库时被调用。
---

## 属性文档
属性文档allClients

allClients通过这个属性调用实体的客户端远程方法，引擎会将这个消息广播给实体View范围内所有的其他绑定了客户端的实体(包括自己的客户端，绑定了客户端的实体通常为玩家)。例子：avatar的View范围内有玩家A和玩家B以及怪物C。avatar.allClients.attack(monsterID，skillID, damage)此时，玩家自己和玩家A还有玩家B的客户端都会调用到该实体attack方法，在他们的客户端可以调用指定技能的攻击动作做表现。其他参考：

Entity.clientEntityEntity.otherClients

其他参考：| Entity.clientEntityEntity.otherClients |

[Entity](../../keywords.html#entity)Entity.[clientEntity](Entity.html#clientEntity)clientEntity[Entity](../../keywords.html#entity)Entity.[otherClients](Entity.html#otherClients)otherClientsbase

basebase是用于联系[Entity](../../baseapp/classes/Entity.html)Entity实体的entityCall。这个属性是只读的，且如果这个实体没有关联的[Entity](../../baseapp/classes/Entity.html)Entity实体时属性是None。其他参考：

Entity.clientEntityEntity.allClientsEntity.otherClients

其他参考：| Entity.clientEntityEntity.allClientsEntity.otherClients |

[Entity](../../keywords.html#entity)Entity.[clientEntity](Entity.html#clientEntity)clientEntity[Entity](../../keywords.html#entity)Entity.[allClients](Entity.html#allClients)allClients[Entity](../../keywords.html#entity)Entity.[otherClients](Entity.html#otherClients)otherClients类型：

只读的，ENTITYCALL

类型：| 只读的，ENTITYCALL |

只读的，[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALLclassName

className实体的类名。类型：

只读，string

类型：| 只读，string |

只读，stringclient

clientclient是用于联系客户端的entityCall。这个属性是只读的，且如果这个实体没有关联的客户端时属性是None。其他参考：

Entity.clientEntityEntity.allClientsEntity.otherClients

其他参考：| Entity.clientEntityEntity.allClientsEntity.otherClients |

[Entity](../../keywords.html#entity)Entity.[clientEntity](Entity.html#clientEntity)clientEntity[Entity](../../keywords.html#entity)Entity.[allClients](Entity.html#allClients)allClients[Entity](../../keywords.html#entity)Entity.[otherClients](Entity.html#otherClients)otherClients类型：

只读的，ENTITYCALL

类型：| 只读的，ENTITYCALL |

只读的，[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALLcontrolledBy

controlledBy该属性如果设置为某个客户端所关联的服务端实体的BaseEntityCall，那么该实体由对应的客户端来控制移动，如果该属性为None则实体由服务端移动。
当客户端登陆后调用giveClientTo到该实体时，该属性会自动的设置为自己的BaseEntityCall。脚本可以灵活的控制该实体由服务端控制移动或是由客户端（自己的客户端或是其他客户端）控制移动。其他参考：

Entity.onLoseControlledBy

其他参考：| Entity.onLoseControlledBy |

[Entity](../../keywords.html#entity)Entity.[onLoseControlledBy](Entity.html#onLoseControlledBy)onLoseControlledBy类型：

BaseEntityCall

类型：| BaseEntityCall |

[BaseEntityCall](../../keywords.html#EntityCall)BaseEntityCalldirection

direction这个属性描述的是[Entity](../../keywords.html#entity)Entity在世界空间中的朝向，用户可以改变这个属性，数据会同步到客户端。例子：

self.direction.y = 1.0
self.direction.z = 1.0类型：

Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。

类型：| Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。 |

Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。hasWitness

hasWitness这个只读属性如果为True，表示实体已经绑定了一个[Witness](../../keywords.html#Witness)Witness，绑定了Witness的实体则客户端可以通过实体获得实体View范围内的信息。否则为False。类型：

只读的， bool

类型：| 只读的， bool |

只读的， boolid

idid是[Entity](../../keywords.html#entity)Entity的对象id。这个id是一个整型，在base，cell和client相关联的实体之间是相同的。 这个属性是只读的。类型：

只读的，int32

类型：| 只读的，int32 |

只读的，int32isDestroyed

isDestroyed如果这个属性的值为True，[Entity](../../keywords.html#entity)Entity则已经被销毁了。类型：

只读的， bool

类型：| 只读的， bool |

只读的， boolisOnGround

isOnGround如果这个属性的值为True，[Entity](../../keywords.html#entity)Entity在地面上，否则为False。类型：

只读的， bool

类型：| 只读的， bool |

只读的， boolisWitnessed

isWitnessed如果当前实体进入了另一个绑定了Witness的实体的View范围（也可以理解为一个实体被观察者观察到了, 这个属性值为True，否则为False。参考：[Entity](../../keywords.html#entity)Entity.[onWitnessed](Entity.html#onWitnessed)onWitnessed类型：

只读的， bool

类型：| 只读的， bool |

只读的， boollayer

layer一个space可以同时加载多个navmesh数据，不同的navmesh在不同的layer中，不同的layer可被抽象成地面、水面等等。
通过这个属性决定一个实体存在于哪个layer中。参考：[KBEngine.addSpaceGeometryMapping](KBEngine.html#addSpaceGeometryMapping)KBEngine.addSpaceGeometryMapping类型：

int8

类型：| int8 |

int8otherClients

otherClients通过这个属性调用实体的客户端远程方法，引擎会将这个消息广播给实体View范围内所有的其他绑定了客户端的实体(不包括自己的客户端，绑定了客户端的实体通常为玩家)。例子：avatar的View范围内有玩家A和玩家B以及怪物C。avatar.otherClients.attack(monsterID，skillID, damage)此时，玩家A与玩家B的客户端都会调用到该实体attack方法，在他们的客户端可以调用指定技能的攻击动作做表现。其他参考：

Entity.clientEntityEntity.otherClients

其他参考：| Entity.clientEntityEntity.otherClients |

[Entity](../../keywords.html#entity)Entity.[clientEntity](Entity.html#clientEntity)clientEntity[Entity](../../keywords.html#entity)Entity.[otherClients](Entity.html#otherClients)otherClientsposition

position这个实体在世界空间中的坐标(x, y, z)，这个属性可以被用户改变，改变后会同步到客户端。
需要注意的是，不要引用这个属性，引用这个属性很有可能错误的修改了实体的真实坐标。例子：
```
self.position.y = 10.0

```
self.position.y = 10.0如果你想拷贝这个属性值可以使用如下方式：
```

	import Math
	self.copyPosition = Math.Vector3( self.position )

```
import Math
	self.copyPosition = Math.[Vector3](../../keywords.html#vector3)Vector3( self.position )类型：

Vector3

类型：| Vector3 |

[Vector3](../../keywords.html#vector3)Vector3spaceID

spaceID这个属性是实体所在的[空间](../../keywords.html#cn_Space)空间的ID，cell与客户端这个值都保持一致。类型：

只读的，Integer

类型：| 只读的，Integer |

只读的，IntegertopSpeed

topSpeed实体的最大xz轴移动速度（米/秒），这个属性通常要比实际移动速度要大一些，服务端通过这个属性检查客户端的移动合法性，如果移动距离超出速度限制则被强制拉回上一个坐标位置。其他参考：

Entity.topSpeedY

其他参考：| Entity.topSpeedY |

[Entity](../../keywords.html#entity)Entity.[topSpeedY](Entity.html#topSpeedY)topSpeedY类型：

float

类型：| float |

floattopSpeedY

topSpeedY实体的最大y轴移动速度（米/秒），这个属性通常要比实际移动速度要大一些，服务端通过这个属性检查客户端的移动合法性，如果移动距离超出速度限制则被强制拉回上一个坐标位置。其他参考：

Entity.topSpeed

其他参考：| Entity.topSpeed |

[Entity](../../keywords.html#entity)Entity.[topSpeed](Entity.html#topSpeed)topSpeed类型：

float

类型：| float |

floatvolatileInfo

volatileInfo这个属性指定[Entity](../../keywords.html#entity)Entity的易变类数据同步到客户端的策略。易变类数据包括实体的坐标position和实体的朝向direction，易变类数据由于极易改变的特性，引擎底层使用了一套优化的方案将其同步到客户端。这个属性是四个float（position，yaw，pitch，roll）代表距离值，当一个实体靠近当前实体达到距离则服务端向其同步相关数据。如果距离值大于View半径则代表总是同步。还有一个特殊的bool属性optimized，它的作用是控制服务器同步时是否进行优化，目前主要的优化是Y轴。如果为true，在一些行为(如：navigate)导致服务器能确定实体在地面时，服务器不同步实体的Y轴坐标，当同步大量实体时能节省大量带宽，默认为true。用户也可以在.def制定不同实体的同步策略：
```
<Volatile>
    <position/>           <!-- 总是同步 -->
    <yaw/>                <!-- 总是同步 -->
    <pitch>20</pitch>     <!-- 相距20米或以内同步     -->
    <optimized> true </optimized>   
</Volatile>               <!-- roll未指明则总是同步  -->


```
<Volatile>
    <position/>           <!-- 总是同步 -->
    <yaw/>                <!-- 总是同步 -->
    <pitch>20</pitch>     <!-- 相距20米或以内同步     -->
    <optimized> true </optimized>   
</Volatile>               <!-- roll未指明则总是同步  -->类型：

sequence， 四个浮点数(float, float, float, float)

类型：| sequence， 四个浮点数(float, float, float, float) |

sequence， 四个浮点数(float, float, float, float)
---
版权归KBEngine所有。

版权归KBEngine所有。