# KBEngine模块

| KBEngine |

KBEngine
---

# KBEngine模块
KBEngine模块[KBEngine](KBEngine.html)KBEngine模块提供了Python脚本访问[实体](../Classes/Entity.html)实体的部分，特别是它提供了定时器的注册与移除，
以及[实体](../Classes/Entity.html)实体的创建。
## 类
[类](#)类| Entity |
| --- |
| Proxy |

[Entity](../Classes/Entity.html)Entity[Proxy](../Classes/Proxy.html)Proxy
## 成员函数
[成员函数](#)成员函数| def addWatcher( path, dataType, getFunction ): |
| --- |
| def address(  ): |
| def MemoryStream(  ): |
| def charge( ordersID, dbID, byteDatas, pycallback ): |
| def createEntity(  ): |
| def createEntityAnywhere( entityType, *params, callback ): |
| def createEntityRemotely( entityType, baseMB, *params, callback ): |
| def createEntityFromDBID( entityType, dbID, callback, dbInterfaceName ): |
| def createEntityAnywhereFromDBID( entityType, dbID, callback, dbInterfaceName ): |
| def createEntityRemotelyFromDBID( entityType, dbID, baseMB, callback, dbInterfaceName ): |
| def createEntityLocally( entityType, *params ): |
| def debugTracing(  ): |
| def delWatcher( path ): |
| def deleteEntityByDBID( entityType, dbID, callback, dbInterfaceName ): |
| def deregisterReadFileDescriptor( fileDescriptor ): |
| def deregisterWriteFileDescriptor( fileDescriptor ): |
| def executeRawDatabaseCommand( command, callback, threadID, dbInterfaceName ): |
| def genUUID64(  ): |
| def getResFullPath( res ): |
| def getWatcher( path ): |
| def getWatcherDir( path ): |
| def getAppFlags( ): |
| def hasRes( res ): |
| def isShuttingDown(  ): |
| def listPathRes( path, extension ): |
| def lookUpEntityByDBID( entityType, dbID, callback, dbInterfaceName ): |
| def matchPath( res ): |
| def open( res, mode, encoding ): |
| def publish(  ): |
| def quantumPassedPercent(  ): |
| def registerReadFileDescriptor( fileDescriptor, callback ): |
| def registerWriteFileDescriptor( fileDescriptor, callback ): |
| def reloadScript( fullReload ): |
| def scriptLogType( logType ): |
| def setAppFlags( flags ): |
| def time(  ): |
| def urlopen( url, callback, postData, headers ): |

def[addWatcher](#addWatcher)addWatcher( path, dataType, getFunction ):def[address](#address)address(  ):def[MemoryStream](#MemoryStream)MemoryStream(  ):def[charge](#charge)charge( ordersID, dbID, byteDatas, pycallback ):def[createEntity](#createEntity)createEntity(  ):def[createEntityAnywhere](#createEntityAnywhere)createEntityAnywhere( entityType, *params, callback ):def[createEntityRemotely](#createEntityRemotely)createEntityRemotely( entityType, baseMB, *params, callback ):def[createEntityFromDBID](#createEntityFromDBID)createEntityFromDBID( entityType, dbID, callback, dbInterfaceName ):def[createEntityAnywhereFromDBID](#createEntityAnywhereFromDBID)createEntityAnywhereFromDBID( entityType, dbID, callback, dbInterfaceName ):def[createEntityRemotelyFromDBID](#createEntityRemotelyFromDBID)createEntityRemotelyFromDBID( entityType, dbID, baseMB, callback, dbInterfaceName ):def[createEntityLocally](#createEntityLocally)createEntityLocally( entityType, *params ):def[debugTracing](#debugTracing)debugTracing(  ):def[delWatcher](#delWatcher)delWatcher( path ):def[deleteEntityByDBID](#deleteEntityByDBID)deleteEntityByDBID( entityType, dbID, callback, dbInterfaceName ):def[deregisterReadFileDescriptor](#deregisterReadFileDescriptor)deregisterReadFileDescriptor( fileDescriptor ):def[deregisterWriteFileDescriptor](#deregisterWriteFileDescriptor)deregisterWriteFileDescriptor( fileDescriptor ):def[executeRawDatabaseCommand](#executeRawDatabaseCommand)executeRawDatabaseCommand( command, callback, threadID, dbInterfaceName ):def[genUUID64](#genUUID64)genUUID64(  ):def[getResFullPath](#getResFullPath)getResFullPath( res ):def[getWatcher](#getWatcher)getWatcher( path ):def[getWatcherDir](#getWatcherDir)getWatcherDir( path ):def[getAppFlags](#getAppFlags)getAppFlags( ):def[hasRes](#hasRes)hasRes( res ):def[isShuttingDown](#isShuttingDown)isShuttingDown(  ):def[listPathRes](#listPathRes)listPathRes( path, extension ):def[lookUpEntityByDBID](#lookUpEntityByDBID)lookUpEntityByDBID( entityType, dbID, callback, dbInterfaceName ):def[matchPath](#matchPath)matchPath( res ):def[open](#open)open( res, mode, encoding ):def[publish](#publish)publish(  ):def[quantumPassedPercent](#quantumPassedPercent)quantumPassedPercent(  ):def[registerReadFileDescriptor](#registerReadFileDescriptor)registerReadFileDescriptor( fileDescriptor, callback ):def[registerWriteFileDescriptor](#registerWriteFileDescriptor)registerWriteFileDescriptor( fileDescriptor, callback ):def[reloadScript](#reloadScript)reloadScript( fullReload ):def[scriptLogType](#scriptLogType)scriptLogType( logType ):def[setAppFlags](#setAppFlags)setAppFlags( flags ):def[time](#time)time(  ):def[urlopen](#urlopen)urlopen( url, callback, postData, headers ):
## 回调函数
[回调函数](#)回调函数| def onBaseAppReady( isBootstrap ): |
| --- |
| def onBaseAppShutDown( state ): |
| def onCellAppDeath( addr ): |
| def onFini(  ): |
| def onBaseAppData( key, value ): |
| def onBaseAppDataDel( key ): |
| def onGlobalData( key, value ): |
| def onGlobalDataDel( key ): |
| def onInit( isReload ): |
| def onLoseChargeCB( orderID, dbID, success, datas ): |
| def onReadyForLogin( isBootstrap ): |
| def onReadyForShutDown( ): |
| def onAutoLoadEntityCreate( entityType, dbID ): |

def[onBaseAppReady](#onBaseAppReady)onBaseAppReady( isBootstrap ):def[onBaseAppShutDown](#onBaseAppShutDown)onBaseAppShutDown( state ):def[onCellAppDeath](#onCellAppDeath)onCellAppDeath( addr ):def[onFini](#onFini)onFini(  ):def[onBaseAppData](#onBaseAppData)onBaseAppData( key, value ):def[onBaseAppDataDel](#onBaseAppDataDel)onBaseAppDataDel( key ):def[onGlobalData](#onGlobalData)onGlobalData( key, value ):def[onGlobalDataDel](#onGlobalDataDel)onGlobalDataDel( key ):def[onInit](#onInit)onInit( isReload ):def[onLoseChargeCB](#onLoseChargeCB)onLoseChargeCB( orderID, dbID, success, datas ):def[onReadyForLogin](#onReadyForLogin)onReadyForLogin( isBootstrap ):def[onReadyForShutDown](#onReadyForShutDown)onReadyForShutDown( ):def[onAutoLoadEntityCreate](#onAutoLoadEntityCreate)onAutoLoadEntityCreate( entityType, dbID ):
## 属性
[属性](#)属性| LOG_ON_ACCEPT |  |
| --- | --- |
| LOG_ON_REJECT |  |
| LOG_ON_WAIT_FOR_DESTROY |  |
| LOG_TYPE_DBG |  |
| LOG_TYPE_ERR |  |
| LOG_TYPE_INFO |  |
| LOG_TYPE_NORMAL |  |
| LOG_TYPE_WAR |  |
| NEXT_ONLY |  |
| component | 只读 string |
| entities | Entities |
| baseAppData | GlobalDataClient |
| globalData | GlobalDataClient |

[LOG_ON_ACCEPT](#LOG_ON_ACCEPT)LOG_ON_ACCEPT[LOG_ON_REJECT](#LOG_ON_REJECT)LOG_ON_REJECT[LOG_ON_WAIT_FOR_DESTROY](#LOG_ON_WAIT_FOR_DESTROY)LOG_ON_WAIT_FOR_DESTROY[LOG_TYPE_DBG](#LOG_TYPE_DBG)LOG_TYPE_DBG[LOG_TYPE_ERR](#LOG_TYPE_ERR)LOG_TYPE_ERR[LOG_TYPE_INFO](#LOG_TYPE_INFO)LOG_TYPE_INFO[LOG_TYPE_NORMAL](#LOG_TYPE_NORMAL)LOG_TYPE_NORMAL[LOG_TYPE_WAR](#LOG_TYPE_WAR)LOG_TYPE_WAR[NEXT_ONLY](#NEXT_ONLY)NEXT_ONLY[component](#component)component只读[string](STRING.html)string[entities](#entities)entities[Entities](Entities.html)Entities[baseAppData](#baseAppData)baseAppData[GlobalDataClient](baseAppData.html)GlobalDataClient[globalData](#globalData)globalData[GlobalDataClient](baseAppData.html)GlobalDataClient
---

## 成员函数文档
成员函数文档def addWatcher( path, dataType, getFunction ):

def addWatcher(path, dataType, getFunction):功能说明：与调试监视系统交互，允许用户向监视系统注册一个监视变量。例：
```

>>> def countPlayers( ):
>>>     i = 0
>>>     for e in KBEngine.entities.values():
>>>     	if e.__class__.__name__ == "Avatar":
>>>     		i += 1
>>>     return i
>>>
>>> KBEngine.addWatcher( "players", "UINT32", countPlayers )

```
>>> def countPlayers( ):
>>>     i = 0
>>>     for e in KBEngine.entities.values():
>>>     	if e.__class__.__name__ == "Avatar":
>>>     		i += 1
>>>     return i
>>>
>>>[KBEngine](KBEngine.html)KBEngine.[addWatcher](KBEngine.html#addWatcher)addWatcher( "players", "UINT32", countPlayers )这个函数添加一个监视变量在"scripts/players"监视路径之下。函数countPlayers在观察者观察时被调用。参数：

path
创建监视的路径。

dataType
监视变量的值类型。参考: 基本类型

getFunction
这个函数当观察者检索该变量时调用。
这个函数不带参数返回一个代表监视变量的值。

参数：| path | 创建监视的路径。 |
| --- | --- |
| dataType | 监视变量的值类型。参考: 基本类型 |
| getFunction | 这个函数当观察者检索该变量时调用。
这个函数不带参数返回一个代表监视变量的值。 |

path创建监视的路径。dataType监视变量的值类型。参考:[基本类型](../../basetypes.html)基本类型getFunction这个函数当观察者检索该变量时调用。
这个函数不带参数返回一个代表监视变量的值。def address(  ):

def address():功能说明：返回内部网络接口的地址。def MemoryStream(  ):

def MemoryStream():功能说明：返回一个新的MemoryStream对象。MemoryStream对象存储的是二进制信息，提供这个类型是为了让用户能够方便的序列化与反序列化Python基本类型同时能与KBEngine底层序列化规则相同。例如：你可以使用这个对象构造一个KBEngine能解析的网络数据包。用法：
```

>>> s = KBEngine.MemoryStream()
>>> s
>>> b''
>>> s.append("UINT32", 1)
>>> s.pop("UINT32")
>>> 1

```
>>> s = KBEngine.MemoryStream()
>>> s
>>> b''
>>> s.append("UINT32", 1)
>>> s.pop("UINT32")
>>> 1目前MemoryStream能够支持的类型仅为基本数据类型。参考:[基本类型](../../basetypes.html)基本类型def charge( ordersID, dbID, byteDatas, pycallback ):

def charge(ordersID, dbID, byteDatas, pycallback):功能说明：计费接口。参数：

ordersID
string，订单ID。

dbID
uint64，实体的databaseID。

byteDatas
bytes，附带数据，由开发者自己解析和定义。

pycallback
计费回调。
计费回调原型: (当在interfaces中调用KBEngine.chargeResponse后，如果某个订单设置过回调则该回调被调用)
def on**ChargeCB(self, orderID, dbID, success, datas):
ordersID：string，订单ID
dbID：uint64，通常为entity的databaseID
success：bool，是否成功
datas：bytes，附带数据，由开发者自己解析和定义

参数：| ordersID | string，订单ID。 |
| --- | --- |
| dbID | uint64，实体的databaseID。 |
| byteDatas | bytes，附带数据，由开发者自己解析和定义。 |
| pycallback | 计费回调。
计费回调原型: (当在interfaces中调用KBEngine.chargeResponse后，如果某个订单设置过回调则该回调被调用)
def on**ChargeCB(self, orderID, dbID, success, datas):
ordersID：string，订单ID
dbID：uint64，通常为entity的databaseID
success：bool，是否成功
datas：bytes，附带数据，由开发者自己解析和定义 |

ordersIDstring，订单ID。dbIDuint64，实体的[databaseID](../Classes/Entity.html#databaseID)databaseID。byteDatasbytes，附带数据，由开发者自己解析和定义。pycallback计费回调。计费回调原型: (当在interfaces中调用KBEngine.chargeResponse后，如果某个订单设置过回调则该回调被调用)def on**ChargeCB(self, orderID, dbID, success, datas):ordersID：string，订单IDdbID：uint64，通常为entity的[databaseID](../Classes/Entity.html#databaseID)databaseIDsuccess：bool，是否成功datas：bytes，附带数据，由开发者自己解析和定义def createEntity(  ):

def createEntity():功能说明：[KBEngine](KBEngine.html)KBEngine.[createEntityLocally](KBEngine.html#createEntityLocally)createEntityLocally的别名.def createEntityAnywhere( entityType, params, callback ):

def createEntityAnywhere(entityType, params, callback):功能说明：创建一个新的[Entity](../Classes/Entity.html)Entity实体，
服务端可能选择任何的[Baseapp](../index.html)Baseapp来创建[Entity](../Classes/Entity.html)Entity实体。这个方法应作为[KBEngine](KBEngine.html)KBEngine.[createEntityLocally](KBEngine.html#createEntityLocally)createEntityLocally的首选，
这样服务端会灵活地选择一个合适的[Baseapp](../index.html)Baseapp来创建实体。函数参数需要提供创建的实体的类型，还有一个Python字典作为参数来初始化实体的值。这个Python字典不需要用户提供所有的属性，没有提供的属性默认为实体定义文件".def"提供的默认值。例子：
```

params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

def onCreateEntityCallback(entity)
	print(entity)

createEntityAnywhere("Avatar", params, onCreateEntityCallback)

```
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

def onCreateEntityCallback(entity)
	print(entity)

createEntityAnywhere("Avatar", params, onCreateEntityCallback)参数：

entityType
string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

params
可选参数, 一个Python字典对象。
如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。
如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。

callback
callback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是Entity实体的entityCall，失败时是None。

参数：| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| params | 可选参数, 一个Python字典对象。
如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。
如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。 |
| callback | callback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是Entity实体的entityCall，失败时是None。 |

entityTypestring，指定要创建的[Entity](../Classes/Entity.html)Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。params可选参数, 一个Python字典对象。
如果一个指定的键是一个[Entity](../Classes/Entity.html)Entity属性，他的值会用来初始化这个[Entity](../Classes/Entity.html)Entity实体的属性。
如果这个键是一个[Cell](Cell.html)Cell属性，它会被添加到[Entity](../Classes/Entity.html)Entity实体的'[cellData](../Classes/Entity.html#cellData)cellData'属性，这个'[cellData](../Classes/Entity.html#cellData)cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。callbackcallback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall，失败时是None。返回：


通过回调返回Entity实体的entityCall。

返回：| 通过回调返回Entity实体的entityCall。 |

通过回调返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。def createEntityRemotely( entityType, baseMB, params, callback ):

def createEntityRemotely(entityType, baseMB, params, callback):功能说明：通过baseMB参数在一个指定的baseapp上创建一个新的[Entity](../Classes/Entity.html)Entity实体。应该将[KBEngine](KBEngine.html)KBEngine.[createEntityAnywhere](KBEngine.html#createEntityLocally)createEntityAnywhere方法作为首选。函数参数需要提供创建的实体的类型，还有一个Python字典作为参数来初始化实体的值。这个Python字典不需要用户提供所有的属性，没有提供的属性默认为实体定义文件".def"提供的默认值。例子：
```

params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

def onCreateEntityCallback(entity)
	print(entity)

createEntityRemotely("Avatar", baseEntityCall, params, onCreateEntityCallback)

```
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

def onCreateEntityCallback(entity)
	print(entity)

createEntityRemotely("Avatar", baseEntityCall, params, onCreateEntityCallback)参数：

entityType
string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

baseMB
BaseEntityCall，这是一个Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。

params
可选参数, 一个Python字典对象。
如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。
如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。

callback
callback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是Entity实体的entityCall，失败时是None。

参数：| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| baseMB | BaseEntityCall，这是一个Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。 |
| params | 可选参数, 一个Python字典对象。
如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。
如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。 |
| callback | callback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是Entity实体的entityCall，失败时是None。 |

entityTypestring，指定要创建的[Entity](../Classes/Entity.html)Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。baseMBBaseEntityCall，这是一个[Entity](../Classes/Entity.html)Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。params可选参数, 一个Python字典对象。
如果一个指定的键是一个[Entity](../Classes/Entity.html)Entity属性，他的值会用来初始化这个[Entity](../Classes/Entity.html)Entity实体的属性。
如果这个键是一个[Cell](Cell.html)Cell属性，它会被添加到[Entity](../Classes/Entity.html)Entity实体的'[cellData](../Classes/Entity.html#cellData)cellData'属性，这个'[cellData](../Classes/Entity.html#cellData)cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。callbackcallback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall，失败时是None。返回：


通过回调返回Entity实体的entityCall。

返回：| 通过回调返回Entity实体的entityCall。 |

通过回调返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。def createEntityFromDBID( entityType, dbID, callback, dbInterfaceName ):

def createEntityFromDBID(entityType, dbID, callback, dbInterfaceName):功能说明：从数据库里加载数据创建一个[Entity](../Classes/Entity.html)Entity实体。
这个新的[Entity](../Classes/Entity.html)Entity实体会在调用这个函数的[Baseapp](../index.html)Baseapp上创建。
如果该实体已经从数据库检出，那么将返回这个存在的[Entity](../Classes/Entity.html)Entity实体的引用。参数：

entityType
string，指定要加载的Entity实体类型。实体类型在/scripts/entities.xml列出。

dbID
指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。

callback
这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活
wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。
失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| entityType | string，指定要加载的Entity实体类型。实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | 这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活
wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。
失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

entityTypestring，指定要加载的[Entity](../Classes/Entity.html)Entity实体类型。实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。dbID指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的[databaseID](../Classes/Entity.html#databaseID)databaseID属性。callback这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个[entityCall](../../keywords.html#EntityCall)entityCall或者是新创建的[Entity](../Classes/Entity.html)Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。def createEntityAnywhereFromDBID( entityType, dbID, callback, dbInterfaceName ):

def createEntityAnywhereFromDBID(entityType, dbID, callback, dbInterfaceName):功能说明：从数据库里加载数据创建一个[Entity](../Classes/Entity.html)Entity实体。
服务端可能选择任何的[Baseapp](../index.html)Baseapp来创建[Entity](../Classes/Entity.html)Entity实体。使用这个函数将有助于BaseApps负载平衡。如果该实体已经从数据库检出，那么将返回这个存在的[Entity](../Classes/Entity.html)Entity实体的引用。参数：

entityType
string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

dbID
这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。

callback
这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活
wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。
失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | 这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活
wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。
失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

entityTypestring，指定要创建的Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。dbID这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的[databaseID](../Classes/Entity.html#databaseID)databaseID属性。callback这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个[entityCall](../../keywords.html#EntityCall)entityCall或者是新创建的[Entity](../Classes/Entity.html)Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。返回：


通过回调返回Entity实体的entityCall。

返回：| 通过回调返回Entity实体的entityCall。 |

通过回调返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。def createEntityRemotelyFromDBID( entityType, dbID, baseMB, callback, dbInterfaceName ):

def createEntityRemotelyFromDBID(entityType, dbID, baseMB, callback, dbInterfaceName):功能说明：从数据库里加载数据并通过baseMB参数在一个指定的baseapp上创建一个[Entity](../Classes/Entity.html)Entity实体。如果该实体已经从数据库检出，那么将返回这个存在的[Entity](../Classes/Entity.html)Entity实体的引用。参数：

entityType
string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

dbID
这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。

baseMB
BaseEntityCall，这是一个Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。

callback
这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活
wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。
失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| baseMB | BaseEntityCall，这是一个Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。 |
| callback | 这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活
wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。
失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

entityTypestring，指定要创建的Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。dbID这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的[databaseID](../Classes/Entity.html#databaseID)databaseID属性。baseMBBaseEntityCall，这是一个[Entity](../Classes/Entity.html)Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。callback这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。
如果操作成功，baseRef会是一个[entityCall](../../keywords.html#EntityCall)entityCall或者是新创建的[Entity](../Classes/Entity.html)Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。返回：


通过回调返回Entity实体的entityCall。

返回：| 通过回调返回Entity实体的entityCall。 |

通过回调返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。def createEntityLocally( entityType, params ):

def createEntityLocally(entityType, params):功能说明：创建一个新的[Entity](../Classes/Entity.html)Entity实体。函数参数需要提供创建的实体的类型，还有一个Python字典作为参数来初始化实体的值。这个Python字典不需要用户提供所有的属性，没有提供的属性默认为实体定义文件".def"提供的默认值。[KBEngine](KBEngine.html)KBEngine.[createEntityAnywhere](KBEngine.html#createEntityAnywhere)createEntityAnywhere应该作为这个方法的首选，因为服务端可以灵活地
在合适的[Baseapp](../index.html)Baseapp上创建实体。例子：
```

params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

baseEntity = createEntityLocally("Avatar", params)

```
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

baseEntity = createEntityLocally("Avatar", params)参数：

entityType
string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

params
可选参数, 一个Python字典对象。
如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。
如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。

参数：| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| params | 可选参数, 一个Python字典对象。
如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。
如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。 |

entityTypestring，指定要创建的Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。params可选参数, 一个Python字典对象。
如果一个指定的键是一个[Entity](../Classes/Entity.html)Entity属性，他的值会用来初始化这个[Entity](../Classes/Entity.html)Entity实体的属性。
如果这个键是一个[Cell](Cell.html)Cell属性，它会被添加到[Entity](../Classes/Entity.html)Entity实体的'[cellData](../Classes/Entity.html#cellData)cellData'属性，这个'[cellData](../Classes/Entity.html#cellData)cellData'属性是一个Python字典，
然后在后面会用来初始化cell实体的属性。返回：

新创建的Entity实体（参考Entity）

返回：| 新创建的Entity实体（参考Entity） |

新创建的Entity实体（参考[Entity](../Classes/Entity.html)Entity）def debugTracing(  ):

def debugTracing():功能说明：输出当前KBEngine追踪的Python扩展对象计数器。扩展对象包括：固定字典、固定数组、Entity、EntityCall...在服务端正常关闭时如果计数器不为零，此时说明泄露已存在，日志将会输出错误信息。ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): FixedArray : leaked(128)ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): EntityCall : leaked(8)参数：

path
要删除的变量的路径。

参数：| path | 要删除的变量的路径。 |

path要删除的变量的路径。def delWatcher( path ):

def delWatcher(path):功能说明：与调试监视系统交互，允许用户在脚本删除监视的变量。参数：

path
要删除的变量的路径。

参数：| path | 要删除的变量的路径。 |

path要删除的变量的路径。def deleteEntityByDBID( entityType, dbID, callback, dbInterfaceName ):

def deleteEntityByDBID(entityType, dbID, callback, dbInterfaceName):功能说明：从数据库删除指定的实体（包括属性所产生的子表数据），如果实体没有从数据库检出则删除成功，
如果实体已经从数据库检出那么[KBEngine](KBEngine.html)KBEngine服务系统将会删除失败，并且从回调中返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。参数：

entityType
string，指定要删除的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

dbID
指定要删除的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。

callback
callback是一个可选的回调函数，只有一个参数，当实体没有从数据库检出时将会成功删除数据，参数是True。如果实体已经从数据库检出那么参数是Entity实体的entityCall。

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| entityType | string，指定要删除的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要删除的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | callback是一个可选的回调函数，只有一个参数，当实体没有从数据库检出时将会成功删除数据，参数是True。如果实体已经从数据库检出那么参数是Entity实体的entityCall。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

entityTypestring，指定要删除的Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。dbID指定要删除的实体的数据库ID。这个实体的数据库ID存储在该实体的[databaseID](../Classes/Entity.html#databaseID)databaseID属性。callbackcallback是一个可选的回调函数，只有一个参数，当实体没有从数据库检出时将会成功删除数据，参数是True。如果实体已经从数据库检出那么参数是[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。def deregisterReadFileDescriptor( fileDescriptor ):

def deregisterReadFileDescriptor(fileDescriptor):功能说明：注销已经通过[KBEngine](KBEngine.html)KBEngine.[registerReadFileDescriptor](KBEngine.html#registerReadFileDescriptor)registerReadFileDescriptor注册的回调。例子:[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)http://www.kbengine.org/assets/other/py/Poller.py参数：

fileDescriptor
socket描述符/文件描述符。

参数：| fileDescriptor | socket描述符/文件描述符。 |

fileDescriptorsocket描述符/文件描述符。def deregisterWriteFileDescriptor( fileDescriptor ):

def deregisterWriteFileDescriptor(fileDescriptor):功能说明：注销已经通过[KBEngine](KBEngine.html)KBEngine.[registerWriteFileDescriptor](KBEngine.html#registerWriteFileDescriptor)registerWriteFileDescriptor注册的回调。例子:[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)http://www.kbengine.org/assets/other/py/Poller.py参数：

fileDescriptor
socket描述符/文件描述符。

参数：| fileDescriptor | socket描述符/文件描述符。 |

fileDescriptorsocket描述符/文件描述符。def executeRawDatabaseCommand( command, callback, threadID, dbInterfaceName ):

def executeRawDatabaseCommand(command, callback, threadID, dbInterfaceName):功能说明：这个脚本函数在数据库上执行原始数据库命令，该命令将直接由相关数据库进行解析。请注意使用该函数修改实体数据可能不生效，因为如果实体已经检出，被修改过的实体数据将仍会被实体存档而导致覆盖。强烈不推荐这个函数用于读取或修改实体数据。参数：

command
这个数据库命令将会因为不同数据库配置方案而不同。对于方案为MySQL数据库它是一个SQL查询语句。

callback

  可选参数，带有命令执行结果的回调对象（比如说是一个函数）。这个回调带有4个参数：结果集合，影响的行数，自増长值，错误信息。
  
  声明样例：
  def 
  sqlcallback(result, rows, insertid, error):
    print(result, rows, insertid, error)  

    如同上面的例子所示，result参数对应的就是“结果集合”，这个结果集合参数是一个行列表。
    每一行是一个包含字段值的字符串列表。
    命令执行没有返回结果集合（比如说是DELETE命令），
    或者
    命令执行有错误时这个结果集合为None。
    
    rows参数则是“影响的行数”，它是一个整数，表示命令执行受影响的行数。这个参数只和不返回结果结合的命令（如DELETE）相关。
    如果有结果集合返回或者命令执行有错误时这个参数为None。
    
insertid对应的是“自増长值”，类似于实体的databaseID，当成功的向一张带有自増长类型字段的表中插入数据时，它返回该数据在插入时自増长字段所被赋于的值。
    更多的信息可以参阅mysql的mysql_insert_id()方法。另外，此参数仅在数据库类型为mysql时有意义。

    error则对应了“错误信息”，当命令执行有错误时，这个参数是一个描述错误的字符串。命令执行没有发生错误时这个参数为None。
  
threadID
int32，可选参数，指定一个线程来处理本条命令。用户可以通过这个参数控制某一类命令的执行先后顺序（dbmgr是多线程处理的），默认是不指定，如果threadID是实体的ID，
那么将加入到该实体的存档队列中由线程逐条写入。

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| command | 这个数据库命令将会因为不同数据库配置方案而不同。对于方案为MySQL数据库它是一个SQL查询语句。 |
| --- | --- |
| callback | 可选参数，带有命令执行结果的回调对象（比如说是一个函数）。这个回调带有4个参数：结果集合，影响的行数，自増长值，错误信息。
  
  声明样例：
  def 
  sqlcallback(result, rows, insertid, error):
    print(result, rows, insertid, error)  

    如同上面的例子所示，result参数对应的就是“结果集合”，这个结果集合参数是一个行列表。
    每一行是一个包含字段值的字符串列表。
    命令执行没有返回结果集合（比如说是DELETE命令），
    或者
    命令执行有错误时这个结果集合为None。
    
    rows参数则是“影响的行数”，它是一个整数，表示命令执行受影响的行数。这个参数只和不返回结果结合的命令（如DELETE）相关。
    如果有结果集合返回或者命令执行有错误时这个参数为None。
    
insertid对应的是“自増长值”，类似于实体的databaseID，当成功的向一张带有自増长类型字段的表中插入数据时，它返回该数据在插入时自増长字段所被赋于的值。
    更多的信息可以参阅mysql的mysql_insert_id()方法。另外，此参数仅在数据库类型为mysql时有意义。

    error则对应了“错误信息”，当命令执行有错误时，这个参数是一个描述错误的字符串。命令执行没有发生错误时这个参数为None。 |
| threadID | int32，可选参数，指定一个线程来处理本条命令。用户可以通过这个参数控制某一类命令的执行先后顺序（dbmgr是多线程处理的），默认是不指定，如果threadID是实体的ID，
那么将加入到该实体的存档队列中由线程逐条写入。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

command这个数据库命令将会因为不同数据库配置方案而不同。对于方案为MySQL数据库它是一个SQL查询语句。callback可选参数，带有命令执行结果的回调对象（比如说是一个函数）。这个回调带有4个参数：结果集合，影响的行数，自増长值，错误信息。
  
  声明样例：
  def 
  sqlcallback(result, rows, insertid, error):
    print(result, rows, insertid, error)  

    如同上面的例子所示，result参数对应的就是“结果集合”，这个结果集合参数是一个行列表。
    每一行是一个包含字段值的字符串列表。
    命令执行没有返回结果集合（比如说是DELETE命令），
    或者
    命令执行有错误时这个结果集合为None。
    
    rows参数则是“影响的行数”，它是一个整数，表示命令执行受影响的行数。这个参数只和不返回结果结合的命令（如DELETE）相关。
    如果有结果集合返回或者命令执行有错误时这个参数为None。

可选参数，带有命令执行结果的回调对象（比如说是一个函数）。这个回调带有4个参数：结果集合，影响的行数，自増长值，错误信息。声明样例：def 
  sqlcallback(result, rows, insertid, error):print(result, rows, insertid, error)如同上面的例子所示，result参数对应的就是“结果集合”，这个结果集合参数是一个行列表。
    每一行是一个包含字段值的字符串列表。命令执行没有返回结果集合（比如说是DELETE命令），
    或者
    命令执行有错误时这个结果集合为None。rows参数则是“影响的行数”，它是一个整数，表示命令执行受影响的行数。这个参数只和不返回结果结合的命令（如DELETE）相关。如果有结果集合返回或者命令执行有错误时这个参数为None。insertid对应的是“自増长值”，类似于实体的databaseID，当成功的向一张带有自増长类型字段的表中插入数据时，它返回该数据在插入时自増长字段所被赋于的值。
    更多的信息可以参阅mysql的mysql_insert_id()方法。另外，此参数仅在数据库类型为mysql时有意义。

    error则对应了“错误信息”，当命令执行有错误时，这个参数是一个描述错误的字符串。命令执行没有发生错误时这个参数为None。

insertid对应的是“自増长值”，类似于实体的databaseID，当成功的向一张带有自増长类型字段的表中插入数据时，它返回该数据在插入时自増长字段所被赋于的值。更多的信息可以参阅mysql的mysql_insert_id()方法。另外，此参数仅在数据库类型为mysql时有意义。error则对应了“错误信息”，当命令执行有错误时，这个参数是一个描述错误的字符串。命令执行没有发生错误时这个参数为None。threadIDint32，可选参数，指定一个线程来处理本条命令。用户可以通过这个参数控制某一类命令的执行先后顺序（dbmgr是多线程处理的），默认是不指定，如果threadID是实体的ID，那么将加入到该实体的存档队列中由线程逐条写入。dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。def genUUID64(  ):

def genUUID64():功能说明：该函数生成一个64位的唯一ID。注意：这个函数依赖于Baseapps服务进程启动参数gus，请正确设置启动参数保持唯一性。另外如果gus超过65535则该函数只能在当前进程上保持唯一性。用途：多个服务进程上产生唯一物品ID并且在合服时不会产生冲突。多个服务进程上产生一个房间ID，不需要进行唯一性校验。返回：


返回一个64位的integer。

返回：| 返回一个64位的integer。 |

返回一个64位的integer。def getResFullPath( res ):

def getResFullPath(res):功能说明：获取资源的绝对路径。注意：资源必须在[KBE_RES_PATH](../../keywords.html#KBE_RES_PATH)KBE_RES_PATH之下才可以访问到。参数：

res
string，资源的相对路径。

参数：| res | string，资源的相对路径。 |

resstring，资源的相对路径。返回：


string，如果存在返回资源的绝对路径，否则返回空。

返回：| string，如果存在返回资源的绝对路径，否则返回空。 |

string，如果存在返回资源的绝对路径，否则返回空。def getWatcher( path ):

def getWatcher(path):功能说明：从KBEngine调试系统中获取一个监视变量的值。例子：在baseapp1的Python命令行输入:>>>KBEngine.getWatcher("/root/stats/runningTime")12673648533>>>KBEngine.getWatcher("/root/scripts/players")32133参数：

path
string，该变量的绝对路径包括变量名(可以在GUIConsole的watcher页查看)。

参数：| path | string，该变量的绝对路径包括变量名(可以在GUIConsole的watcher页查看)。 |

pathstring，该变量的绝对路径包括变量名(可以在GUIConsole的watcher页查看)。返回：


该变量的值。

返回：| 该变量的值。 |

该变量的值。def getWatcherDir( path ):

def getWatcherDir(path):功能说明：从KBEngine调试系统中获取一个监视目录下的元素列表(目录、变量名)。例子：在baseapp1的Python命令行输入:>>>KBEngine.getWatcher("/root")('stats', 'objectPools', 'network', 'syspaths', 'ThreadPool', 'cprofiles', 'scripts', 'numProxices', 'componentID', 'componentType', 'uid', 'numClients', 'globalOrder', 'username', 'load', 'gametime', 'entitiesSize', 'groupOrder')参数：

path
string，该变量的绝对路径(可以在GUIConsole的watcher页查看)。

参数：| path | string，该变量的绝对路径(可以在GUIConsole的watcher页查看)。 |

pathstring，该变量的绝对路径(可以在GUIConsole的watcher页查看)。返回：


监视目录下的元素列表(目录、变量名)。

返回：| 监视目录下的元素列表(目录、变量名)。 |

监视目录下的元素列表(目录、变量名)。def getAppFlags(  ):

def getAppFlags():功能说明：获取当前引擎APP的标记, 参考:[KBEngine](KBEngine.html)KBEngine.[setAppFlags](KBEngine.html#setAppFlags)setAppFlags。返回：


KBEngine.APP_FLAGS_*。

返回：| KBEngine.APP_FLAGS_*。 |

KBEngine.APP_FLAGS_*。def hasRes( res ):

def hasRes(res):功能说明：使用这个接口可以判断一个相对路径的资源是否存在。注意：资源必须在[KBE_RES_PATH](../../keywords.html#KBE_RES_PATH)KBE_RES_PATH之下才可以访问到。例子:>>>KBEngine.hasRes("scripts/entities.xml")True参数：

res
string，资源的相对路径。

参数：| res | string，资源的相对路径。 |

resstring，资源的相对路径。返回：


BOOL, 存在返回True，否则返回False。

返回：| BOOL, 存在返回True，否则返回False。 |

BOOL, 存在返回True，否则返回False。def isShuttingDown(  ):

def isShuttingDown():功能说明：返回服务端是否正在关闭中。在[onBaseAppShutDown(state=0)](KBEngine.html#onBaseAppShutDown)onBaseAppShutDown(state=0)回调函数被调用后，这个函数返回True。返回：


系统正在关闭返回True，否则返回False。

返回：| 系统正在关闭返回True，否则返回False。 |

系统正在关闭返回True，否则返回False。def listPathRes( path, extension ):

def listPathRes(path, extension):功能说明：获得一个资源目录下的资源列表。注意：资源必须在[KBE_RES_PATH](../../keywords.html#KBE_RES_PATH)KBE_RES_PATH之下才可以访问到。例子:>>>KBEngine.listPathRes("scripts/cell/interfaces")('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')>>>KBEngine.listPathRes("scripts/cell/interfaces", "txt")('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')>>>KBEngine.listPathRes("scripts/cell/interfaces", "txt|py")('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')>>>KBEngine.listPathRes("scripts/cell/interfaces", ("txt", "py"))('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')参数：

res
string，资源的相对路径。

extension
string，可选参数，扩展名。

参数：| res | string，资源的相对路径。 |
| --- | --- |
| extension | string，可选参数，扩展名。 |

resstring，资源的相对路径。extensionstring，可选参数，扩展名。返回：


Tuple, 资源列表。

返回：| Tuple, 资源列表。 |

Tuple, 资源列表。def lookUpEntityByDBID( entityType, dbID, callback, dbInterfaceName ):

def lookUpEntityByDBID(entityType, dbID, callback, dbInterfaceName):功能说明：查询一个实体是否从数据库检出，
如果实体已经从数据库检出那么[KBEngine](KBEngine.html)KBEngine服务系统将从回调中返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall。参数：

entityType
string，指定要查询的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

dbID
指定要查询的Entity实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。

callback
callback只有一个参数，当实体没有从数据库检出时将会返回True。如果实体已经从数据库检出那么将返回Entity实体的entityCall, 其他任何情况返回False。

dbInterfaceName
string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。

参数：| entityType | string，指定要查询的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要查询的Entity实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | callback只有一个参数，当实体没有从数据库检出时将会返回True。如果实体已经从数据库检出那么将返回Entity实体的entityCall, 其他任何情况返回False。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

entityTypestring，指定要查询的Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。dbID指定要查询的[Entity](../Classes/Entity.html)Entity实体的数据库ID。这个实体的数据库ID存储在该实体的[databaseID](../Classes/Entity.html#databaseID)databaseID属性。callbackcallback只有一个参数，当实体没有从数据库检出时将会返回True。如果实体已经从数据库检出那么将返回[Entity](../Classes/Entity.html)Entity实体的[entityCall](../../keywords.html#EntityCall)entityCall, 其他任何情况返回False。dbInterfaceNamestring，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。def matchPath( res ):

def matchPath(res):功能说明：使用相对路径的资源获得资源的绝对路径。注意：资源必须在[KBE_RES_PATH](../../keywords.html#KBE_RES_PATH)KBE_RES_PATH之下才可以访问到。例子:>>>KBEngine.matchPath("scripts/entities.xml")'/home/kbe/kbengine/demo/res/scripts/[entities.xml](../../keywords.html#entities.xml)entities.xml'参数：

res
string，资源的相对路径(包括资源名称)。

参数：| res | string，资源的相对路径(包括资源名称)。 |

resstring，资源的相对路径(包括资源名称)。返回：


string, 资源的绝对路径。

返回：| string, 资源的绝对路径。 |

string, 资源的绝对路径。def open( res, mode, encoding ):

def open(res, mode, encoding):功能说明：使用这个接口可以使用相对路径来打开相关资源。注意：资源必须在[KBE_RES_PATH](../../keywords.html#KBE_RES_PATH)KBE_RES_PATH之下才可以访问到。参数：

res
string，资源的相对路径。

mode
string，可选参数，默认值是'r'，文件操作模式：
r     以只读方式打开，
w     以写方式打开，
a     以追加模式打开 (从 EOF 开始, 必要时创建新文件)
r+     以读写模式打开
w+     以读写模式打开 (参见 w )
a+     以读写模式打开 (参见 a )
rb     以二进制读模式打开
wb     以二进制写模式打开 (参见 w )
ab     以二进制追加模式打开 (参见 a )
rb+    以二进制读写模式打开 (参见 r+ )
wb+    以二进制读写模式打开 (参见 w+ )
ab+    以二进制读写模式打开 (参见 a+ )

encoding
string，可选参数，用于解码或编码文件的编码的名称，默认编码与平台相关。

参数：| res | string，资源的相对路径。 |
| --- | --- |
| mode | string，可选参数，默认值是'r'，文件操作模式：
r     以只读方式打开，
w     以写方式打开，
a     以追加模式打开 (从 EOF 开始, 必要时创建新文件)
r+     以读写模式打开
w+     以读写模式打开 (参见 w )
a+     以读写模式打开 (参见 a )
rb     以二进制读模式打开
wb     以二进制写模式打开 (参见 w )
ab     以二进制追加模式打开 (参见 a )
rb+    以二进制读写模式打开 (参见 r+ )
wb+    以二进制读写模式打开 (参见 w+ )
ab+    以二进制读写模式打开 (参见 a+ ) |
| encoding | string，可选参数，用于解码或编码文件的编码的名称，默认编码与平台相关。 |

resstring，资源的相对路径。modestring，可选参数，默认值是'r'，文件操作模式：r     以只读方式打开，w     以写方式打开，a     以追加模式打开 (从 EOF 开始, 必要时创建新文件)r+     以读写模式打开w+     以读写模式打开 (参见 w )a+     以读写模式打开 (参见 a )rb     以二进制读模式打开wb     以二进制写模式打开 (参见 w )ab     以二进制追加模式打开 (参见 a )rb+    以二进制读写模式打开 (参见 r+ )wb+    以二进制读写模式打开 (参见 w+ )ab+    以二进制读写模式打开 (参见 a+ )encodingstring，可选参数，用于解码或编码文件的编码的名称，默认编码与平台相关。def publish(  ):

def publish():功能说明：这个接口返回当前服务端发行模式。返回：


int8，0：debug，1：release，其它可自定义。

返回：| int8，0：debug，1：release，其它可自定义。 |

int8，0：debug，1：release，其它可自定义。def quantumPassedPercent(  ):

def quantumPassedPercent():功能说明：返回取得当前tick占用一个时钟周期的百分比。返回：


返回取得当前tick占用一个时钟周期的百分比。

返回：| 返回取得当前tick占用一个时钟周期的百分比。 |

返回取得当前tick占用一个时钟周期的百分比。def registerReadFileDescriptor( fileDescriptor, callback ):

def registerReadFileDescriptor(fileDescriptor, callback):功能说明：注册一个回调函数，这个回调函数当文件描述符可读时被调用。例子:[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)http://www.kbengine.org/assets/other/py/Poller.py参数：

fileDescriptor
socket描述符/文件描述符。

callback
一个回调函数，socket描述符/文件描述符作为它的唯一参数。

参数：| fileDescriptor | socket描述符/文件描述符。 |
| --- | --- |
| callback | 一个回调函数，socket描述符/文件描述符作为它的唯一参数。 |

fileDescriptorsocket描述符/文件描述符。callback一个回调函数，socket描述符/文件描述符作为它的唯一参数。def registerWriteFileDescriptor( fileDescriptor, callback ):

def registerWriteFileDescriptor(fileDescriptor, callback):功能说明：注册一个回调函数，这个回调函数当socket描述符/文件描述符可写时被调用。例子:[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)http://www.kbengine.org/assets/other/py/Poller.py参数：

fileDescriptor
socket描述符/文件描述符。

callback
一个回调函数，socket描述符/文件描述符作为它的唯一参数。

参数：| fileDescriptor | socket描述符/文件描述符。 |
| --- | --- |
| callback | 一个回调函数，socket描述符/文件描述符作为它的唯一参数。 |

fileDescriptorsocket描述符/文件描述符。callback一个回调函数，socket描述符/文件描述符作为它的唯一参数。def reloadScript( fullReload ):

def reloadScript(fullReload):功能说明：重新加载与实体和自定义数据类型相关的Python模块。当前实体类会设置为新加载的类。
这个方法应该只用于开发模式，对于产品模式不合适。下面几点应该注意：1）重载脚本仅仅能在[Baseapp](../index.html)Baseapp上执行， 用户应该确保所有的服务端组件加载完成。2）自定义类型在脚本重载后应该确保内存中已经实例化的对象也被更新，下面是一个例子:
```
for e in KBEngine.entities.values():
   if type( e ) is Avatar.Avatar:
      e.customData.__class__ = CustomClass

```
for e in[KBEngine](KBEngine.html)KBEngine.[entities](KBEngine.html#entities)entities.values():
   if type( e ) is Avatar.Avatar:
      e.customData.__class__ = CustomClass当这个方法完成时[KBEngine](KBEngine.html)KBEngine.[onInit](KBEngine.html#onInit)onInit( True ) 被调用。参数：

fullReload
可选的boolean参数，指定是否同时重新加载实体定义。如果这个参数为False，则实体定义不会被重新加载。默认为True。

参数：| fullReload | 可选的boolean参数，指定是否同时重新加载实体定义。如果这个参数为False，则实体定义不会被重新加载。默认为True。 |

fullReload可选的boolean参数，指定是否同时重新加载实体定义。如果这个参数为False，则实体定义不会被重新加载。默认为True。返回：

重新加载成功返回True，否则返回False。

返回：| 重新加载成功返回True，否则返回False。 |

重新加载成功返回True，否则返回False。def scriptLogType( logType ):

def scriptLogType(logType):功能说明：设置当前Python.print输出的信息类型(参考: KBEngine.LOG_TYPE_*)。def setAppFlags( flags ):

def setAppFlags(flags):功能说明：设置当前引擎APP的标记。KBEngine.APP_FLAGS_NONE // 默认的(未设置标记)KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING //不参与负载均衡例如：KBEngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING | KBEngine.APP_FLAGS_*)def time(  ):

def time():功能说明：这个方法返回当前游戏的时间（周期数）。返回：


uint32，当前游戏的时间，这里指周期数，周期受频率影响，频率由配置文件kbengine.xml或者kbengine_defaults.xml->gameUpdateHertz决定。

返回：| uint32，当前游戏的时间，这里指周期数，周期受频率影响，频率由配置文件kbengine.xml或者kbengine_defaults.xml->gameUpdateHertz决定。 |

uint32，当前游戏的时间，这里指周期数，周期受频率影响，频率由配置文件[kbengine.xml](../../keywords.html#kbengine.xml)kbengine.xml或者[kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->gameUpdateHertz决定。def urlopen( url, callback, postData, headers ):

def urlopen(url, callback, postData, headers):功能说明：这个脚本函数在提供对外HTTP/HTTPS异步请求。参数：

url
有效的HTTP/HTTPS网址，字符串类型。

callback

可选参数，带有请求执行结果的回调对象（比如说是一个函数）。这个回调带有5个参数：HTTP请求返回码（如：200)，返回的内容，返回的HTTP协议头，是否成功，请求的网址。

声明样例：
def 
onHttpCallback(httpcode, data, headers, success, url):
    print(httpcode, data, headers, success, url)  

如同上面的例子所示:
httpcode:参数对应的就是“HTTP请求返回码”，这个结果集合参数是一个整形值。
data:参数则是“返回的内容”，它是一个字符串。

headers:参数是“服务器返回的HTTP协议头”，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。
success:则对应了“执行是否成功”，当请求执行有错误时，为False，可以通过httpcode进一步判断错误信息。

url:是“请求所用的网址。


postData
可选参数，默认是GET方式请求服务器，如果需要POST方式请提供需要POST的内容，引擎将自动使用POST方式请求服务器，它是一个bytes。

headers
可选参数，请求时使用的HTTP头，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。

参数：| url | 有效的HTTP/HTTPS网址，字符串类型。 |
| --- | --- |
| callback | 可选参数，带有请求执行结果的回调对象（比如说是一个函数）。这个回调带有5个参数：HTTP请求返回码（如：200)，返回的内容，返回的HTTP协议头，是否成功，请求的网址。

声明样例：
def 
onHttpCallback(httpcode, data, headers, success, url):
    print(httpcode, data, headers, success, url)  

如同上面的例子所示:
httpcode:参数对应的就是“HTTP请求返回码”，这个结果集合参数是一个整形值。
data:参数则是“返回的内容”，它是一个字符串。

headers:参数是“服务器返回的HTTP协议头”，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。
success:则对应了“执行是否成功”，当请求执行有错误时，为False，可以通过httpcode进一步判断错误信息。

url:是“请求所用的网址。 |
| postData | 可选参数，默认是GET方式请求服务器，如果需要POST方式请提供需要POST的内容，引擎将自动使用POST方式请求服务器，它是一个bytes。 |
| headers | 可选参数，请求时使用的HTTP头，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。 |

url有效的HTTP/HTTPS网址，字符串类型。callback可选参数，带有请求执行结果的回调对象（比如说是一个函数）。这个回调带有5个参数：HTTP请求返回码（如：200)，返回的内容，返回的HTTP协议头，是否成功，请求的网址。

声明样例：
def 
onHttpCallback(httpcode, data, headers, success, url):
    print(httpcode, data, headers, success, url)  

如同上面的例子所示:
httpcode:参数对应的就是“HTTP请求返回码”，这个结果集合参数是一个整形值。
data:参数则是“返回的内容”，它是一个字符串。

可选参数，带有请求执行结果的回调对象（比如说是一个函数）。这个回调带有5个参数：HTTP请求返回码（如：200)，返回的内容，返回的HTTP协议头，是否成功，请求的网址。声明样例：def 
onHttpCallback(httpcode, data, headers, success, url):print(httpcode, data, headers, success, url)如同上面的例子所示:httpcode:参数对应的就是“HTTP请求返回码”，这个结果集合参数是一个整形值。data:参数则是“返回的内容”，它是一个字符串。headers:参数是“服务器返回的HTTP协议头”，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。
success:则对应了“执行是否成功”，当请求执行有错误时，为False，可以通过httpcode进一步判断错误信息。

headers:参数是“服务器返回的HTTP协议头”，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。success:则对应了“执行是否成功”，当请求执行有错误时，为False，可以通过httpcode进一步判断错误信息。url:是“请求所用的网址。

url:是“请求所用的网址。postData可选参数，默认是GET方式请求服务器，如果需要POST方式请提供需要POST的内容，引擎将自动使用POST方式请求服务器，它是一个bytes。headers可选参数，请求时使用的HTTP头，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。
---

## 回调函数文档
回调函数文档def onBaseAppReady( isBootstrap ):

def onBaseAppReady(isBootstrap):功能说明：当前[Baseapp](../index.html)Baseapp进程已经准备好的时候回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

isBootstrap
bool，是否为第一个启动的Baseapp。

参数：| isBootstrap | bool，是否为第一个启动的Baseapp。 |

isBootstrapbool，是否为第一个启动的[Baseapp](../index.html)Baseapp。def onBaseAppShutDown( state ):

def onBaseAppShutDown(state):功能说明：[Baseapp](../index.html)Baseapp关闭过程会回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

state
如果state为0，意指在断开所有客户端之前，如果state为1，意指在将所有实体写入数据库之前，如果state为2，意指在所有实体被写入数据库之后。

参数：| state | 如果state为0，意指在断开所有客户端之前，如果state为1，意指在将所有实体写入数据库之前，如果state为2，意指在所有实体被写入数据库之后。 |

state如果state为0，意指在断开所有客户端之前，如果state为1，意指在将所有实体写入数据库之前，如果state为2，意指在所有实体被写入数据库之后。def onCellAppDeath( addr ):

def onCellAppDeath(addr):功能说明：某个cellapp死亡会回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

addr
死亡的cellapp地址。
tuple:(ip, port) 网络字节序

参数：| addr | 死亡的cellapp地址。
tuple:(ip, port) 网络字节序 |

addr死亡的cellapp地址。tuple:(ip, port) 网络字节序def onFini(  ):

def onFini():功能说明：引擎正式关闭后回调此函数。注意：该回调接口必须实现在入口模块[kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。def onBaseAppData( key, value ):

def onBaseAppData(key, value):功能说明：KBEngine.baseAppData有改变时回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

key
被改变数据的键。

value
被改变数据的值。

参数：| key | 被改变数据的键。 |
| --- | --- |
| value | 被改变数据的值。 |

key被改变数据的键。value被改变数据的值。def onBaseAppDataDel( key ):

def onBaseAppDataDel(key):功能说明：KBEngine.baseAppData有删除的时候回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

key
被删除数据的键。

参数：| key | 被删除数据的键。 |

key被删除数据的键。def onGlobalData( key, value ):

def onGlobalData(key, value):功能说明：KBEngine.globalData有改变的时候回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

key
被改变数据的键。

value
被改变数据的值。

参数：| key | 被改变数据的键。 |
| --- | --- |
| value | 被改变数据的值。 |

key被改变数据的键。value被改变数据的值。def onGlobalDataDel( key ):

def onGlobalDataDel(key):功能说明：KBEngine.globalData有删除的时候回调此函数。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

key
被删除数据的键。

参数：| key | 被删除数据的键。 |

key被删除数据的键。def onInit( isReload ):

def onInit(isReload):功能说明：当引擎启动后初始化完所有的脚本后这个接口被调用。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

isReload
bool，是否是被重写加载脚本后触发的。

参数：| isReload | bool，是否是被重写加载脚本后触发的。 |

isReloadbool，是否是被重写加载脚本后触发的。def onLoseChargeCB( orderID, dbID, success, datas ):

def onLoseChargeCB(orderID, dbID, success, datas):功能说明：当在interfaces中调用KBEngine.chargeResponse后，如果该订单丢失或者是不明interfaces未被记录的订单会收到此回调通知。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

ordersID
string，订单ID。

dbID
uint64，实体的数据库ID, 参见: Entity.databaseID。

success
bool，是否成功。

datas
bytes，附带信息。

参数：| ordersID | string，订单ID。 |
| --- | --- |
| dbID | uint64，实体的数据库ID, 参见: Entity.databaseID。 |
| success | bool，是否成功。 |
| datas | bytes，附带信息。 |

ordersIDstring，订单ID。dbIDuint64，实体的数据库ID, 参见:[Entity.databaseID](../Classes/Entity.html#databaseID)Entity.databaseID。successbool，是否成功。datasbytes，附带信息。def onReadyForLogin( isBootstrap ):

def onReadyForLogin(isBootstrap):功能说明：当引擎启动并初始化完成后会一直调用此接口询问脚本层是否准备完毕，如果脚本层准备完毕则loginapp允许客户端登录。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。参数：

isBootstrap
bool，是否为第一个启动的Baseapp。

参数：| isBootstrap | bool，是否为第一个启动的Baseapp。 |

isBootstrapbool，是否为第一个启动的[Baseapp](../index.html)Baseapp。返回：

返回值大于等于1.0则脚本层准备完成，否则返回准备的进度值0.0~1.0。

返回：| 返回值大于等于1.0则脚本层准备完成，否则返回准备的进度值0.0~1.0。 |

返回值大于等于1.0则脚本层准备完成，否则返回准备的进度值0.0~1.0。def onReadyForShutDown(  ):

def onReadyForShutDown():功能说明：如果这个函数在脚本中有实现，当进程准备退出时，该回调函数被调用。可以通过该回调控制进程退出的时机。注意：该回调接口必须实现在入口模块([kbengine_defaults.xml](../../keywords.html#kbengine_defaults.xml)kbengine_defaults.xml->entryScriptFile)中。返回：


bool，如果返回True，则允许进入进程退出流程，返回其它值则进程会过一段时间后再次询问。

返回：| bool，如果返回True，则允许进入进程退出流程，返回其它值则进程会过一段时间后再次询问。 |

bool，如果返回True，则允许进入进程退出流程，返回其它值则进程会过一段时间后再次询问。def onAutoLoadEntityCreate( entityType, dbID ):

def onAutoLoadEntityCreate(entityType, dbID):功能说明：自动加载的实体创建时的回调，如果脚本层实现此回调，那么实体由脚本层创建，否则引擎默认使用createEntityAnywhereFromDBID来创建实体。这个回调被调用是由于[Entity.writeToDB](../Classes/Entity.html#writeToDB)Entity.writeToDB时设置了实体自动加载。注：该回调优先于onBaseAppReady执行，可在onBaseAppReady时检查是否已加载实体。参数：

entityType
string，指定要查询的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。

dbID
指定要查询的Entity实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。

参数：| entityType | string，指定要查询的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要查询的Entity实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |

entityTypestring，指定要查询的Entity实体的类型。有效的实体类型在[/scripts/entities.xml](../../keywords.html#entities.xml)/scripts/entities.xml列出。dbID指定要查询的[Entity](../Classes/Entity.html)Entity实体的数据库ID。这个实体的数据库ID存储在该实体的[databaseID](../Classes/Entity.html#databaseID)databaseID属性。
---

## 属性文档
属性文档LOG_ON_ACCEPT

LOG_ON_ACCEPT说明：这个常量由[Proxy](../Classes/Proxy.html)Proxy.[onLogOnAttempt](../Classes/Proxy.html#onLogOnAttempt)onLogOnAttempt返回，意指允许新的client与一个[Proxy实体](../Classes/Proxy.html)Proxy实体绑定。如果[Proxy实体](../Classes/Proxy.html)Proxy实体已经存在一个client绑定关系，那么将踢出之前的client。LOG_ON_REJECT

LOG_ON_REJECT说明：这个常量由[Proxy](../Classes/Proxy.html)Proxy.[onLogOnAttempt](../Classes/Proxy.html#onLogOnAttempt)onLogOnAttempt返回，意指拒绝当前client与[Proxy实体](../Classes/Proxy.html)Proxy实体绑定。LOG_ON_WAIT_FOR_DESTROY

LOG_ON_WAIT_FOR_DESTROY说明：这个常量由[Proxy](../Classes/Proxy.html)Proxy.[onLogOnAttempt](../Classes/Proxy.html#onLogOnAttempt)onLogOnAttempt返回，当前请求client将会等待直到[Proxy实体](../Classes/Proxy.html)Proxy实体完全销毁，底层再完成后续绑定过程。
在这返回之前[Proxy](../Classes/Proxy.html)Proxy.[destroy](../Classes/Entity.html#destroy)destroy或者[Proxy](../Classes/Proxy.html)Proxy.[destroyCellEntity](../Classes/Entity.html#destroyCellEntity)destroyCellEntity应该被调用。LOG_TYPE_DBG

LOG_TYPE_DBG说明：日志输出类型为调试类型。由[scriptLogType](#scriptLogType)scriptLogType设置。LOG_TYPE_ERR

LOG_TYPE_ERR说明：日志输出类型为错误类型。由[scriptLogType](#scriptLogType)scriptLogType设置。LOG_TYPE_INFO

LOG_TYPE_INFO说明：日志输出类型为常规信息类型。由[scriptLogType](#scriptLogType)scriptLogType设置。LOG_TYPE_NORMAL

LOG_TYPE_NORMAL说明：日志输出类型为常规类型。由[scriptLogType](#scriptLogType)scriptLogType设置。LOG_TYPE_WAR

LOG_TYPE_WAR说明：日志输出类型为警告类型。由[scriptLogType](#scriptLogType)scriptLogType设置。NEXT_ONLY

NEXT_ONLY说明：这个常量用于[Entity.shouldAutoBackup](../Classes/Entity.html#shouldAutoBackup)Entity.shouldAutoBackup和[Entity.shouldAutoArchive](../Classes/Entity.html#shouldAutoArchive)Entity.shouldAutoArchive属性。这个值意指在下一次认为可以的时候自动备份该实体，然后这个属性自动设为False（0）。component

component说明：这是正运行在当前Python环境的组件。（至今为止）可能值有'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' 和 'editor'。entities

entities说明：entities是一个字典对象，包含当前进程上所有的实体。调试泄露的实体（调用过destroy却没有释放内存的实体，通常是由于被引用导致无法释放）：
```

>>> KBEngine.entities.garbages.items()
[(1025, Avatar object at 0x7f92431ceae8.)]

>>> e = _[0][1]
>>> import gc
>>> gc.get_referents(e)
[{'spacesIsOk': True, 'bootstrapIdx': 1}, ]

```
>>> KBEngine.entities.garbages.items()
[(1025, Avatar object at 0x7f92431ceae8.)]>>> e = _[0][1]
>>> import gc
>>> gc.get_referents(e)
[{'spacesIsOk': True, 'bootstrapIdx': 1},]调试泄露的KBEngine封装的Python对象：[KBEngine.debugTracing](#debugTracing)KBEngine.debugTracing类型：

Entities

类型：| Entities |

[Entities](Entities.html)EntitiesbaseAppData

baseAppData说明：这个属性包含一个类字典的对象，这个对象会在所有的BaseApps之间自动同步。
当字典的一个值被修改，这个修改会广播到所有的BaseApps。例子：
```
KBEngine.baseAppData[ "hello" ] = "there"

```
[KBEngine](KBEngine.html)KBEngine.[baseAppData](KBEngine.html#baseAppData)baseAppData[ "hello" ] = "there"其余BaseApps可以访问下面的：
```
print KBEngine.baseAppData[ "hello" ]

```
print[KBEngine](KBEngine.html)KBEngine.[baseAppData](KBEngine.html#baseAppData)baseAppData[ "hello" ]键和值可以是任意类型，但这些类型必须在所有目标组件上能够被封装和被拆封。当一个值被改变或被删除，一个回调函数会在所有组件被调用。
参看：[KBEngine](KBEngine.html)KBEngine.[onBaseAppData](KBEngine.html#onBaseAppData)onBaseAppData和[KBEngine](KBEngine.html)KBEngine.[onDelBaseAppData](KBEngine.html#onDelBaseAppData)onDelBaseAppData。注意：只有顶层的值才会被广播，如果你有一个值（比如一个列表），它改变了内部的值（比如只是改变一个数），这个信息不会被广播。不要进行下面的操作：
```
KBEngine.baseAppData[ "list" ] = [1, 2, 3]
KBEngine.baseAppData[ "list" ][1] = 7

```
[KBEngine](KBEngine.html)KBEngine.[baseAppData](KBEngine.html#baseAppData)baseAppData[ "list" ] = [1, 2, 3][KBEngine](KBEngine.html)KBEngine.[baseAppData](KBEngine.html#baseAppData)baseAppData[ "list" ][1] = 7这样，本地访问是[1, 7, 3]，远程访问是[1, 2, 3]。globalData

globalData说明：这个属性包含一个类字典的对象，这个对象会在所有的BaseApps和CellApps之间自动同步。
当字典的一个值被修改，这个修改会广播到所有的BaseApps和CellApps。例子：
```
KBEngine.globalData[ "hello" ] = "there"

```
[KBEngine](KBEngine.html)KBEngine.[globalData](KBEngine.html#globalData)globalData[ "hello" ] = "there"其余[Baseapp](../index.html)Baseapp或者[Cellapp](../index.html)[Cellapp](../../cellapp/index.html)Cellapp可以访问下面的：
```
print KBEngine.globalData[ "hello" ]

```
print[KBEngine](KBEngine.html)KBEngine.[globalData](KBEngine.html#globalData)globalData[ "hello" ]键和值可以是任意类型，但这些类型必须在所有目标组件上能够被封装和被拆封。当一个值被改变或被删除，一个回调函数会在所有组件被调用。
参看：[KBEngine](KBEngine.html)KBEngine.[onGlobalData](KBEngine.html#onGlobalData)onGlobalData和[KBEngine](KBEngine.html)KBEngine.[onGlobalDataDel](KBEngine.html#onGlobalDataDel)onGlobalDataDel。注意：只有顶层的值才会被广播，如果你有一个值（比如一个列表），它改变了内部的值（比如只是改变一个数），这个信息不会被广播。不要进行下面的操作：
```
KBEngine.globalData[ "list" ] = [1, 2, 3]
KBEngine.globalData[ "list" ][1] = 7

```
[KBEngine](KBEngine.html)KBEngine.[globalData](KBEngine.html#globalData)globalData[ "list" ] = [1, 2, 3][KBEngine](KBEngine.html)KBEngine.[globalData](KBEngine.html#globalData)globalData[ "list" ][1] = 7这样，本地访问是[1, 7, 3]，远程访问是[1, 2, 3]。
---
版权归KBEngine所有。

版权归KBEngine所有。