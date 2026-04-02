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
[成员函数](#)成员函数| def addTimer( self, initialOffset, repeatOffset=0, userArg=0 ): |
| --- |
| def createCellEntity( self, cellEntityCall ): |
| def createCellEntityInNewSpace( self, cellappIndex ): |
| def delTimer( self, id ): |
| def destroy( self, deleteFromDB, writeToDB ): |
| def destroyCellEntity( self ): |
| def getComponent( self, componentName, all ): |
| def fireEvent( self, eventName, *args ): |
| def registerEvent( self, eventName, callback ): |
| def deregisterEvent( self, eventName, callback ): |
| def writeToDB( self, callback, shouldAutoLoad, dbInterfaceName ): |

def[addTimer](#addTimer)addTimer( self, initialOffset, repeatOffset=0, userArg=0 ):def[createCellEntity](#createCellEntity)createCellEntity( self, cellEntityCall ):def[createCellEntityInNewSpace](#createCellEntityInNewSpace)createCellEntityInNewSpace( self, cellappIndex ):def[delTimer](#delTimer)delTimer( self, id ):def[destroy](#destroy)destroy( self, deleteFromDB, writeToDB ):def[destroyCellEntity](#destroyCellEntity)destroyCellEntity( self ):def[getComponent](#getComponent)getComponent( self, componentName, all ):def[fireEvent](#fireEvent)fireEvent( self, eventName, *args ):def[registerEvent](#registerEvent)registerEvent( self, eventName, callback ):def[deregisterEvent](#deregisterEvent)deregisterEvent( self, eventName, callback ):def[writeToDB](#writeToDB)writeToDB( self, callback, shouldAutoLoad, dbInterfaceName ):
## 回调函数
[回调函数](#)回调函数| def onCreateCellFailure( self ): |
| --- |
| def onDestroy( self ): |
| def onGetCell( self ): |
| def onLoseCell( self ): |
| def onPreArchive( self ): |
| def onRestore( self ): |
| def onTimer( self, timerHandle, userData ): |
| def onWriteToDB( self, cellData ): |

def[onCreateCellFailure](#onCreateCellFailure)onCreateCellFailure( self ):def[onDestroy](#onDestroy)onDestroy( self ):def[onGetCell](#onGetCell)onGetCell( self ):def[onLoseCell](#onLoseCell)onLoseCell( self ):def[onPreArchive](#onPreArchive)onPreArchive( self ):def[onRestore](#onRestore)onRestore( self ):def[onTimer](#onTimer)onTimer( self, timerHandle, userData ):def[onWriteToDB](#onWriteToDB)onWriteToDB( self, cellData ):
## 属性
[属性](#)属性| cell | 只读 CellEntityCall |
| --- | --- |
| cellData | CELLDATADICT |
| className | 只读 string |
| client | 只读 ClientEntityCall |
| databaseID | 只读 int64 |
| databaseInterfaceName | 只读 string |
| id | 只读 int32 |
| isDestroyed | bool |
| shouldAutoArchive | True, False or KBEngine.NEXT_ONLY |
| shouldAutoBackup | True, False or KBEngine.NEXT_ONLY |

[cell](#cell)cell只读[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall[cellData](#cellData)cellData[CELLDATADICT](CELLDATADICT.html)CELLDATADICT[className](#className)className只读[string](STRING.html)string[client](#client)client只读[ClientEntityCall](../../keywords.html#EntityCall)ClientEntityCall[databaseID](#databaseID)databaseID只读[int64](int64.html)int64[databaseInterfaceName](#databaseInterfaceName)databaseInterfaceName只读[string](STRING.html)string[id](#id)id只读[int32](int32.html)int32[isDestroyed](#isDestroyed)isDestroyed[bool](bool.html)bool[shouldAutoArchive](#shouldAutoArchive)shouldAutoArchiveTrue, False or[KBEngine](../Modules/KBEngine.html)KBEngine.[NEXT_ONLY](KBEngine.html#NEXT_ONLY)NEXT_ONLY[shouldAutoBackup](#shouldAutoBackup)shouldAutoBackupTrue, False or[KBEngine](../Modules/KBEngine.html)KBEngine.[NEXT_ONLY](KBEngine.html#NEXT_ONLY)NEXT_ONLY
---

## 详细描述
详细描述[Entity](Entity.html)Entity类代表一个驻留在[Baseapp](../index.html)Baseapp上的实体。[Entity](Entity.html)Entity实体可以通过[KBEngine](../Modules/KBEngine.html)KBEngine.[createEntity](KBEngine.html#createEntity)createEntity函数(以及以createEntity为前缀的函数)创建。一个[Entity](Entity.html)Entity实体也可以通过[Cellapp](../../cellapp/index.html)Cellapp的函数[KBEngine](../Modules/KBEngine.html)KBEngine.[createEntityOnBaseApp](../../cellapp/Modules/KBEngine.html#createEntityOnBaseApp)createEntityOnBaseApp远程创建。一个[Entity](Entity.html)Entity实体可以在一个活动的[cells](../../keywords.html#cell)cells里面链接到一个实体，还可以被用来在一个合适的[cell](../../keywords.html#cell)cell上创建一个关联的实体。
这个类允许你创建和销毁在[cell](../../keywords.html#cell)cell上的实体或者在base实体上注册一个定时器也或者访问此对象的联系信息，还可以访问一个[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall，通过它这个base实体可以与它的[cell](../../keywords.html#cell)cell实体通信（这个被关联的[cell](../../keywords.html#cell)cell实体可以移动到不同[cell](../../keywords.html#cell)cell上，作为该[cell](../../keywords.html#cell)cell实体移动的结果KBEngine会进行负载均衡）。
---

## 成员函数文档
成员函数文档def addTimer( self, initialOffset, repeatOffset=0, userArg=0 ):

def addTimer(self, initialOffset, repeatOffset=0, userArg=0):功能说明：注册一个定时器，定时器由回调函数onTimer[onTimer](#onTimer)onTimer触发，回调函数将在"initialOffset"秒后被执行第1次，而后将每间隔"repeatOffset"秒执行1次，可设定一个用户参数"userArg"（仅限integer类型）。onTimer[onTimer](#onTimer)onTimer函数必须在entity的base部分被定义，且带有2个参数，第1个integer类型的是timer的id（可用于移除timer的"[delTimer](#delTimer)delTimer"函数），第2个是用户参数"userArg"。例子:
```
# 这里是使用addTimer的一个例子
import KBEngine
 
class MyBaseEntity( KBEngine.Entity ):
 
    def __init__( self ):
        KBEngine.Entity.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    # Entity的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyBaseEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if 这是不断重复的定时器，当不再需要该定时器的时候，调用下面函数移除:
        #     self.delTimer( id )

```

```
# 这里是使用addTimer的一个例子
import KBEngine
 
class MyBaseEntity( KBEngine.Entity ):
 
    def __init__( self ):
        KBEngine.Entity.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    # Entity的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyBaseEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if 这是不断重复的定时器，当不再需要该定时器的时候，调用下面函数移除:
        #     self.delTimer( id )
```
# 这里是使用addTimer的一个例子
import[KBEngine](../Modules/KBEngine.html)KBEngineclass MyBaseEntity([KBEngine](../Modules/KBEngine.html)KBEngine.[Entity](Entity.html)Entity):
 
    def __init__( self ):[KBEngine](../Modules/KBEngine.html)KBEngine.[Entity](Entity.html)Entity.__init__( self )
 
        # 增加一个定时器，5秒后执行第1次，而后每1秒执行1次，用户参数是9
        self.addTimer( 5, 1, 9 )
 
        # 增加一个定时器，1秒后执行，用户参数缺省是0
        self.addTimer( 1 )
 
    #[Entity](Entity.html)Entity的定时器回调"onTimer"被调用
    def onTimer( self, id, userArg ):
        print "MyBaseEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
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

integer，该函数返回timer的内部id，这个id可用于[delTimer](#delTimer)delTimer移除定时器。def createCellEntity( self, cellEntityCall ):

def createCellEntity(self, cellEntityCall):功能说明：请求在一个[cell](../../keywords.html#cell)cell里面创建一个关联的实体。用于创建[cell](../../keywords.html#cell)cell实体的信息被存储在该实体的属性cellData里。这个属性是一个字典，对应实体的.def文件里的默认值，同时还包括用于表示实体位置和方向(roll, pitch, yaw)的"position", "direction" 和 "spaceID"。参数：

cellEntityCall
一个可选的CellEntityCall参数，
指定哪个空间里创建这个cell实体的。

只能使用一个直接的CellEntityCall。如果你有一个实体的BaseEntityCall，你不可以使用baseEntityCall.cell传给这个函数。
你必须在这个实体的base上创建一个新的函数来回传这个直接的CellEntityCall。

例如：
baseEntityCallOfNearbyEntity.createCellNearSelf( self )
在实体的base上：
def createCellNearSelf( self, baseEntityCall ):
    baseEntityCall.createCellNearHere( self.cell )
在原实体的base上调用createCellNearSelf()方法：
def createCellNearHere( self. cellEntityCall ):
    self.createCellEntity( cellEntityCall )

参数：| cellEntityCall | 一个可选的CellEntityCall参数，
指定哪个空间里创建这个cell实体的。

只能使用一个直接的CellEntityCall。如果你有一个实体的BaseEntityCall，你不可以使用baseEntityCall.cell传给这个函数。
你必须在这个实体的base上创建一个新的函数来回传这个直接的CellEntityCall。

例如：
baseEntityCallOfNearbyEntity.createCellNearSelf( self )
在实体的base上：
def createCellNearSelf( self, baseEntityCall ):
    baseEntityCall.createCellNearHere( self.cell )
在原实体的base上调用createCellNearSelf()方法：
def createCellNearHere( self. cellEntityCall ):
    self.createCellEntity( cellEntityCall ) |

cellEntityCall一个可选的[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall参数，
指定哪个空间里创建这个cell实体的。只能使用一个直接的[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall。如果你有一个实体的[BaseEntityCall](../../keywords.html#EntityCall)BaseEntityCall，你不可以使用baseEntityCall.cell传给这个函数。你必须在这个实体的base上创建一个新的函数来回传这个直接的[CellEntityCall](../../keywords.html#EntityCall)CellEntityCall。例如：
```
baseEntityCallOfNearbyEntity.createCellNearSelf( self )
```
baseEntityCallOfNearbyEntity.createCellNearSelf( self )在实体的base上：
```
def createCellNearSelf( self, baseEntityCall ):
    baseEntityCall.createCellNearHere( self.cell )
```
def createCellNearSelf( self, baseEntityCall ):
    baseEntityCall.createCellNearHere( self.cell )在原实体的base上调用createCellNearSelf()方法：
```
def createCellNearHere( self. cellEntityCall ):
    self.createCellEntity( cellEntityCall )
```
def createCellNearHere( self. cellEntityCall ):
    self.createCellEntity( cellEntityCall )def createCellEntityInNewSpace( self, cellappIndex ):

def createCellEntityInNewSpace(self, cellappIndex):功能说明：在cellapp上创建一个空间(space)并且将该实体的[cell](../../keywords.html#cell)cell创建到这个新的空间中，它请求通过cellappmgr来完成。用于创建cell实体的信息被存储在该实体的属性cellData里。这个属性是一个字典，对应实体的.def文件里的默认值同时还包括用于表示实体位置和方向(roll, pitch, yaw)的"position", "direction" 和 "spaceID"。参数：

cellappIndex
integer，为None或者0则由引擎负载均衡进行动态选择，如果大于0则在指定的cellapp中创建space。
例子：假如预期会开启4个cellapp，那么cellappIndex需要指定时索引可以是1、2、3、4，
如果实际运行的cellapp不足4个，例如只有3个，那么cellappIndex输入4时由于数量溢出4代表1，如果输入5则代表2。
提示：此功能可以结合KBngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING)来使用，例如：将大地图space放到几个固定的cellapp中，并将这些cellapp设置为不参与负载均衡，其他cellapp用于放置副本space。
创建副本space时cellappIndex设置为0或者None，副本地图的消耗将不会影响到大地图进程，从而保证了主场景的流畅。

参数：| cellappIndex | integer，为None或者0则由引擎负载均衡进行动态选择，如果大于0则在指定的cellapp中创建space。
例子：假如预期会开启4个cellapp，那么cellappIndex需要指定时索引可以是1、2、3、4，
如果实际运行的cellapp不足4个，例如只有3个，那么cellappIndex输入4时由于数量溢出4代表1，如果输入5则代表2。
提示：此功能可以结合KBngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING)来使用，例如：将大地图space放到几个固定的cellapp中，并将这些cellapp设置为不参与负载均衡，其他cellapp用于放置副本space。
创建副本space时cellappIndex设置为0或者None，副本地图的消耗将不会影响到大地图进程，从而保证了主场景的流畅。 |

cellappIndexinteger，为None或者0则由引擎负载均衡进行动态选择，如果大于0则在指定的cellapp中创建space。例子：假如预期会开启4个cellapp，那么cellappIndex需要指定时索引可以是1、2、3、4，如果实际运行的cellapp不足4个，例如只有3个，那么cellappIndex输入4时由于数量溢出4代表1，如果输入5则代表2。提示：此功能可以结合KBngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING)来使用，例如：将大地图space放到几个固定的cellapp中，并将这些cellapp设置为不参与负载均衡，其他cellapp用于放置副本space。
创建副本space时cellappIndex设置为0或者None，副本地图的消耗将不会影响到大地图进程，从而保证了主场景的流畅。def delTimer( self, id ):

def delTimer(self, id):功能说明：函数delTimer用于移除一个注册的定时器，移除后的定时器不再执行。只执行1次的定时器在执行回调后自动移除，不必要使用delTimer移除。
如果delTimer函数使用一个无效的id（例如已经移除），将会产生错误。到[Entity](Entity.html)Entity.[addTimer](Entity.html#addTimer)addTimer参考定时器的一个使用例子。参数：

id
integer，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。

参数：| id | integer，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。 |

idinteger，它指定要移除的定时器id。如果参数为字符串"All"，则一次性移除所有的定时器。def destroy( self, deleteFromDB, writeToDB ):

def destroy(self, deleteFromDB, writeToDB):功能说明：这个函数销毁该实体的base部分。如果实体存在cell部分，那么用户必须先销毁cell部分，否则将会产生错误。要销毁实体的cell部分，调用[Entity](Entity.html)Entity.[destroyCellEntity](Entity.html#destroyCellEntity)destroyCellEntity。也许在onLoseCell回调里调用self.destroy更为恰当。这能保证实体的base部分被销毁。参数：

deleteFromDB
如果是True，在数据库里与这个实体有关联的条目将会被删除，该参数默认为False。

writeToDB
如果是True，与这个实体相关联的存档属性将会写入数据库。只有在这个实体是从数据库读取的或者是使用过Entity.writeToDB写入数据库才会被执行。这个参数默认为True，但当deleteFromDB为True的时候它将被忽略。

参数：| deleteFromDB | 如果是True，在数据库里与这个实体有关联的条目将会被删除，该参数默认为False。 |
| --- | --- |
| writeToDB | 如果是True，与这个实体相关联的存档属性将会写入数据库。只有在这个实体是从数据库读取的或者是使用过Entity.writeToDB写入数据库才会被执行。这个参数默认为True，但当deleteFromDB为True的时候它将被忽略。 |

deleteFromDB如果是True，在数据库里与这个实体有关联的条目将会被删除，该参数默认为False。writeToDB如果是True，与这个实体相关联的存档属性将会写入数据库。只有在这个实体是从数据库读取的或者是使用过[Entity](Entity.html)Entity.[writeToDB](Entity.html#writeToDB)writeToDB写入数据库才会被执行。这个参数默认为True，但当deleteFromDB为True的时候它将被忽略。def destroyCellEntity( self ):

def destroyCellEntity(self):功能说明：destroyCellEntity请求销毁关联的cell实体。如果没有关联的cell实体该方法将会产生错误。def writeToDB( self, callback, shouldAutoLoad, dbInterfaceName ):

def writeToDB(self, callback, shouldAutoLoad, dbInterfaceName):功能说明：该函数保存这个实体的存档属性到数据库，使得以后需要的时候可以重新从数据库加载。实体也可以被标记为自动加载，这样当服务启动后实体将会被重新创建。参数：

callback
这个可选参数是当数据库操作完成后的回调函数。
它有两个参数。第一个是boolean类型标志成功或失败，第二个是base实体。

shouldAutoLoad
这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。
注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。
脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如：
def onAutoLoadEntityCreate(entityType, dbid): 
          KBEngine.createEntityFromDBID(entityType, dbid)

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| callback | 这个可选参数是当数据库操作完成后的回调函数。
它有两个参数。第一个是boolean类型标志成功或失败，第二个是base实体。 |
| --- | --- |
| shouldAutoLoad | 这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。
注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。
脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如：
def onAutoLoadEntityCreate(entityType, dbid): 
          KBEngine.createEntityFromDBID(entityType, dbid) |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

callback这个可选参数是当数据库操作完成后的回调函数。
它有两个参数。第一个是boolean类型标志成功或失败，第二个是base实体。shouldAutoLoad这个可选参数指定这个实体在服务启动的时候是否需要从数据库加载。注意：服务器启动时自动加载实体，底层默认将会调用createEntityAnywhereFromDBID将实体创建到一个负载最小的baseapp上，整个过程将会在第一个启动的baseapp调用onBaseAppReady之前完成。脚本层可以在个性化脚本(kbengine_defaults.xml->baseapp->entryScriptFile定义)中重新实现实体的创建方法，例如：def onAutoLoadEntityCreate(entityType, dbid):KBEngine.createEntityFromDBID(entityType, dbid)dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。def getComponent( self, componentName, all ):

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
回调函数文档def onCreateCellFailure( self ):

def onCreateCellFailure(self):功能说明：如果这个函数在脚本中有实现，这个函数在cell实体创建失败的时候被调用。
这个函数没有参数。def onDestroy( self ):

def onDestroy(self):功能说明：如果这个函数在脚本中有实现，这个函数在调用[Entity](Entity.html)Entity.[destroy](Entity.html#destroy)destroy()后，在实际销毁之前被调用。
这个函数没有参数。def onGetCell( self ):

def onGetCell(self):功能说明：如果这个函数在脚本中有实现，这个函数在它获得cell实体的时候被调用。
这个函数没有参数。def onLoseCell( self ):

def onLoseCell(self):功能说明：如果这个函数在脚本中有实现，这个函数在它关联的cell实体销毁之后被调用。
这个函数没有参数。def onPreArchive( self ):

def onPreArchive(self):功能说明：如果这个函数在脚本中有实现，这个函数在该实体自动写入数据库之前被调用。这个回调在[Entity](Entity.html)Entity.[onWriteToDB](Entity.html#onWriteToDB)onWriteToDB回调之前被调用。如果该回调返回False，该归档操作中止。这个回调应该返回True使得操作继续。如果这个回调不存在，则归档操作继续进行。def onRestore( self ):

def onRestore(self):功能说明：如果这个函数在脚本中有实现，这个函数在[Entity](Entity.html)Entity应用程序崩溃后在其它[Entity](Entity.html)Entity应用程序上
重新创建该实体时被调用。
这个函数没有参数。def onTimer( self, timerHandle, userData ):

def onTimer(self, timerHandle, userData):功能说明：这个函数当一个与此实体关联的定时器触发的时候被调用。
一个定时器可以使用[Entity](Entity.html)Entity.[addTimer](Entity.html#addTimer)addTimer函数添加。参数：

timerHandle
定时器的id。

userData
传进Entity.addTimer的integer用户数据。

参数：| timerHandle | 定时器的id。 |
| --- | --- |
| userData | 传进Entity.addTimer的integer用户数据。 |

timerHandle定时器的id。userData传进[Entity](Entity.html)Entity.[addTimer](Entity.html#addTimer)addTimer的integer用户数据。def onWriteToDB( self, cellData ):

def onWriteToDB(self, cellData):功能说明：如果这个函数在脚本中有实现，这个函数在实体数据将要写进数据库的时候被调用。需要注意的是在该回调里调用writeToDB会导致无限循环。参数：

cellData
包含将要存进数据库的cell属性。
cellData是一个字典。

参数：| cellData | 包含将要存进数据库的cell属性。
cellData是一个字典。 |

cellData包含将要存进数据库的cell属性。[cellData](Entity.html#cellData)cellData是一个字典。
---

## 属性文档
属性文档cell

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

只读，stringclient

client说明：client是用于联系客户端的EntityCall。这个属性是只读的，且如果这个base实体没有关联的客户端时属性是None。类型：

只读 ENTITYCALL

类型：| 只读 ENTITYCALL |

只读[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALLdatabaseID

databaseID说明：databaseID是实体的永久ID(数据库id)。这个id是uint64类型且大于0，如果是0则表示该实体不是永久的。类型：

只读 int64

类型：| 只读 int64 |

只读 int64databaseInterfaceName

databaseInterfaceName说明：databaseInterfaceName是实体持久化所在的数据库接口名称，该接口名称在kbengine_defaults->dbmgr中配置。实体必须持久化过（databaseID>0）该属性才可用，否则返回空字符串。类型：

只读 string

类型：| 只读 string |

只读 stringid

id说明：id是实体的对象id。这个id是一个整型，在base，cell和client相关联的实体之间是相同的。
这个属性是只读的。类型：

只读 int32

类型：| 只读 int32 |

只读 int32isDestroyed

isDestroyed说明：如果该[Entity](Entity.html)Entity实体已经被销毁了，这个属性为True。类型：

bool

类型：| bool |

boolshouldAutoArchive

shouldAutoArchive说明：这个属性决定了自动存档的策略。如果设为True，自动存档将可用，如果设为False，自动存档将不可用。
如果设为[KBEngine](../Modules/KBEngine.html)KBEngine.[NEXT_ONLY](KBEngine.html#NEXT_ONLY)NEXT_ONLY，自动存档将在下一个预定的时间可用，
在下一次存档后，这个属性将置为False。类型：

True, False or KBEngine.NEXT_ONLY

类型：| True, False or KBEngine.NEXT_ONLY |

True, False or[KBEngine](../Modules/KBEngine.html)KBEngine.[NEXT_ONLY](KBEngine.html#NEXT_ONLY)NEXT_ONLYshouldAutoBackup

shouldAutoBackup说明：这个属性决定了自动备份的策略。如果设为True，自动备份将可用，如果设为False，自动备份将不可用。
如果设为[KBEngine](../Modules/KBEngine.html)KBEngine.[NEXT_ONLY](KBEngine.html#NEXT_ONLY)NEXT_ONLY，自动备份将在下一个预定的时间可用，
在下一次备份后，这个属性将置为False。类型：

True, False or KBEngine.NEXT_ONLY

类型：| True, False or KBEngine.NEXT_ONLY |

True, False or[KBEngine](../Modules/KBEngine.html)KBEngine.[NEXT_ONLY](KBEngine.html#NEXT_ONLY)NEXT_ONLY
---
版权归KBEngine所有。

版权归KBEngine所有。