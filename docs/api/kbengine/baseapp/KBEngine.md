# KBEngine模块

[KBEngine](./KBEngine.md)模块提供了Python脚本访问实体的部分，特别是它提供了定时器的注册与移除，
以及实体的创建。

## 类

- Entity
- Proxy

## 成员函数

- def [addWatcher](#addWatcher)( path, dataType, getFunction ):
- def [address](#address)( ):
- def [MemoryStream](#MemoryStream)( ):
- def [charge](#charge)( ordersID, dbID, byteDatas, pycallback ):
- def [createEntity](#createEntity)( ):
- def [createEntityAnywhere](#createEntityAnywhere)( entityType, *params, callback ):
- def [createEntityRemotely](#createEntityRemotely)( entityType, baseMB, *params, callback ):
- def [createEntityFromDBID](#createEntityFromDBID)( entityType, dbID, callback, dbInterfaceName ):
- def [createEntityAnywhereFromDBID](#createEntityAnywhereFromDBID)( entityType, dbID, callback, dbInterfaceName ):
- def [createEntityRemotelyFromDBID](#createEntityRemotelyFromDBID)( entityType, dbID, baseMB, callback, dbInterfaceName ):
- def [createEntityLocally](#createEntityLocally)( entityType, *params ):
- def [debugTracing](#debugTracing)( ):
- def [delWatcher](#delWatcher)( path ):
- def [deleteEntityByDBID](#deleteEntityByDBID)( entityType, dbID, callback, dbInterfaceName ):
- def [deregisterReadFileDescriptor](#deregisterReadFileDescriptor)( fileDescriptor ):
- def [deregisterWriteFileDescriptor](#deregisterWriteFileDescriptor)( fileDescriptor ):
- def [executeRawDatabaseCommand](#executeRawDatabaseCommand)( command, callback, threadID, dbInterfaceName ):
- def [genUUID64](#genUUID64)( ):
- def [getResFullPath](#getResFullPath)( res ):
- def [getWatcher](#getWatcher)( path ):
- def [getWatcherDir](#getWatcherDir)( path ):
- def [getAppFlags](#getAppFlags)( ):
- def [hasRes](#hasRes)( res ):
- def [isShuttingDown](#isShuttingDown)( ):
- def [listPathRes](#listPathRes)( path, extension ):
- def [lookUpEntityByDBID](#lookUpEntityByDBID)( entityType, dbID, callback, dbInterfaceName ):
- def [matchPath](#matchPath)( res ):
- def [open](#open)( res, mode, encoding ):
- def [publish](#publish)( ):
- def [quantumPassedPercent](#quantumPassedPercent)( ):
- def [registerReadFileDescriptor](#registerReadFileDescriptor)( fileDescriptor, callback ):
- def [registerWriteFileDescriptor](#registerWriteFileDescriptor)( fileDescriptor, callback ):
- def [reloadScript](#reloadScript)( fullReload ):
- def [scriptLogType](#scriptLogType)( logType ):
- def [setAppFlags](#setAppFlags)( flags ):
- def [time](#time)( ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## 回调函数

- def [onBaseAppReady](#onBaseAppReady)( isBootstrap ):
- def [onBaseAppShutDown](#onBaseAppShutDown)( state ):
- def [onCellAppDeath](#onCellAppDeath)( addr ):
- def [onFini](#onFini)( ):
- def [onBaseAppData](#onBaseAppData)( key, value ):
- def [onBaseAppDataDel](#onBaseAppDataDel)( key ):
- def [onGlobalData](#onGlobalData)( key, value ):
- def [onGlobalDataDel](#onGlobalDataDel)( key ):
- def [onInit](#onInit)( isReload ):
- def [onLoseChargeCB](#onLoseChargeCB)( orderID, dbID, success, datas ):
- def [onReadyForLogin](#onReadyForLogin)( isBootstrap ):
- def [onReadyForShutDown](#onReadyForShutDown)( ):
- def [onAutoLoadEntityCreate](#onAutoLoadEntityCreate)( entityType, dbID ):

## 属性

| [LOG_ON_ACCEPT](#LOG_ON_ACCEPT) |  |
| --- | --- |
| [LOG_ON_REJECT](#LOG_ON_REJECT) |  |
| [LOG_ON_WAIT_FOR_DESTROY](#LOG_ON_WAIT_FOR_DESTROY) |  |
| [LOG_TYPE_DBG](#LOG_TYPE_DBG) |  |
| [LOG_TYPE_ERR](#LOG_TYPE_ERR) |  |
| [LOG_TYPE_INFO](#LOG_TYPE_INFO) |  |
| [LOG_TYPE_NORMAL](#LOG_TYPE_NORMAL) |  |
| [LOG_TYPE_WAR](#LOG_TYPE_WAR) |  |
| [NEXT_ONLY](#NEXT_ONLY) |  |
| [component](#component) | 只读string |
| [entities](#entities) | Entities |
| [baseAppData](#baseAppData) | GlobalDataClient |
| [globalData](#globalData) | GlobalDataClient |

## 成员函数文档

<a id="addWatcher"></a>

### def addWatcher(path, dataType, getFunction):

功能说明：
与调试监视系统交互，允许用户向监视系统注册一个监视变量。
例：

```python
>>> def countPlayers( ):
>>>     i = 0
>>>     for e in KBEngine.entities.values():
>>>     	if e.__class__.__name__ == "Avatar":
>>>     		i += 1
>>>     return i
>>>
>>> 
KBEngine
.
addWatcher
( "players", "UINT32", countPlayers )
```

这个函数添加一个监视变量在"scripts/players"监视路径之下。函数countPlayers在观察者观察时被调用。

参数：

| path | 创建监视的路径。 |
| --- | --- |
| dataType | 监视变量的值类型。参考:基本类型 |
| getFunction | 这个函数当观察者检索该变量时调用。 这个函数不带参数返回一个代表监视变量的值。 |

<a id="address"></a>

### def address():

功能说明：
返回内部网络接口的地址。

<a id="MemoryStream"></a>

### def MemoryStream():

功能说明：
返回一个新的MemoryStream对象。
MemoryStream对象存储的是二进制信息，提供这个类型是为了让用户能够方便的序列化与反序列化Python基本类型同时能与KBEngine底层序列化规则相同。
例如：你可以使用这个对象构造一个KBEngine能解析的网络数据包。
用法：

```python
>>> s = KBEngine.MemoryStream()
>>> s
>>> b''
>>> s.append("UINT32", 1)
>>> s.pop("UINT32")
>>> 1
```

目前MemoryStream能够支持的类型仅为基本数据类型。参考:基本类型

<a id="charge"></a>

### def charge(ordersID, dbID, byteDatas, pycallback):

功能说明：
计费接口。

参数：

| ordersID | string，订单ID。 |
| --- | --- |
| dbID | uint64，实体的databaseID。 |
| byteDatas | bytes，附带数据，由开发者自己解析和定义。 |
| pycallback | 计费回调。 计费回调原型: (当在interfaces中调用KBEngine.chargeResponse后，如果某个订单设置过回调则该回调被调用) def on**ChargeCB(self, orderID, dbID, success, datas): ordersID：string，订单ID dbID：uint64，通常为entity的databaseID success：bool，是否成功 datas：bytes，附带数据，由开发者自己解析和定义 |

<a id="createEntity"></a>

### def createEntity():

功能说明：
[KBEngine](./KBEngine.md).[createEntityLocally](./KBEngine.md#createEntityLocally)的别名.

<a id="createEntityAnywhere"></a>

### def createEntityAnywhere(entityType, params, callback):

功能说明：
创建一个新的Entity实体，
服务端可能选择任何的Baseapp来创建Entity实体。
这个方法应作为[KBEngine](./KBEngine.md).[createEntityLocally](./KBEngine.md#createEntityLocally)的首选，
这样服务端会灵活地选择一个合适的Baseapp来创建实体。
函数参数需要提供创建的实体的类型，还有一个Python字典作为参数来初始化实体的值。
这个Python字典不需要用户提供所有的属性，没有提供的属性默认为实体定义文件".def"提供的默认值。
例子：

```python
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

def onCreateEntityCallback(entity)
	print(entity)

createEntityAnywhere("Avatar", params, onCreateEntityCallback)
```

参数：

| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| params | 可选参数, 一个Python字典对象。 如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。 如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典， 然后在后面会用来初始化cell实体的属性。 |
| callback | callback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是Entity实体的entityCall，失败时是None。 |

返回：

- 通过回调返回Entity实体的entityCall。

<a id="createEntityRemotely"></a>

### def createEntityRemotely(entityType, baseMB, params, callback):

功能说明：
通过baseMB参数在一个指定的baseapp上创建一个新的Entity实体。
应该将[KBEngine](./KBEngine.md).[createEntityAnywhere](./KBEngine.md#createEntityLocally)方法作为首选。
函数参数需要提供创建的实体的类型，还有一个Python字典作为参数来初始化实体的值。
这个Python字典不需要用户提供所有的属性，没有提供的属性默认为实体定义文件".def"提供的默认值。
例子：

```python
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

def onCreateEntityCallback(entity)
	print(entity)

createEntityRemotely("Avatar", baseEntityCall, params, onCreateEntityCallback)
```

参数：

| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| baseMB | BaseEntityCall，这是一个Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。 |
| params | 可选参数, 一个Python字典对象。 如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。 如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典， 然后在后面会用来初始化cell实体的属性。 |
| callback | callback是一个可选的回调函数，当实体完成创建时被调用。回调函数带有一个参数，当成功的时候是Entity实体的entityCall，失败时是None。 |

返回：

- 通过回调返回Entity实体的entityCall。

<a id="createEntityFromDBID"></a>

### def createEntityFromDBID(entityType, dbID, callback, dbInterfaceName):

功能说明：
从数据库里加载数据创建一个Entity实体。
这个新的Entity实体会在调用这个函数的Baseapp上创建。
如果该实体已经从数据库检出，那么将返回这个存在的Entity实体的引用。

参数：

| entityType | string，指定要加载的Entity实体类型。实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | 这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。 如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活 wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。 失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

<a id="createEntityAnywhereFromDBID"></a>

### def createEntityAnywhereFromDBID(entityType, dbID, callback, dbInterfaceName):

功能说明：
从数据库里加载数据创建一个Entity实体。
服务端可能选择任何的Baseapp来创建Entity实体。
使用这个函数将有助于BaseApps负载平衡。
如果该实体已经从数据库检出，那么将返回这个存在的Entity实体的引用。

参数：

| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | 这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。 如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活 wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。 失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

返回：

- 通过回调返回Entity实体的entityCall。

<a id="createEntityRemotelyFromDBID"></a>

### def createEntityRemotelyFromDBID(entityType, dbID, baseMB, callback, dbInterfaceName):

功能说明：
从数据库里加载数据并通过baseMB参数在一个指定的baseapp上创建一个Entity实体。
如果该实体已经从数据库检出，那么将返回这个存在的Entity实体的引用。

参数：

| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 这是一个指定要创建的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| baseMB | BaseEntityCall，这是一个Entity的EntityCall。实体将被创建在该Entity对应的Baseapp进程上。 |
| callback | 这是一个可选的回调函数，当操作完成的时候它会被调用。回调函数带有3个参数：baseRef，databaseID和wasActive。 如果操作成功，baseRef会是一个entityCall或者是新创建的Entity实体的直接引用，databaseID会是实体的数据库ID，无论该实体是否已经激活 wasActive都会有所指示，如果wasActive是True则baseRef是已经存在的实体的引用(已经从数据库检出)。如果操作失败这三个参数的值，baseRef将会是None，databaseID将会是0，wasActive将会是False。 失败最常见的原因是实体在数据库里不存在，但偶尔也会出现其它错误比如说超时或者是分配ID失败。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

返回：

- 通过回调返回Entity实体的entityCall。

<a id="createEntityLocally"></a>

### def createEntityLocally(entityType, params):

功能说明：
创建一个新的Entity实体。
函数参数需要提供创建的实体的类型，还有一个Python字典作为参数来初始化实体的值。
这个Python字典不需要用户提供所有的属性，没有提供的属性默认为实体定义文件".def"提供的默认值。
[KBEngine](./KBEngine.md).[createEntityAnywhere](./KBEngine.md#createEntityAnywhere)应该作为这个方法的首选，因为服务端可以灵活地
在合适的Baseapp上创建实体。
例子：

```python
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

baseEntity = createEntityLocally("Avatar", params)
```

参数：

| entityType | string，指定要创建的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| params | 可选参数, 一个Python字典对象。 如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。 如果这个键是一个Cell属性，它会被添加到Entity实体的'cellData'属性，这个'cellData'属性是一个Python字典， 然后在后面会用来初始化cell实体的属性。 |

返回：

- 新创建的Entity实体（参考Entity）

<a id="debugTracing"></a>

### def debugTracing():

功能说明：
输出当前KBEngine追踪的Python扩展对象计数器。
扩展对象包括：固定字典、固定数组、Entity、EntityCall...
在服务端正常关闭时如果计数器不为零，此时说明泄露已存在，日志将会输出错误信息。
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): FixedArray : leaked(128)
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): EntityCall : leaked(8)

参数：

| path | 要删除的变量的路径。 |
| --- | --- |

<a id="delWatcher"></a>

### def delWatcher(path):

功能说明：
与调试监视系统交互，允许用户在脚本删除监视的变量。

参数：

| path | 要删除的变量的路径。 |
| --- | --- |

<a id="deleteEntityByDBID"></a>

### def deleteEntityByDBID(entityType, dbID, callback, dbInterfaceName):

功能说明：
从数据库删除指定的实体（包括属性所产生的子表数据），如果实体没有从数据库检出则删除成功，
如果实体已经从数据库检出那么[KBEngine](./KBEngine.md)服务系统将会删除失败，并且从回调中返回Entity实体的entityCall。

参数：

| entityType | string，指定要删除的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要删除的实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | callback是一个可选的回调函数，只有一个参数，当实体没有从数据库检出时将会成功删除数据，参数是True。如果实体已经从数据库检出那么参数是Entity实体的entityCall。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

<a id="deregisterReadFileDescriptor"></a>

### def deregisterReadFileDescriptor(fileDescriptor):

功能说明：
注销已经通过[KBEngine](./KBEngine.md).[registerReadFileDescriptor](./KBEngine.md#registerReadFileDescriptor)注册的回调。
例子:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

参数：

| fileDescriptor | socket描述符/文件描述符。 |
| --- | --- |

<a id="deregisterWriteFileDescriptor"></a>

### def deregisterWriteFileDescriptor(fileDescriptor):

功能说明：
注销已经通过[KBEngine](./KBEngine.md).[registerWriteFileDescriptor](./KBEngine.md#registerWriteFileDescriptor)注册的回调。
例子:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

参数：

| fileDescriptor | socket描述符/文件描述符。 |
| --- | --- |

<a id="executeRawDatabaseCommand"></a>

### def executeRawDatabaseCommand(command, callback, threadID, dbInterfaceName):

功能说明：
这个脚本函数在数据库上执行原始数据库命令，该命令将直接由相关数据库进行解析。
请注意使用该函数修改实体数据可能不生效，因为如果实体已经检出，被修改过的实体数据将仍会被实体存档而导致覆盖。
强烈不推荐这个函数用于读取或修改实体数据。

参数：

| command | 这个数据库命令将会因为不同数据库配置方案而不同。对于方案为MySQL数据库它是一个SQL查询语句。 |
| --- | --- |
| callback | 可选参数，带有命令执行结果的回调对象（比如说是一个函数）。这个回调带有4个参数：结果集合，影响的行数，自増长值，错误信息。 声明样例： def sqlcallback(result, rows, insertid, error): print(result, rows, insertid, error) 如同上面的例子所示，result参数对应的就是“结果集合”，这个结果集合参数是一个行列表。 每一行是一个包含字段值的字符串列表。 命令执行没有返回结果集合（比如说是DELETE命令）， 或者 命令执行有错误时这个结果集合为None。 rows参数则是“影响的行数”，它是一个整数，表示命令执行受影响的行数。这个参数只和不返回结果结合的命令（如DELETE）相关。 如果有结果集合返回或者命令执行有错误时这个参数为None。<br>insertid对应的是“自増长值”，类似于实体的databaseID，当成功的向一张带有自増长类型字段的表中插入数据时，它返回该数据在插入时自増长字段所被赋于的值。 更多的信息可以参阅mysql的mysql_insert_id()方法。另外，此参数仅在数据库类型为mysql时有意义。 error则对应了“错误信息”，当命令执行有错误时，这个参数是一个描述错误的字符串。命令执行没有发生错误时这个参数为None。 |
| threadID | int32，可选参数，指定一个线程来处理本条命令。用户可以通过这个参数控制某一类命令的执行先后顺序（dbmgr是多线程处理的），默认是不指定，如果threadID是实体的ID， 那么将加入到该实体的存档队列中由线程逐条写入。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

<a id="genUUID64"></a>

### def genUUID64():

功能说明：
该函数生成一个64位的唯一ID。
注意：这个函数依赖于Baseapps服务进程启动参数gus，请正确设置启动参数保持唯一性。
另外如果gus超过65535则该函数只能在当前进程上保持唯一性。
用途：
多个服务进程上产生唯一物品ID并且在合服时不会产生冲突。
多个服务进程上产生一个房间ID，不需要进行唯一性校验。

返回：

- 返回一个64位的integer。

<a id="getResFullPath"></a>

### def getResFullPath(res):

功能说明：
获取资源的绝对路径。
注意：资源必须在KBE_RES_PATH之下才可以访问到。

参数：

| res | string，资源的相对路径。 |
| --- | --- |

返回：

- string，如果存在返回资源的绝对路径，否则返回空。

<a id="getWatcher"></a>

### def getWatcher(path):

功能说明：
从KBEngine调试系统中获取一个监视变量的值。
例子：在baseapp1的Python命令行输入:
>>>KBEngine.getWatcher("/root/stats/runningTime")
12673648533
>>>KBEngine.getWatcher("/root/scripts/players")
32133

参数：

| path | string，该变量的绝对路径包括变量名(可以在GUIConsole的watcher页查看)。 |
| --- | --- |

返回：

- 该变量的值。

<a id="getWatcherDir"></a>

### def getWatcherDir(path):

功能说明：
从KBEngine调试系统中获取一个监视目录下的元素列表(目录、变量名)。
例子：在baseapp1的Python命令行输入:
>>>KBEngine.getWatcher("/root")
('stats', 'objectPools', 'network', 'syspaths', 'ThreadPool', 'cprofiles', 'scripts', 'numProxices', 'componentID', 'componentType', 'uid', 'numClients', 'globalOrder', 'username', 'load', 'gametime', 'entitiesSize', 'groupOrder')

参数：

| path | string，该变量的绝对路径(可以在GUIConsole的watcher页查看)。 |
| --- | --- |

返回：

- 监视目录下的元素列表(目录、变量名)。

<a id="getAppFlags"></a>

### def getAppFlags():

功能说明：
获取当前引擎APP的标记, 参考:[KBEngine](./KBEngine.md).[setAppFlags](./KBEngine.md#setAppFlags)。

返回：

- KBEngine.APP_FLAGS_*。

<a id="hasRes"></a>

### def hasRes(res):

功能说明：
使用这个接口可以判断一个相对路径的资源是否存在。
注意：资源必须在KBE_RES_PATH之下才可以访问到。
例子:
>>>KBEngine.hasRes("scripts/entities.xml")
True

参数：

| res | string，资源的相对路径。 |
| --- | --- |

返回：

- BOOL, 存在返回True，否则返回False。

<a id="isShuttingDown"></a>

### def isShuttingDown():

功能说明：
返回服务端是否正在关闭中。
在[onBaseAppShutDown(state=0)](./KBEngine.md#onBaseAppShutDown)回调函数被调用后，这个函数返回True。

返回：

- 系统正在关闭返回True，否则返回False。

<a id="listPathRes"></a>

### def listPathRes(path, extension):

功能说明：
获得一个资源目录下的资源列表。
注意：资源必须在KBE_RES_PATH之下才可以访问到。
例子:
>>>KBEngine.listPathRes("scripts/cell/interfaces")
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')
>>>KBEngine.listPathRes("scripts/cell/interfaces", "txt")
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')
>>>KBEngine.listPathRes("scripts/cell/interfaces", "txt|py")
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')
>>>KBEngine.listPathRes("scripts/cell/interfaces", ("txt", "py"))
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/新建文本文档.txt')

参数：

| res | string，资源的相对路径。 |
| --- | --- |
| extension | string，可选参数，扩展名。 |

返回：

- Tuple, 资源列表。

<a id="lookUpEntityByDBID"></a>

### def lookUpEntityByDBID(entityType, dbID, callback, dbInterfaceName):

功能说明：
查询一个实体是否从数据库检出，
如果实体已经从数据库检出那么[KBEngine](./KBEngine.md)服务系统将从回调中返回Entity实体的entityCall。

参数：

| entityType | string，指定要查询的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要查询的Entity实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |
| callback | callback只有一个参数，当实体没有从数据库检出时将会返回True。如果实体已经从数据库检出那么将返回Entity实体的entityCall, 其他任何情况返回False。 |
| dbInterfaceName | string，可选参数，指定由某个数据库接口来完成, 默认使用"default"接口。数据库接口由kbengine_defaults.xml->dbmgr->databaseInterfaces中定义。 |

<a id="matchPath"></a>

### def matchPath(res):

功能说明：
使用相对路径的资源获得资源的绝对路径。
注意：资源必须在KBE_RES_PATH之下才可以访问到。
例子:
>>>KBEngine.matchPath("scripts/entities.xml")
'/home/kbe/kbengine/demo/res/scripts/entities.xml'

参数：

| res | string，资源的相对路径(包括资源名称)。 |
| --- | --- |

返回：

- string, 资源的绝对路径。

<a id="open"></a>

### def open(res, mode, encoding):

功能说明：
使用这个接口可以使用相对路径来打开相关资源。
注意：资源必须在KBE_RES_PATH之下才可以访问到。

参数：

| res | string，资源的相对路径。 |
| --- | --- |
| mode | string，可选参数，默认值是'r'，文件操作模式： r 以只读方式打开， w 以写方式打开， a 以追加模式打开 (从 EOF 开始, 必要时创建新文件) r+ 以读写模式打开 w+ 以读写模式打开 (参见 w ) a+ 以读写模式打开 (参见 a ) rb 以二进制读模式打开 wb 以二进制写模式打开 (参见 w ) ab 以二进制追加模式打开 (参见 a ) rb+ 以二进制读写模式打开 (参见 r+ ) wb+ 以二进制读写模式打开 (参见 w+ ) ab+ 以二进制读写模式打开 (参见 a+ ) |
| encoding | string，可选参数，用于解码或编码文件的编码的名称，默认编码与平台相关。 |

<a id="publish"></a>

### def publish():

功能说明：
这个接口返回当前服务端发行模式。

返回：

- int8，0：debug，1：release，其它可自定义。

<a id="quantumPassedPercent"></a>

### def quantumPassedPercent():

功能说明：
返回取得当前tick占用一个时钟周期的百分比。

返回：

- 返回取得当前tick占用一个时钟周期的百分比。

<a id="registerReadFileDescriptor"></a>

### def registerReadFileDescriptor(fileDescriptor, callback):

功能说明：
注册一个回调函数，这个回调函数当文件描述符可读时被调用。
例子:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

参数：

| fileDescriptor | socket描述符/文件描述符。 |
| --- | --- |
| callback | 一个回调函数，socket描述符/文件描述符作为它的唯一参数。 |

<a id="registerWriteFileDescriptor"></a>

### def registerWriteFileDescriptor(fileDescriptor, callback):

功能说明：
注册一个回调函数，这个回调函数当socket描述符/文件描述符可写时被调用。
例子:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

参数：

| fileDescriptor | socket描述符/文件描述符。 |
| --- | --- |
| callback | 一个回调函数，socket描述符/文件描述符作为它的唯一参数。 |

<a id="reloadScript"></a>

### def reloadScript(fullReload):

功能说明：
重新加载与实体和自定义数据类型相关的Python模块。当前实体类会设置为新加载的类。
这个方法应该只用于开发模式，对于产品模式不合适。下面几点应该注意：

1）本次调用只影响当前 BaseApp 进程，用户应该确保所有相关服务端组件都执行热更新。
2）自定义类型在热更新后应该确保内存中已经实例化的对象也被更新，下面是一个例子:

```python
for e in KBEngine.entities.values():
   if type( e ) is Avatar.Avatar:
      e.customData.__class__ = CustomClass
```

3）已经保存到 timer、事件表、回调表里的旧 Python callable 不会自动重绑，需要业务在 `onInit(1)` 或统一 reload hook 中清理并重建。

当这个方法完成时[KBEngine](./KBEngine.md).[onInit](./KBEngine.md#onInit)(1) 被调用。

参数：

| fullReload | 可选的boolean参数，指定是否同时重新加载实体定义。如果这个参数为False，则实体定义不会被重新加载。默认为True。 |
| --- | --- |

返回：

- 无返回值（`None`）。热更新过程中的错误通过 Python 异常或日志输出。

<a id="scriptLogType"></a>

### def scriptLogType(logType):

功能说明：
设置当前Python.print输出的信息类型(参考: KBEngine.LOG_TYPE_*)。

<a id="setAppFlags"></a>

### def setAppFlags(flags):

功能说明：
设置当前引擎APP的标记。
KBEngine.APP_FLAGS_NONE // 默认的(未设置标记)
KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING //不参与负载均衡
例如：
KBEngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING | KBEngine.APP_FLAGS_*)

<a id="time"></a>

### def time():

功能说明：
这个方法返回当前游戏的时间（周期数）。

返回：

- uint32，当前游戏的时间，这里指周期数，周期受频率影响，频率由配置文件kbengine.xml或者kbengine_defaults.xml->gameUpdateHertz决定。

<a id="urlopen"></a>

### def urlopen(url, callback, postData, headers):

功能说明：
这个脚本函数在提供对外HTTP/HTTPS异步请求。

参数：

| url | 有效的HTTP/HTTPS网址，字符串类型。 |
| --- | --- |
| callback | 可选参数，带有请求执行结果的回调对象（比如说是一个函数）。这个回调带有5个参数：HTTP请求返回码（如：200)，返回的内容，返回的HTTP协议头，是否成功，请求的网址。 声明样例： def onHttpCallback(httpcode, data, headers, success, url): print(httpcode, data, headers, success, url) 如同上面的例子所示:<br>httpcode:参数对应的就是“HTTP请求返回码”，这个结果集合参数是一个整形值。<br>data:参数则是“返回的内容”，它是一个字符串。<br>headers:参数是“服务器返回的HTTP协议头”，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。<br>success:则对应了“执行是否成功”，当请求执行有错误时，为False，可以通过httpcode进一步判断错误信息。<br>url:是“请求所用的网址。 |
| postData | 可选参数，默认是GET方式请求服务器，如果需要POST方式请提供需要POST的内容，引擎将自动使用POST方式请求服务器，它是一个bytes。 |
| headers | 可选参数，请求时使用的HTTP头，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。 |

## 回调函数文档

<a id="onBaseAppReady"></a>

### def onBaseAppReady(isBootstrap):

功能说明：
当前Baseapp进程已经准备好的时候回调此函数。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| isBootstrap | bool，是否为第一个启动的Baseapp。 |
| --- | --- |

<a id="onBaseAppShutDown"></a>

### def onBaseAppShutDown(state):

功能说明：
Baseapp关闭过程会回调此函数。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| state | 如果state为0，意指在断开所有客户端之前，如果state为1，意指在将所有实体写入数据库之前，如果state为2，意指在所有实体被写入数据库之后。 |
| --- | --- |

<a id="onCellAppDeath"></a>

### def onCellAppDeath(addr):

功能说明：
某个cellapp死亡会回调此函数。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| addr | 死亡的cellapp地址。 tuple:(ip, port) 网络字节序 |
| --- | --- |

<a id="onFini"></a>

### def onFini():

功能说明：
引擎正式关闭后回调此函数。
注意：该回调接口必须实现在入口模块kbengine_defaults.xml->entryScriptFile)中。

<a id="onBaseAppData"></a>

### def onBaseAppData(key, value):

功能说明：
当前 `BaseApp` 在收到其他 `BaseApp` 对 `KBEngine.baseAppData` 的广播，或在启动阶段从 `dbmgr` 回放已有键值时，如果某个顶层 `key` 被新增或覆盖，就会回调此函数。
当前进程自己直接执行 `KBEngine.baseAppData[key] = value` 时，不会额外再回调一次本函数。

注意：

- 该回调接口必须实现在入口模块(`kbengine_defaults.xml -> entryScriptFile`)中。
- 只有顶层 `key -> value` 的新增或整体改写会触发；`value` 内部可变对象的局部修改不会触发。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

参数：

| key | 被改变数据的键。 |
| --- | --- |
| value | 被改变数据的值。 |

<a id="onBaseAppDataDel"></a>

### def onBaseAppDataDel(key):

功能说明：
当前 `BaseApp` 在收到其他 `BaseApp` 对 `KBEngine.baseAppData` 的删除广播时，会在对应顶层 `key` 从本地字典移除后回调此函数。
当前进程自己直接执行 `del KBEngine.baseAppData[key]` 时，不会额外再回调一次本函数。

注意：

- 该回调接口必须实现在入口模块(`kbengine_defaults.xml -> entryScriptFile`)中。
- 只对应顶层 `key` 的删除同步。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

参数：

| key | 被删除数据的键。 |
| --- | --- |

<a id="onGlobalData"></a>

### def onGlobalData(key, value):

功能说明：
当前 `BaseApp` 在收到其他 `BaseApp` 或 `CellApp` 对 `KBEngine.globalData` 的广播，或在启动阶段从 `dbmgr` 回放已有键值时，如果某个顶层 `key` 被新增或覆盖，就会回调此函数。
当前进程自己直接执行 `KBEngine.globalData[key] = value` 时，不会额外再回调一次本函数。

注意：

- 该回调接口必须实现在入口模块(`kbengine_defaults.xml -> entryScriptFile`)中。
- 只有顶层 `key -> value` 的新增或整体改写会触发；`value` 内部可变对象的局部修改不会触发。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

参数：

| key | 被改变数据的键。 |
| --- | --- |
| value | 被改变数据的值。 |

<a id="onGlobalDataDel"></a>

### def onGlobalDataDel(key):

功能说明：
当前 `BaseApp` 在收到其他 `BaseApp` 或 `CellApp` 对 `KBEngine.globalData` 的删除广播时，会在对应顶层 `key` 从本地字典移除后回调此函数。
当前进程自己直接执行 `del KBEngine.globalData[key]` 时，不会额外再回调一次本函数。

注意：

- 该回调接口必须实现在入口模块(`kbengine_defaults.xml -> entryScriptFile`)中。
- 只对应顶层 `key` 的删除同步。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

参数：

| key | 被删除数据的键。 |
| --- | --- |

<a id="onInit"></a>

### def onInit(isReload):

功能说明：
当引擎启动后初始化完所有的脚本后这个接口被调用。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| isReload | bool，是否是被重写加载脚本后触发的。 |
| --- | --- |

<a id="onLoseChargeCB"></a>

### def onLoseChargeCB(orderID, dbID, success, datas):

功能说明：
当在interfaces中调用KBEngine.chargeResponse后，如果该订单丢失或者是不明interfaces未被记录的订单会收到此回调通知。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| ordersID | string，订单ID。 |
| --- | --- |
| dbID | uint64，实体的数据库ID, 参见:Entity.databaseID。 |
| success | bool，是否成功。 |
| datas | bytes，附带信息。 |

<a id="onReadyForLogin"></a>

### def onReadyForLogin(isBootstrap):

功能说明：
当引擎启动并初始化完成后会一直调用此接口询问脚本层是否准备完毕，如果脚本层准备完毕则loginapp允许客户端登录。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| isBootstrap | bool，是否为第一个启动的Baseapp。 |
| --- | --- |

返回：

- 返回值大于等于1.0则脚本层准备完成，否则返回准备的进度值0.0~1.0。

<a id="onReadyForShutDown"></a>

### def onReadyForShutDown():

功能说明：
如果这个函数在脚本中有实现，当进程准备退出时，该回调函数被调用。
可以通过该回调控制进程退出的时机。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

返回：

- bool，如果返回True，则允许进入进程退出流程，返回其它值则进程会过一段时间后再次询问。

<a id="onAutoLoadEntityCreate"></a>

### def onAutoLoadEntityCreate(entityType, dbID):

功能说明：
自动加载的实体创建时的回调，如果脚本层实现此回调，那么实体由脚本层创建，否则引擎默认使用createEntityAnywhereFromDBID来创建实体。
这个回调被调用是由于Entity.writeToDB时设置了实体自动加载。
注：该回调优先于onBaseAppReady执行，可在onBaseAppReady时检查是否已加载实体。

参数：

| entityType | string，指定要查询的Entity实体的类型。有效的实体类型在/scripts/entities.xml列出。 |
| --- | --- |
| dbID | 指定要查询的Entity实体的数据库ID。这个实体的数据库ID存储在该实体的databaseID属性。 |

## 属性文档

<a id="LOG_ON_ACCEPT"></a>

LOG_ON_ACCEPT

说明：
这个常量由Proxy.onLogOnAttempt返回，意指允许新的client与一个Proxy实体绑定。
如果Proxy实体已经存在一个client绑定关系，那么将踢出之前的client。

<a id="LOG_ON_REJECT"></a>

LOG_ON_REJECT

说明：
这个常量由Proxy.onLogOnAttempt返回，意指拒绝当前client与Proxy实体绑定。

<a id="LOG_ON_WAIT_FOR_DESTROY"></a>

LOG_ON_WAIT_FOR_DESTROY

说明：
这个常量由Proxy.onLogOnAttempt返回，当前请求client将会等待直到Proxy实体完全销毁，底层再完成后续绑定过程。
在这返回之前Proxy.destroy或者Proxy.destroyCellEntity应该被调用。

<a id="LOG_TYPE_DBG"></a>

LOG_TYPE_DBG

说明：
日志输出类型为调试类型。
由[scriptLogType](#scriptLogType)设置。

<a id="LOG_TYPE_ERR"></a>

LOG_TYPE_ERR

说明：
日志输出类型为错误类型。
由[scriptLogType](#scriptLogType)设置。

<a id="LOG_TYPE_INFO"></a>

LOG_TYPE_INFO

说明：
日志输出类型为常规信息类型。
由[scriptLogType](#scriptLogType)设置。

<a id="LOG_TYPE_NORMAL"></a>

LOG_TYPE_NORMAL

说明：
日志输出类型为常规类型。
由[scriptLogType](#scriptLogType)设置。

<a id="LOG_TYPE_WAR"></a>

LOG_TYPE_WAR

说明：
日志输出类型为警告类型。
由[scriptLogType](#scriptLogType)设置。

<a id="NEXT_ONLY"></a>

NEXT_ONLY

说明：
这个常量用于Entity.shouldAutoBackup和Entity.shouldAutoArchive属性。这个值意指在下一次认为可以的时候自动备份该实体，然后这个属性自动设为False（0）。

<a id="component"></a>

component

说明：
这是正运行在当前Python环境的组件。（至今为止）可能值有'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' 和 'editor'。

<a id="entities"></a>

entities

说明：
entities是一个字典对象，包含当前进程上所有的实体。
调试泄露的实体（调用过destroy却没有释放内存的实体，通常是由于被引用导致无法释放）：

```python
>>> KBEngine.entities.garbages.items()
[(1025, Avatar object at 0x7f92431ceae8.)]


>>> e = _[0][1]
>>> import gc
>>> gc.get_referents(e)
[{'spacesIsOk': True, 'bootstrapIdx': 1}, 
]
```

调试泄露的KBEngine封装的Python对象：
[KBEngine.debugTracing](#debugTracing)

类型：

- Entities

<a id="baseAppData"></a>

baseAppData

说明：
`BaseApp` 进程级共享字典，只在所有 `BaseApp` 之间同步。
脚本执行 `KBEngine.baseAppData[key] = value` 或 `del KBEngine.baseAppData[key]` 时，请求会先发往 `dbmgr` 持有的权威副本，再广播给其他 `BaseApp`。
写入方本地值会在本次脚本赋值/删除流程中直接更新，不会再收到一次自己的回环广播；其他 `BaseApp` 收到广播后，才会触发 [onBaseAppData](./KBEngine.md#onBaseAppData) 或 [onBaseAppDataDel](./KBEngine.md#onBaseAppDataDel)。
新启动的 `BaseApp` 在登录 `dbmgr` 时，还会收到当前全量快照，不只是后续增量。
键和值都必须能在目标组件上完成 `pickle/unpickle`。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

例子：

```python
KBEngine.baseAppData["hello"] = "there"
```

其余 `BaseApp` 可以读取：

```python
print(KBEngine.baseAppData["hello"])
```

参看：[KBEngine](./KBEngine.md).[onBaseAppData](./KBEngine.md#onBaseAppData) 和 [KBEngine](./KBEngine.md).[onBaseAppDataDel](./KBEngine.md#onBaseAppDataDel)。

注意：

- 同步粒度是顶层 `key -> value`；只改 `value` 内部元素不会广播。
- 它不会同步到 `CellApp`。
- 不要做下面这种“局部改列表元素”的写法：

```python
KBEngine.baseAppData["list"] = [1, 2, 3]

KBEngine.baseAppData["list"][1] = 7
```

这样本地看到的是 `[1, 7, 3]`，其他 `BaseApp` 仍然是 `[1, 2, 3]`。

<a id="globalData"></a>

globalData

说明：
`BaseApp` 与 `CellApp` 共享的进程级字典。
脚本执行 `KBEngine.globalData[key] = value` 或 `del KBEngine.globalData[key]` 时，请求会先进入 `dbmgr` 持有的权威副本，再由 `dbmgr` 广播给所有其他 `BaseApp` 和 `CellApp`。
写入方本地不会收到一遍自己的同步回调；其他组件在完成反序列化和本地落地后，才会触发 [onGlobalData](./KBEngine.md#onGlobalData) 或 [onGlobalDataDel](./KBEngine.md#onGlobalDataDel)。
新启动的 `BaseApp` 或 `CellApp` 也会在登录 `dbmgr` 后收到当前全量快照。
键和值都必须能在目标组件上完成 `pickle/unpickle`。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

例子：

```python
KBEngine.globalData["hello"] = "there"
```

其余 `BaseApp` 或 `CellApp` 可以读取：

```python
print(KBEngine.globalData["hello"])
```

参看：[KBEngine](./KBEngine.md).[onGlobalData](./KBEngine.md#onGlobalData) 和 [KBEngine](./KBEngine.md).[onGlobalDataDel](./KBEngine.md#onGlobalDataDel)。

注意：

- 同步粒度是顶层 `key -> value`；只改 `value` 内部元素不会广播。
- 它同步给所有 `BaseApp` 和 `CellApp`，但不会同步到 `loginapp`、`interfaces` 这类其他组件。
- 不要做下面这种“局部改列表元素”的写法：

```python
KBEngine.globalData["list"] = [1, 2, 3]

KBEngine.globalData["list"][1] = 7
```

这样本地看到的是 `[1, 7, 3]`，远端组件仍然是 `[1, 2, 3]`。
