# Entity类

[KBEngine模块]

Entity是KBEngine模块的一部分。[更多...](#detailed_description)

```python
import KBEngine
```

## 成员函数

- def [addTimer](#addTimer)( self, initialOffset, repeatOffset=0, userArg=0 ):
- def [createCellEntity](#createCellEntity)( self, cellEntityCall ):
- def [createCellEntityInNewSpace](#createCellEntityInNewSpace)( self, cellappIndex ):
- def [delTimer](#delTimer)( self, id ):
- def [destroy](#destroy)( self, deleteFromDB, writeToDB ):
- def [destroyCellEntity](#destroyCellEntity)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):
- def [writeToDB](#writeToDB)( self, callback, shouldAutoLoad, dbInterfaceName ):

## 回调函数

- def [onCreateCellFailure](#onCreateCellFailure)( self ):
- def [onDestroy](#onDestroy)( self ):
- def [onGetCell](#onGetCell)( self ):
- def [onLoseCell](#onLoseCell)( self ):
- def [onPreArchive](#onPreArchive)( self ):
- def [onRestore](#onRestore)( self ):
- def [onTimer](#onTimer)( self, timerHandle, userData ):
- def [onWriteToDB](#onWriteToDB)( self, cellData ):

## 属性

| [cell](#cell) | 只读CellEntityCall |
| --- | --- |
| [cellData](#cellData) | CELLDATADICT |
| [className](#className) | 只读string |
| [client](#client) | 只读ClientEntityCall |
| [databaseID](#databaseID) | 只读int64 |
| [databaseInterfaceName](#databaseInterfaceName) | 只读string |
| [id](#id) | 只读int32 |
| [isDestroyed](#isDestroyed) | bool |
| [shouldAutoArchive](#shouldAutoArchive) | True, False orKBEngine.NEXT_ONLY |
| [shouldAutoBackup](#shouldAutoBackup) | True, False orKBEngine.NEXT_ONLY |

<a id="detailed_description"></a>

## 详细描述

[Entity](./Entity.md)类代表一个驻留在Baseapp上的实体。[Entity](./Entity.md)实体可以通过KBEngine.createEntity函数(以及以createEntity为前缀的函数)创建。一个[Entity](./Entity.md)实体也可以通过Cellapp的函数KBEngine.createEntityOnBaseApp远程创建。
一个[Entity](./Entity.md)实体可以在一个活动的cells里面链接到一个实体，还可以被用来在一个合适的cell上创建一个关联的实体。
这个类允许你创建和销毁在cell上的实体或者在base实体上注册一个定时器也或者访问此对象的联系信息，
还可以访问一个CellEntityCall，通过它这个base实体可以与它的cell实体通信（这个被关联的cell实体可以移动到不同cell上，作为该cell实体移动的结果KBEngine会进行负载均衡）。

## 成员函数文档

<a id="addTimer"></a>

### def addTimer(self, initialOffset, repeatOffset=0, userArg=0):

功能说明：
注册一个定时器，定时器由回调函数[onTimer](#onTimer)触发，回调函数将在"initialOffset"秒后被执行第1次，而后将每间隔"repeatOffset"秒执行1次，可设定一个用户参数"userArg"（仅限integer类型）。
[onTimer](#onTimer)函数必须在entity的base部分被定义，且带有2个参数，第1个integer类型的是timer的id（可用于移除timer的"[delTimer](#delTimer)"函数），第2个是用户参数"userArg"。
例子:

```python
# 这里是使用addTimer的一个例子
import 
KBEngine

 
class MyBaseEntity( 
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
        print "MyBaseEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
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

<a id="createCellEntity"></a>

### def createCellEntity(self, cellEntityCall):

功能说明：
请求在指定 `CellEntityCall` 所在的空间里创建当前 Base 实体对应的 cell 实体。
它不是在 Base 进程里本地直接构造一个 cell 对象，而是由 Base 把 `entityType`、`baseapp componentID`、是否带客户端以及 `cellData` 序列化后发给目标 CellApp，再由对端完成真正的 Cell 实例创建。
创建成功前 `cell` 属性仍不可用；Base 侧真正拿到直接 `cellEntityCall` 的时机，是后续收到回调并触发 [onGetCell](#onGetCell) 的时候。

源码解析：

- [实体系统：`Entity.createCellEntity()` 不是本地建对象，而是一次 Base -> Cell 交接](/architecture/source-analysis/entity-system.html#base-entity-create-cell)

参数：

| cellEntityCall | 一个可选的CellEntityCall参数， 指定哪个空间里创建这个cell实体的。 只能使用一个直接的CellEntityCall。如果你有一个实体的BaseEntityCall，你不可以使用baseEntityCall.cell传给这个函数。 你必须在这个实体的base上创建一个新的函数来回传这个直接的CellEntityCall。 例如： ```python baseEntityCallOfNearbyEntity.createCellNearSelf( self ) ``` 在实体的base上： ```python def createCellNearSelf( self, baseEntityCall ): baseEntityCall.createCellNearHere( self.cell ) ``` 在原实体的base上调用createCellNearSelf()方法： ```python def createCellNearHere( self. cellEntityCall ): self.createCellEntity( cellEntityCall ) ``` |
| --- | --- |

<a id="createCellEntityInNewSpace"></a>

### def createCellEntityInNewSpace(self, cellappIndex):

功能说明：
在cellapp上创建一个空间(space)并且将该实体的cell创建到这个新的空间中，它请求通过cellappmgr来完成。
用于创建cell实体的信息被存储在该实体的属性cellData里。这个属性是一个字典，对应实体的.def文件里的默认值同时还包括用于表示
实体位置和方向(roll, pitch, yaw)的"position", "direction" 和 "spaceID"。

参数：

| cellappIndex | integer，为None或者0则由引擎负载均衡进行动态选择，如果大于0则在指定的cellapp中创建space。 例子：假如预期会开启4个cellapp，那么cellappIndex需要指定时索引可以是1、2、3、4， 如果实际运行的cellapp不足4个，例如只有3个，那么cellappIndex输入4时由于数量溢出4代表1，如果输入5则代表2。 提示：此功能可以结合KBngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING)来使用，例如：将大地图space放到几个固定的cellapp中，并将这些cellapp设置为不参与负载均衡，其他cellapp用于放置副本space。 创建副本space时cellappIndex设置为0或者None，副本地图的消耗将不会影响到大地图进程，从而保证了主场景的流畅。 |
| --- | --- |

<a id="delTimer"></a>

### def delTimer(self, id):

功能说明：
函数delTimer用于移除一个注册的定时器，移除后的定时器不再执行。只执行1次的定时器在执行回调后自动移除，不必要使用delTimer移除。
如果delTimer函数使用一个无效的id（例如已经移除），将会产生错误。
到[Entity](./Entity.md).[addTimer](./Entity.md#addTimer)参考定时器的一个使用例子。

参数：

| id | integer，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。 |
| --- | --- |

<a id="destroy"></a>

### def destroy(self, deleteFromDB, writeToDB):

功能说明：
这个函数销毁该实体的base部分。如果实体存在cell部分，那么用户必须先销毁cell部分，否则将会产生错误。要销毁实体的cell部分，调用[Entity](./Entity.md).[destroyCellEntity](./Entity.md#destroyCellEntity)。
也许在onLoseCell回调里调用self.destroy更为恰当。这能保证实体的base部分被销毁。

参数：

| deleteFromDB | 如果是True，在数据库里与这个实体有关联的条目将会被删除，该参数默认为False。 |
| --- | --- |
| writeToDB | 如果是True，与这个实体相关联的存档属性将会写入数据库。只有在这个实体是从数据库读取的或者是使用过[Entity](./Entity.md).[writeToDB](./Entity.md#writeToDB)写入数据库才会被执行。这个参数默认为True，但当deleteFromDB为True的时候它将被忽略。 |

<a id="destroyCellEntity"></a>

### def destroyCellEntity(self):

功能说明：
destroyCellEntity请求销毁关联的cell实体。如果没有关联的cell实体该方法将会产生错误。

<a id="writeToDB"></a>

### def writeToDB(self, callback, shouldAutoLoad, dbInterfaceName):

功能说明：
该函数保存这个实体的存档属性到数据库，使得以后需要的时候可以重新从数据库加载。
实体也可以被标记为自动加载，这样当服务启动后实体将会被重新创建。

参数：

| callback | 这个可选参数是当数据库操作完成后的回调函数。 它有两个参数。第一个是boolean类型标志成功或失败，第二个是base实体。 |
| --- | --- |
| shouldAutoLoad | 这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。 注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。 脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如： def onAutoLoadEntityCreate(entityType, dbid): KBEngine.createEntityFromDBID(entityType, dbid) |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

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

<a id="onCreateCellFailure"></a>

### def onCreateCellFailure(self):

功能说明：
如果这个函数在脚本中有实现，这个函数在cell实体创建失败的时候被调用。
这个函数没有参数。

<a id="onDestroy"></a>

### def onDestroy(self):

功能说明：
如果这个函数在脚本中有实现，这个函数在调用[Entity](./Entity.md).[destroy](./Entity.md#destroy)()后，在实际销毁之前被调用。
这个函数没有参数。

<a id="onGetCell"></a>

### def onGetCell(self):

功能说明：
如果这个函数在脚本中有实现，这个函数会在 Base 侧收到 cell 创建成功回调、并补齐当前实体的直接 `cellEntityCall` 后被调用。
底层会先结束 `creatingCell` 状态并销毁一次性的 `cellData` 启动数据，然后才进入这个脚本回调；恢复路径 `restoreCell()` 不会触发它。

源码解析：

- [实体系统：`Entity.createCellEntity()` 不是本地建对象，而是一次 Base -> Cell 交接](/architecture/source-analysis/entity-system.html#base-entity-create-cell)

这个函数没有参数。

<a id="onLoseCell"></a>

### def onLoseCell(self):

功能说明：
如果这个函数在脚本中有实现，这个函数在它关联的cell实体销毁之后被调用。
这个函数没有参数。

<a id="onPreArchive"></a>

### def onPreArchive(self):

功能说明：
如果这个函数在脚本中有实现，这个函数在该实体自动写入数据库之前被调用。这个回调在[Entity](./Entity.md).[onWriteToDB](./Entity.md#onWriteToDB)回调之前被调用。
如果该回调返回False，该归档操作中止。这个回调应该返回True使得操作继续。如果这个回调不存在，则归档操作继续进行。

<a id="onRestore"></a>

### def onRestore(self):

功能说明：
如果这个函数在脚本中有实现，这个函数在[Entity](./Entity.md)应用程序崩溃后在其它[Entity](./Entity.md)应用程序上
重新创建该实体时被调用。
这个函数没有参数。

<a id="onTimer"></a>

### def onTimer(self, timerHandle, userData):

功能说明：
这个函数当一个与此实体关联的定时器触发的时候被调用。
一个定时器可以使用[Entity](./Entity.md).[addTimer](./Entity.md#addTimer)函数添加。

参数：

| timerHandle | 定时器的id。 |
| --- | --- |
| userData | 传进[Entity](./Entity.md).[addTimer](./Entity.md#addTimer)的integer用户数据。 |

<a id="onWriteToDB"></a>

### def onWriteToDB(self, cellData):

功能说明：
如果这个函数在脚本中有实现，这个函数在实体数据将要写进数据库的时候被调用。
需要注意的是在该回调里调用writeToDB会导致无限循环。

参数：

| cellData | 包含将要存进数据库的cell属性。[cellData](./Entity.md#cellData)是一个字典。 |
| --- | --- |

## 属性文档

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

<a id="client"></a>

client

说明：
client是用于联系客户端的EntityCall。这个属性是只读的，且如果这个base实体没有关联的客户端时属性是None。

类型：

- 只读ENTITYCALL

<a id="databaseID"></a>

databaseID

说明：
databaseID是实体的永久ID(数据库id)。这个id是uint64类型且大于0，如果是0则表示该实体不是永久的。

类型：

- 只读 int64

<a id="databaseInterfaceName"></a>

databaseInterfaceName

说明：
databaseInterfaceName是实体持久化所在的数据库接口名称，该接口名称在kbengine_defaults->dbmgr中配置。实体必须持久化过（databaseID>0）该属性才可用，否则返回空字符串。

类型：

- 只读 string

<a id="id"></a>

id

说明：
id是实体的对象id。这个id是一个整型，在base，cell和client相关联的实体之间是相同的。
这个属性是只读的。

类型：

- 只读 int32

<a id="isDestroyed"></a>

isDestroyed

说明：
如果该[Entity](./Entity.md)实体已经被销毁了，这个属性为True。

类型：

- bool

<a id="shouldAutoArchive"></a>

shouldAutoArchive

说明：
这个属性决定了自动存档的策略。如果设为True，自动存档将可用，如果设为False，自动存档将不可用。
如果设为KBEngine.NEXT_ONLY，自动存档将在下一个预定的时间可用，
在下一次存档后，这个属性将置为False。

类型：

- True, False orKBEngine.NEXT_ONLY

<a id="shouldAutoBackup"></a>

shouldAutoBackup

说明：
这个属性决定了自动备份的策略。如果设为True，自动备份将可用，如果设为False，自动备份将不可用。
如果设为KBEngine.NEXT_ONLY，自动备份将在下一个预定的时间可用，
在下一次备份后，这个属性将置为False。

类型：

- True, False orKBEngine.NEXT_ONLY
