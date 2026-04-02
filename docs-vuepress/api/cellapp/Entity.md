# Entity类

[KBEngine模块]

Entity是KBEngine模块的一部分。[更多...](#detailed_description)

```python
import KBEngine
```

## 成员函数

- def [accelerate](#accelerate)( self, accelerateType, acceleration ):
- def [addYawRotator](#addYawRotator)( self, targetYaw, velocity, userArg ):
- def [addProximity](#addProximity)( self, range, userArg ):
- def [addTimer](#addTimer)( self, start, interval=0.0, userData=0 ):
- def [cancelController](#cancelController)( self, controllerID ):
- def [clientEntity](#clientEntity)( self, destID ):
- def [canNavigate](#canNavigate)( self ):
- def [debugView](#debugView)( self ):
- def [delTimer](#delTimer)( self, id ):
- def [destroy](#destroy)( self ):
- def [destroySpace](#destroySpace)( self ):
- def [entitiesInView](#entitiesInView)( self, pending):
- def [entitiesInRange](#entitiesInRange)( self, range, entityType=None, position=None ):
- def [isReal](#isReal)( self ):
- def [moveToEntity](#moveToEntity)( self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos ):
- def [moveToPoint](#moveToPoint)( self, destination, velocity, distance, userData, faceMovement, moveVertically ):
- def [getViewRadius](#getViewRadius)( self ):
- def [getViewHystArea](#getViewHystArea)( self ):
- def [getRandomPoints](#getRandomPoints)( self, centerPos, maxRadius, maxPoints, layer ):
- def [navigate](#navigate)( self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData ):
- def [navigatePathPoints](#navigatePathPoints)( self, destination, maxSearchDistance, layer ):
- def [setViewRadius](#setViewRadius)( self, radius, hyst=5 ):
- def [teleport](#teleport)( self, nearbyMBRef, position, direction ):
- def [writeToDB](#writeToDB)( self, shouldAutoLoad, dbInterfaceName ):
- def [getWitnesses](#getWitnesses)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## 回调函数

- def [onDestroy](#onDestroy)( self ):
- def [onEnterTrap](#onEnterTrap)( self, entity, rangeXZ, rangeY, controllerID, userArg ):
- def [onEnteredView](#onEnteredView)( self, entity ):
- def [onGetWitness](#onGetWitness)( self ):
- def [onLeaveTrap](#onLeaveTrap)( self, entity, rangeXZ, rangeY, controllerID, userArg ):
- def [onLoseControlledBy](#onLoseControlledBy)( self, id ):
- def [onLoseWitness](#onLoseWitness)( self ):
- def [onMove](#onMove)( self, controllerID, userData ):
- def [onMoveOver](#onMoveOver)( self, controllerID, userData ):
- def [onMoveFailure](#onMoveFailure)( self, controllerID, userData ):
- def [onRestore](#onRestore)( self ):
- def [onSpaceGone](#onSpaceGone)( self ):
- def [onTurn](#onTurn)( self, controllerID, userData ):
- def [onTeleport](#onTeleport)( self ):
- def [onTeleportFailure](#onTeleportFailure)( self ):
- def [onTeleportSuccess](#onTeleportSuccess)( self, nearbyEntity ):
- def [onTimer](#onTimer)( self, timerHandle, userData ):
- def [onUpdateBegin](#onUpdateBegin)( self ):
- def [onUpdateEnd](#onUpdateEnd)( self ):
- def [onWitnessed](#onWitnessed)( self, isWitnessed ):
- def [onWriteToDB](#onWriteToDB)( self ):

## 属性

| [allClients](#allClients) | 只读 PyClient |
| --- | --- |
| [base](#base) | 只读BaseEntityCall |
| [client](#client) | 只读ClientEntityCall |
| [controlledBy](#controlledBy) | BaseEntityCall |
| [className](#className) | 只读string |
| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| [hasWitness](#hasWitness) | boolean |
| [id](#id) | 只读 Integer |
| [isDestroyed](#isDestroyed) | 只读 bool |
| [isWitnessed](#isWitnessed) | 只读 bool |
| [layer](#layer) | int8 |
| [otherClients](#otherClients) | 只读 PyClient |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | 只读 uint32 |
| [topSpeed](#topSpeed) | float |
| [topSpeedY](#topSpeedY) | float |
| [volatileInfo](#volatileInfo) | float |

<a id="detailed_description"></a>

## 详细描述

类Entity的实例代表着在cell上的游戏对象。一个Entity可以是"real"或者"ghosted"的，一个"ghost"Entity是一个存活在邻近的cell上
的"real"Entity的拷贝。对于每一个实体来说有一个唯一的"real"Entity实例，和有0个
或者更多的"ghost"Entity实例。
一个Entity实例操控着实体的位置数据，包括他的空间和旋转。它还控制着这些数据发给
客户端的频率（如果可以）。位置的数据可以被唯一的客户端所更新，被控制器对象，被teleport成员函数修改。
控制器是非python对象，可以适用在cell实体上随着时间的过去来改变它们的位置数据，它们通过成员函数如"trackEntity"和"turnToYaw"来
添加到Entity，可以通过"cancelController"移除。
感兴趣的范围，或"View"对于所有属于客户端的KBEngine实体来说是一个重要的概念。
一个实体的View是围绕这个实体的客户端（如果它有）所能感知的区域。这用于选择发给客户端的数据量。View的实际
形状由x轴和z轴上的距离范围定义，还有一个类似形状向外延伸的滞后区域。一个Entity进入
另一个Entity的View，但不会离开它直到它离开滞后区域。一个Entity可以
通过"setViewRadius"修改它的View大小。
可以通过"entitiesInRange"找到一个具体距离之内的所有实体，通过"addProximity"设置一个陷阱捕获进入陷阱的所有实体。
cellApp上新的Entity可以使用KBEngine.createEntity创建。一个实体还可以通过baseApp远程调用KBEngine.createCellEntity函数
来创建。
一个Entity可以通过ENTITYCALL访问在base和client应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。

## 成员函数文档

<a id="accelerate"></a>

### def accelerate(self, accelerateType, acceleration):

功能说明：
加速实体当前运动。

可影响的运动包括：

- Entity.[moveToEntity](./Entity.md#moveToEntity)
- Entity.[moveToPoint](./Entity.md#moveToPoint)
- Entity.[navigate](./Entity.md#navigate)
- Entity.[addYawRotator](./Entity.md#addYawRotator)

参数：

| accelerateType | string，影响的运动类型，如：Movement、Turn。 |
| --- | --- |
| velocity | float，每秒的加速度，如果传入的是负值则表示减速度。 |

返回：

- 影响后的实体当前速度。

<a id="addYawRotator"></a>

### def addYawRotator(self, targetYaw, velocity, userArg):

功能说明：
控制实体绕yaw旋转，旋转完成将会通过Entity.[onTurn](./Entity.md#onTurn)通知。
使用Entity.[cancelController](./Entity.md#cancelController)带上控制器ID或者使用Entity.[cancelController("Movement")](./Entity.md#cancelController)来删除它。

参看：

- Entity.[cancelController](./Entity.md#cancelController)

参数：

| targetYaw | float，给定的目标yaw弧度。 |
| --- | --- |
| velocity | float，旋转时每秒的弧度。 |
| userArg | 是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议 在回调原型里设置默认值为0。 |

<a id="addProximity"></a>

### def addProximity(self, rangeXZ, rangeY, userArg):

功能说明：
创建一个范围触发器，当有其它实体进入或离开这个触发器区域的时候会通知这个Entity。这个区域是一个方形（为了效率）。如果
其它实体在x轴和z轴上均在给定的距离里面，则实体被视为在这个范围里面。这个Entity通过onEnterTrap和onLeaveTrap函数被
通知，这两个函数可以如下定义：

```python
	def onEnterTrap( self, entityEntering, rangeXZ, rangeY, controllerID, userArg = 0 ):
	def onLeaveTrap( self, entityLeaving, rangeXZ, rangeY, controllerID, userArg = 0 ):
```

由于这个范围触发器是一个控制器，使用Entity.[cancelController](./Entity.md#cancelController)带上控制器ID来删除它。
需要注意的是回调有可能会立刻被触发，即使在addProximity()调用返回之前。

参看：

- Entity.[cancelController](./Entity.md#cancelController)

参数：

| rangeXZ | float，给定触发器xz轴区域的大小，必须大于等于0。 |
| --- | --- |
| rangeY | float，给定触发器y轴高度，必须大于等于0。 需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y 开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。 3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。 |
| userArg | 是一个可选的整型，所有控制器共有。如果这个值不为0则传给回调函数。建议 在回调原型里设置默认值为0。 |

返回：

- 返回创建控制器的id。

<a id="addTimer"></a>

### def addTimer(self, start, interval=0.0, userData=0):

功能说明：
注册一个定时器，定时器由回调函数[onTimer](#onTimer)触发，回调函数将在"initialOffset"秒后被执行第1次，而后将每间隔"repeatOffset"秒执行1次，可设定一个用户参数"userArg"（仅限integer类型）。
[onTimer](#onTimer)函数必须在entity的cell部分被定义，且带有2个参数，第1个integer类型的是timer的id（可用于移除timer的"[delTimer](#delTimer)"函数），第2个是用户参数"userArg"。
例子:

```python
# 这里是使用addTimer的一个例子
import 
KBEngine

 
class MyCellEntity( 
KBEngine
.
Entity
 ):
 
    def __init__( self ):
        
KBEngine
.
Entity
.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    # 
Entity
的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyCellEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if 这是不断重复的定时器，当不再需要该定时器的时候，调用下面函数移除:
        #     self.delTimer( id )
```

参数：

| initialOffset | float，指定定时器从注册到第一次回调的时间间隔（秒）。 |
| --- | --- |
| repeatOffset | float，指定第一次回调执行后每次执行的时间间隔（秒）。必须用函数[delTimer](#delTimer)移除定时器，否则它会一直重复下去。值小于等于0将被忽略。 |
| userArg | integer，指定底层回调"[onTimer](#onTimer)"时的userArg参数值。 |

返回:

- integer，该函数返回timer的内部id，这个id可用于[delTimer](#delTimer)移除定时器。

<a id="cancelController"></a>

### def cancelController(self, controllerID):

功能说明：
函数cancelController停止一个控制器对Entity的影响。它只能在一个real实体上被调用。

参数：

| controllerID | controllerID是要取消的控制器的索引，它是一个整型。一个专用的控制器类型的字符串也可以作为它的类型。 例如，一次只有一个移动/导航控制器可以被激活，这可以用entity.cancelController( "Movement" )取消。 |
| --- | --- |

<a id="clientEntity"></a>

### def clientEntity(self, destID):

功能说明：
通过这个方法可以访问自己客户端（当前实体必须绑定了客户端）中某个实体的方法，只有在View范围内的实体才会同步到客户端。它只能在一个real实体上被调用。

参数：

| destID | 目标实体的ID。 |
| --- | --- |

<a id="canNavigate"></a>

### def canNavigate(self):

功能说明：
通过这个方法判断当前实体是否可以使用导航（Entity.[navigate](./Entity.md#navigate)）功能。它只能在一个real实体上被调用。
通常当实体所在Space使用Entity.[addSpaceGeometryMapping](./Entity.md#addSpaceGeometryMapping)加载过有效的导航用的碰撞数据（Navmesh或者2D的tile数据）并且实体在有效导航区域该功能可用。

返回：

- bool， 如果实体可在当前Space中使用导航功能返回True，否则返回False。

<a id="debugView"></a>

### def debugView(self):

功能说明：
debugView输出Entity的View的详细信息到cell的调试日志。
一份View系统工作的描述可以在Entity类文档中找到。
一份样品信息如下：

```python
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 size=4, Seen=4, Pending=0, ViewRadius=50.000, ViewHyst=5.000
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(102), position(771.586.211.002.776.55), dist=0
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Monster(1028), position(820.834.211.635.768.749), dist=49.8659
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 NPC(1025), position(784.024.210.95.782.273), dist=13.6915
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(106), position(771.586.211.002.776.55), dist=0
```

信息的第一行告诉我们：

- 实体#1000的数据
- 有4个实体在它的View区域并且已经同步给客户端。
- 有0个实体在它的View区域，正在等待同步到客户端。
- View的半径是 50.000
- View的滞后区域向外延伸了5.000

<a id="delTimer"></a>

### def delTimer(self, id):

功能说明：
函数delTimer用于移除一个注册的定时器，移除后的定时器不再执行。只执行1次的定时器在执行回调后自动移除，不必要使用delTimer移除。
如果delTimer函数使用一个无效的id（例如已经移除），将会产生错误。

参数：

| id | integer，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。 |
| --- | --- |

<a id="destroy"></a>

### def destroy(self):

功能说明：
这个函数销毁它的本地Entity实例，如果实体在其他进程上存在ghost部分也会同时通知销毁。
这个函数最好由实体自己调用，如果这个实体是一个ghost则会抛出一个异常。如果回调函数onDestroy()被实现则被调用。

<a id="destroySpace"></a>

### def destroySpace(self):

功能说明：
销毁这个实体所在的空间。

<a id="entitiesInView"></a>

### def entitiesInView(self, pending):

功能说明：
获得这个实体的View范围内的实体列表。

参数：

| pending | bool，可选参数，默认值为False，只返回客户端可见的所有实体，否则返回服务端可见但未同步到客户端的所有实体。 |
| --- | --- |

<a id="entitiesInRange"></a>

### def entitiesInRange(self, range, entityType=None, position=None):

功能说明：
在给定的距离内搜索[实体](./Entity.md)。这是一个球形的搜索，3个轴的距离
都要测量。这可以找到在这个[实体](./Entity.md)的View范围之外的实体，但不能找到其他cell的实体。
例子：

```python
  self.entitiesInRange( 100, 'Creature', (100, 0, 100) )
```

搜索到‘Creature’类型的实体列表（‘Creature’的子类实例化的实体）。中心点是(100, 0, 100)，搜索半径是100米。

```python
  [ e for e in self.entitiesInRange( 100, None, (100,0,100) ) if isinstance( e, BaseType ) ]
```

将给出一个来自‘BaseType’或‘BaseType’的子类实例化的实体列表。

参数：

| range | 围绕这个实体搜索的距离，float类型 |
| --- | --- |
| entityType | 一个可选的字符串参数，实体的类型名称，用于匹配实体。如果实体类型是一个有效的类名（ 有效的实体类型在/scripts/entities.xml列出 ）， 则只有这个类型的实体会被返回，否则将这个范围的所有实体都返回。 |
| position | 一个可选的Vector3类型参数，作为搜索半径的中心, 默认以实体自身为中心。 |

返回：

- 在给定范围内的Entity对象列表。

<a id="isReal"></a>

### def isReal(self):

功能说明：
这个函数返回这个Entity是real的还是一个ghost的。
这个函数很少被用到但对调试很有用。

返回：

- bool， 如果是real实体返回True，否则返回False。

<a id="moveToEntity"></a>

### def moveToEntity(self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos):

功能说明：
直线移动[实体](./Entity.md)到另一个Entity位置。
任何[实体](./Entity.md)，在任意时刻只能有一个移动控制器，重复调用任何移动函数将终止之前的移动控制器。
调用后函数将返回一个可以用于取消这次移动的控制器ID。
例如，Entity.[cancelController](./Entity.md#cancelController)( movementID )。
移动取消还可以调用Entity.[cancelController](./Entity.md#cancelController)( "Movement" )。当移动被取消之后回调通知方法将不被调用。

```python
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):
```

参考:

- Entity.[cancelController](./Entity.md#cancelController)

参数：

| destEntityID | int，目标Entity的ID |
| --- | --- |
| velocity | float，Entity移动的速度，单位m/s |
| distance | float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。 |
| userData | object，可选参数，回调通知被调用时其中userData参数将为此值。 |
| faceMovement | bool，可选参数，如果实体面向移动方向则为true。如果是其它机制则为false。 |
| moveVertically | bool，可选参数，设为True指移动为直线移动，设为False指贴着地面直线移动。 |
| offsetPos | Vector3，可选参数，设置一定的偏移值，例如使移动目标位置位于实体的左侧。 |

返回：

- int，新创建的控制器ID。

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

<a id="getViewRadius"></a>

### def getViewRadius(self):

功能说明：
这个函数返回这个Entity当前的View半径值。
数据可以通过Entity.[setViewRadius](./Entity.md#setViewRadius)( radius, hyst )设置。

返回：

- float， View半径。

<a id="getViewHystArea"></a>

### def getViewHystArea(self):

功能说明：
这个函数返回这个Entity的View当前滞后区域值。
数据可以通过Entity.[setViewRadius](./Entity.md#setViewRadius)( radius, hyst )设置。

返回：

- float， View当前滞后区域值。

<a id="getRandomPoints"></a>

### def getRandomPoints(self, centerPos, maxRadius, maxPoints, layer):

功能说明：
这个函数用于获取以某个坐标点为中心一定区域内Entity.[navigate](./Entity.md#navigate)可到达的随机坐标点。

参数：

| centerPos | Vector3，Entity中心坐标点 |
| --- | --- |
| maxRadius | float，最大的搜索半径 |
| maxPoints | uint32，最多返回的随机坐标点数量。 |
| layer | int8，使用某个层的navmesh来搜索。 |

返回：

- tuple，包含一个或者多个坐标的数组。

<a id="navigate"></a>

### def navigate(self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData):

功能说明：
使用导航系统来使这个Entity向一个目标点移动，成功或失败会调用回调函数。
KBEngine可以有数个预先生成好的导航网格，不同的网格大小（会导致不同的导航路径）。
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

| destination | Vector3，Entity移向的目标点。 |
| --- | --- |
| velocity | float，Entity的移动速度，单位m/s |
| distance | float，距离目标小于该值停止移动，如果该值为0则移动到目标位置。 |
| maxMoveDistance | float，最大的移动距离。 |
| maxSearchDistance | float，从导航数据中最大搜索距离。 |
| faceMovement | bool，如果实体面向移动方向则为true（默认）。如果是其它机制则为false。 |
| layer | int8，使用某个层的navmesh来寻路。 |
| userData | object，传给通知函数的数据。 |

返回：

- int，新创建的控制器ID。

<a id="navigatePathPoints"></a>

### def navigatePathPoints(self, destination, maxSearchDistance, layer):

功能说明：
这个函数返回有当前Entity的位置到destination位置所经过的路径点列表。

参数：

| destination | Vector3，Entity移向的目标点 |
| --- | --- |
| maxSearchDistance | float，最大的搜索距离 |
| layer | int8，使用某个层的navmesh来查找路径列表。 |

<a id="setViewRadius"></a>

### def setViewRadius(self, radius, hyst=5):

功能说明：
指定Entity的感兴趣的区域大小。
这个函数只能用于有Witness关联的实体。
注意：
你可以通过设置kbengine.xml配置选项'cellapp/defaultViewRadius'来设置默认的View半径。
View半径默认是不能大于500.0的。详情请看kbengine_defaults.xml配置选项'cellapp/entity_posdir_updates'。
数据可以通过Entity.[getViewRadius](./Entity.md#getViewRadius)( )与Entity.[getViewHystArea](./Entity.md#getViewHystArea)( )获得。

参数：

| radius | float，radius指定View区域的半径 |
| --- | --- |
| hyst | float，指定超过View区域的滞后区域的大小。合理的设定滞后区域将能够降低View碰撞的敏感度从而提高CPU执行效率。 一个实体进入另一个实体的View必须跨越View半径区域，但实体离开View区域则需要移出View半径区域包括滞后区域。 |

返回：

- None

<a id="teleport"></a>

### def teleport(self, nearbyMBRef, position, direction):

功能说明：
瞬间移动一个Entity到一个指定的空间。这个函数允许指定实体移动后的位置与朝向。
如果需要在不同空间跳转（ 通常用于不同场景或者房间跳转 ），可以传一个CellEntityCall给这个函数（ 这个entityCall所对应的实体必须在目的空间中 ）。
这个函数只能在real的实体上被调用。

参数：

| nearbyMBRef | 一个决定Entity跳往哪个Space的CellEntityCall（ 这个entityCall所对应的实体必须在目的Space中 ），它被认为是传送目的地。 这个可以设为None，在这种情形下它会在当前的cell完成瞬移。 |
| --- | --- |
| position | Entity瞬移后的坐标，是一个有3个float(x, y, z)组成的序列。 |
| direction | Entity瞬移后的朝向，是一个由3个float组成的序列(roll,pitch, yaw)。 |

<a id="writeToDB"></a>

### def writeToDB(self, shouldAutoLoad, dbInterfaceName):

功能说明：
这个函数保存与这个实体相关的数据到数据库，包括base实体的数据。在数据确认传到数据库之前base实体的onWriteToDB函数会被调用。
cell实体的数据同时备份在base实体，确保遇到灾难恢复数据时数据是最新的。
这个函数只能在real实体且实体必须存在base部分时才允许被调用。

参数：

| shouldAutoLoad | 这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。 注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。 脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如： def onAutoLoadEntityCreate(entityType, dbid): KBEngine.createEntityFromDBID(entityType, dbid) |
| --- | --- |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

<a id="getWitnesses"></a>

### def getWitnesses(self):

功能说明：
这个函数返回观察该Entity的所有观察者(玩家)。

返回：

- tuple， 包含零个或者多个Entity的数组。

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

如果这个函数在脚本中有实现，这个函数在调用[Entity](./Entity.md).[destroy](./Entity.md#destroy)()后，在实际销毁之前被调用。
这个函数没有参数。

<a id="onEnterTrap"></a>

### def onEnterTrap(self, entity, rangeXZ, rangeY, controllerID, userArg):

当注册了使用Entity.[addProximity](./Entity.md#addProximity)注册了一个范围触发器，有其他实体进入触发器时，该回调函数被调用。

参数：

| entity | 已经进入了范围的实体。 |
| --- | --- |
| rangeXZ | float，给定触发器xz轴区域的大小，必须大于等于0。 |
| rangeY | float，给定触发器y轴高度，必须大于等于0。 需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y 开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。 3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。 |
| controllerID | 这个触发器的控制器id。 |
| userArg | 这个参数的值由用户调用[addProximity](./Entity.md#addProximity)时给出，用户可以根据此参数对当前行为做一些判断。 |

<a id="onEnteredView"></a>

### def onEnteredView(self, entity):

如果这个函数在脚本中有实现，当一个实体进入了当前实体的View范围，该回调被触发。

参数：

| entity | 进入View范围的实体。 |
| --- | --- |

<a id="onGetWitness"></a>

### def onGetWitness(self):

如果这个函数在脚本中有实现，当实体绑定了Witness时被调用。
也可以访问实体属性[Entity.hasWitness](#hasWitness)得到实体当前的状态。

<a id="onLeaveTrap"></a>

### def onLeaveTrap(self, entity, rangeXZ, rangeY, controllerID, userArg):

如果这个函数在脚本中有实现，当实体离开了当前实体注册的范围触发器区域时被触发，范围触发器由Entity.[addProximity](#addProximity)注册。

参数：

| entity | 已经离开触发器区域的实体。 |
| --- | --- |
| rangeXZ | float，给定触发器xz轴区域的大小，必须大于等于0。 |
| rangeY | float，给定触发器y轴高度，必须大于等于0。 需要注意的是，这个参数要生效必须开放kbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y 开放y轴管理会带来一些消耗，因为一些游戏大量的实体都在同一y轴高度或者在差不多水平线高度，此时碰撞变得非常密集。 3D太空类游戏或者小房间类实体较少的游戏比较适合开放此选项。 |
| controllerID | 这个触发器的控制器ID。 |
| userArg | 这个参数的值由用户调用[addProximity](./Entity.md#addProximity)时给出，用户可以根据此参数对当前行为做一些判断。 |

<a id="onLoseControlledBy"></a>

### def onLoseControlledBy(self, id):

如果这个函数在脚本中有实现，当Entity.[controlledBy](./Entity.md#controlledBy)所关联的实体丢失时改回调方法被调用。

参数：

| id | controlledBy实体的ID。 |
| --- | --- |

<a id="onLoseWitness"></a>

### def onLoseWitness(self):

如果这个函数在脚本中有实现，当实体解除Witness时，该回调被触发。
也可以访问实体属性[Entity.hasWitness](#hasWitness)得到实体当前的状态。

<a id="onMove"></a>

### def onMove(self, controllerID, userData):

如果这个函数在脚本中有实现，相关的Entity.[moveToPoint](./Entity.md#moveToPoint)与Entity.[moveToEntity](./Entity.md#moveToEntity)还有Entity.[navigate](./Entity.md#navigate)方法被调用并且成功后底层每帧移动都会回调此函数。

参数：

| controllerID | 与某个移动相关的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

<a id="onMoveOver"></a>

### def onMoveOver(self, controllerID, userData):

如果这个函数在脚本中有实现，相关的Entity.[moveToPoint](./Entity.md#moveToPoint)与Entity.[moveToEntity](./Entity.md#moveToEntity)还有Entity.[navigate](./Entity.md#navigate)方法被调用后到达指定目标点时会回调此函数。

参数：

| controllerID | 与某个移动相关的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

<a id="onMoveFailure"></a>

### def onMoveFailure(self, controllerID, userData):

如果这个函数在脚本中有实现，相关的Entity.[moveToPoint](./Entity.md#moveToPoint)与Entity.[moveToEntity](./Entity.md#moveToEntity)还有Entity.[navigate](./Entity.md#navigate)方法被调用并且失败时会回调此函数。

参数：

| controllerID | 与某个移动相关的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

<a id="onRestore"></a>

### def onRestore(self):

如果这个函数在脚本中有实现，这个函数在Cell应用程序崩溃后在其它Cell应用程序上重新创建该实体时被调用。这个函数没有参数。

<a id="onSpaceGone"></a>

### def onSpaceGone(self):

如果这个函数在脚本中有实现，当前实体所在的Space将要销毁时触发 。这个函数没有参数。

<a id="onTurn"></a>

### def onTurn(self, controllerID, userData):

如果这个函数在脚本中有实现，相关的Entity.[addYawRotator](./Entity.md#addYawRotator)方法被调用后到达指定yaw时会回调此函数。

参数：

| controllerID | Entity.[addYawRotator](./Entity.md#addYawRotator)返回的控制器ID。 |
| --- | --- |
| userData | 这个参数值由用户在请求移动相关接口时给出。 |

<a id="onTeleport"></a>

### def onTeleport(self):

如果这个函数在脚本中有实现，在通过baseapp的Entity.teleport调用发生的实体传送中，实体(Real entity)被传送之前的时刻此方法会被调用。
请注意，在cell上调用实体的teleport并不会回调此接口，如果你需要这个功能请在Entity.[teleport](./Entity.md#teleport)之后调用此回调。

<a id="onTimer"></a>

### def onTimer(self, timerHandle, userData):

功能说明：
这个函数当一个与此实体关联的定时器触发的时候被调用。
一个定时器可以使用Entity.[addTimer](#addTimer)函数添加。

参数：

| timerHandle | 定时器的id。 |
| --- | --- |
| userData | 传进Entity.[addTimer](#addTimer)的integer用户数据。 |

<a id="onTeleportFailure"></a>

### def onTeleportFailure(self):

如果这个函数在脚本中有实现，当用户调用Entity.[teleport](./Entity.md#teleport)失败时该回调被调用。

<a id="onTeleportSuccess"></a>

### def onTeleportSuccess(self, nearbyEntity):

如果这个函数在脚本中有实现，当用户调用Entity.[teleport](./Entity.md#teleport)成功时该回调被调用。

参数：

| nearbyEntity | 这个参数由用户调用Entity.[teleport](./Entity.md#teleport)时给出。这是一个real实体。 |
| --- | --- |

<a id="onUpdateBegin"></a>

### def onUpdateBegin(self):

当同步一帧开始时被调用。

<a id="onUpdateEnd"></a>

### def onUpdateEnd(self):

当同步一帧结束时被调用。

<a id="onWitnessed"></a>

### def onWitnessed(self, isWitnessed):

如果这个函数在脚本中有实现，如果当前实体进入了另一个绑定了Witness的实体的View范围（也可以理解为一个实体被观察者观察到了），则该实体的回调函数被调用。
可以利用这个函数在实体被观察时激活实体的AI，实体停止被观察时可以停止AI的执行，这样可以降低服务端的计算量从而提升效率。

参数：

| isWitnessed | bool，实体被观察时为True，实体被停止观察时是False。 也可以访问实体属性[Entity.isWitnessed](#isWitnessed)得到实体当前的状态。 |
| --- | --- |

<a id="onWriteToDB"></a>

### def onWriteToDB(self):

如果这个函数在脚本中有实现，这个函数在实体将要存档到数据库时被调用。

## 属性文档

<a id="allClients"></a>

allClients

通过这个属性调用实体的客户端远程方法，引擎会将这个消息广播给实体View范围内所有的其他绑定了客户端的实体(包括自己的客户端，绑定了客户端的实体通常为玩家)。
例子：
avatar的View范围内有玩家A和玩家B以及怪物C。
avatar.allClients.attack(monsterID，skillID, damage)
此时，玩家自己和玩家A还有玩家B的客户端都会调用到该实体attack方法，在他们的客户端可以调用指定技能的攻击动作做表现。

其他参考：

- Entity.[clientEntity](./Entity.md#clientEntity) Entity.[otherClients](./Entity.md#otherClients)

<a id="base"></a>

base

base是用于联系Entity实体的entityCall。这个属性是只读的，且如果这个实体没有关联的Entity实体时属性是None。

其他参考：

- Entity.[clientEntity](./Entity.md#clientEntity) Entity.[allClients](./Entity.md#allClients) Entity.[otherClients](./Entity.md#otherClients)

类型：

- 只读的，ENTITYCALL

<a id="className"></a>

className

实体的类名。

类型：

- 只读，string

<a id="client"></a>

client

client是用于联系客户端的entityCall。这个属性是只读的，且如果这个实体没有关联的客户端时属性是None。

其他参考：

- Entity.[clientEntity](./Entity.md#clientEntity) Entity.[allClients](./Entity.md#allClients) Entity.[otherClients](./Entity.md#otherClients)

类型：

- 只读的，ENTITYCALL

<a id="controlledBy"></a>

controlledBy

该属性如果设置为某个客户端所关联的服务端实体的BaseEntityCall，那么该实体由对应的客户端来控制移动，如果该属性为None则实体由服务端移动。
当客户端登陆后调用giveClientTo到该实体时，该属性会自动的设置为自己的BaseEntityCall。
脚本可以灵活的控制该实体由服务端控制移动或是由客户端（自己的客户端或是其他客户端）控制移动。

其他参考：

- Entity.[onLoseControlledBy](./Entity.md#onLoseControlledBy)

类型：

- BaseEntityCall

<a id="direction"></a>

direction

这个属性描述的是Entity在世界空间中的朝向，用户可以改变这个属性，数据会同步到客户端。

例子：
self.direction.y = 1.0
self.direction.z = 1.0

类型：

- Vector3, 其中包含(roll, pitch, yaw)，以弧度表示。

<a id="hasWitness"></a>

hasWitness

这个只读属性如果为True，表示实体已经绑定了一个Witness，绑定了Witness的实体则客户端可以通过实体获得实体View范围内的信息。否则为False。

类型：

- 只读的， bool

<a id="id"></a>

id

id是Entity的对象id。这个id是一个整型，在base，cell和client相关联的实体之间是相同的。 这个属性是只读的。

类型：

- 只读的，int32

<a id="isDestroyed"></a>

isDestroyed

如果这个属性的值为True，Entity则已经被销毁了。

类型：

- 只读的， bool

<a id="isOnGround"></a>

isOnGround

如果这个属性的值为True，Entity在地面上，否则为False。

类型：

- 只读的， bool

<a id="isWitnessed"></a>

isWitnessed

如果当前实体进入了另一个绑定了Witness的实体的View范围（也可以理解为一个实体被观察者观察到了, 这个属性值为True，否则为False。
参考：
Entity.[onWitnessed](./Entity.md#onWitnessed)

类型：

- 只读的， bool

<a id="layer"></a>

layer

一个space可以同时加载多个navmesh数据，不同的navmesh在不同的layer中，不同的layer可被抽象成地面、水面等等。
通过这个属性决定一个实体存在于哪个layer中。
参考：
KBEngine.addSpaceGeometryMapping

类型：

- int8

<a id="otherClients"></a>

otherClients

通过这个属性调用实体的客户端远程方法，引擎会将这个消息广播给实体View范围内所有的其他绑定了客户端的实体(不包括自己的客户端，绑定了客户端的实体通常为玩家)。
例子：
avatar的View范围内有玩家A和玩家B以及怪物C。
avatar.otherClients.attack(monsterID，skillID, damage)
此时，玩家A与玩家B的客户端都会调用到该实体attack方法，在他们的客户端可以调用指定技能的攻击动作做表现。

其他参考：

- Entity.[clientEntity](./Entity.md#clientEntity) Entity.[otherClients](./Entity.md#otherClients)

<a id="position"></a>

position

这个实体在世界空间中的坐标(x, y, z)，这个属性可以被用户改变，改变后会同步到客户端。
需要注意的是，不要引用这个属性，引用这个属性很有可能错误的修改了实体的真实坐标。
例子：

```python
self.position.y = 10.0
```

如果你想拷贝这个属性值可以使用如下方式：

```python
	import Math
	self.copyPosition = Math.
Vector3
( self.position )
```

类型：

- Vector3

<a id="spaceID"></a>

spaceID

这个属性是实体所在的空间的ID，cell与客户端这个值都保持一致。

类型：

- 只读的，Integer

<a id="topSpeed"></a>

topSpeed

实体的最大xz轴移动速度（米/秒），这个属性通常要比实际移动速度要大一些，服务端通过这个属性检查客户端的移动合法性，如果移动距离超出速度限制则被强制拉回上一个坐标位置。

其他参考：

- Entity.[topSpeedY](./Entity.md#topSpeedY)

类型：

- float

<a id="topSpeedY"></a>

topSpeedY

实体的最大y轴移动速度（米/秒），这个属性通常要比实际移动速度要大一些，服务端通过这个属性检查客户端的移动合法性，如果移动距离超出速度限制则被强制拉回上一个坐标位置。

其他参考：

- Entity.[topSpeed](./Entity.md#topSpeed)

类型：

- float

<a id="volatileInfo"></a>

volatileInfo

这个属性指定Entity的易变类数据同步到客户端的策略。
易变类数据包括实体的坐标position和实体的朝向direction，易变类数据由于极易改变的特性，引擎底层使用了一套优化的方案将其同步到客户端。
这个属性是四个float（position，yaw，pitch，roll）代表距离值，当一个实体靠近当前实体达到距离则服务端向其同步相关数据。如果距离值大于View半径则代表总是同步。
还有一个特殊的bool属性optimized，它的作用是控制服务器同步时是否进行优化，目前主要的优化是Y轴。
如果为true，在一些行为(如：navigate)导致服务器能确定实体在地面时，服务器不同步实体的Y轴坐标，当同步大量实体时能节省大量带宽，默认为true。
用户也可以在.def制定不同实体的同步策略：

```python
<Volatile>
    <position/>           <!-- 总是同步 -->
    <yaw/>                <!-- 总是同步 -->
    <pitch>20</pitch>     <!-- 相距20米或以内同步     -->
    <optimized> true </optimized>   
</Volatile>               <!-- roll未指明则总是同步  -->
```

类型：

- sequence， 四个浮点数(float, float, float, float)
