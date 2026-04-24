# KBEngine模块

[KBEngine](./KBEngine.md)模块提供了Python脚本访问实体的部分，特别是它提供了定时器的注册与移除，
以及实体的创建。

## 类

- Entity

## 成员函数

- def [addSpaceGeometryMapping](#addSpaceGeometryMapping)( spaceID, mapper, path, shouldLoadOnServer, params ):
- def [addWatcher](#addWatcher)( path, dataType, getFunction ):
- def [address](#address)( ):
- def [MemoryStream](#MemoryStream)( ):
- def [createEntity](#createEntity)( entityType, spaceID, position, direction, params ):
- def [debugTracing](#debugTracing)( ):
- def [delSpaceData](#delSpaceData)( spaceID, key ):
- def [delWatcher](#delWatcher)( path ):
- def [deregisterReadFileDescriptor](#deregisterReadFileDescriptor)( fileDescriptor ):
- def [deregisterWriteFileDescriptor](#deregisterWriteFileDescriptor)( fileDescriptor ):
- def [executeRawDatabaseCommand](#executeRawDatabaseCommand)( command, callback, threadID, dbInterfaceName ):
- def [genUUID64](#genUUID64)( ):
- def [getResFullPath](#getResFullPath)( res ):
- def [getSpaceData](#getSpaceData)( spaceID, key ):
- def [getSpaceGeometryMapping](#getSpaceGeometryMapping)( spaceID ):
- def [getWatcher](#getWatcher)( path ):
- def [getWatcherDir](#getWatcherDir)( path ):
- def [getAppFlags](#getAppFlags)( ):
- def [hasRes](#hasRes)( res ):
- def [isShuttingDown](#isShuttingDown)( ):
- def [listPathRes](#listPathRes)( path, extension ):
- def [matchPath](#matchPath)( res ):
- def [open](#open)( res, mode, encoding ):
- def [publish](#publish)( ):
- def [registerReadFileDescriptor](#registerReadFileDescriptor)( fileDescriptor, callback ):
- def [registerWriteFileDescriptor](#registerWriteFileDescriptor)( fileDescriptor, callback ):
- def [raycast](#raycast)( spaceID, layer, src, dst ):
- def [reloadScript](#reloadScript)( fullReload ):
- def [scriptLogType](#scriptLogType)( logType ):
- def [setAppFlags](#setAppFlags)( flags ):
- def [setSpaceData](#setSpaceData)( spaceID, key, value ):
- def [time](#time)( ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## 回调函数

- def [onCellAppData](#onCellAppData)( key, value ):
- def [onCellAppDataDel](#onCellAppDataDel)( key ):
- def [onGlobalData](#onGlobalData)( key, value ):
- def [onGlobalDataDel](#onGlobalDataDel)( key ):
- def [onInit](#onInit)( isReload ):
- def [onSpaceData](#onSpaceData)( spaceID, key, value ):
- def [onSpaceGeometryLoaded](#onSpaceGeometryLoaded)( spaceID, mapping ):
- def [onAllSpaceGeometryLoaded](#onAllSpaceGeometryLoaded)( spaceID, isBootstrap, mapping ):
- def [onReadyForLogin](#onReadyForLogin)( isBootstrap ):

## 属性

| [LOG_TYPE_DBG](#LOG_TYPE_DBG) |  |
| --- | --- |
| [LOG_TYPE_ERR](#LOG_TYPE_ERR) |  |
| [LOG_TYPE_INFO](#LOG_TYPE_INFO) |  |
| [LOG_TYPE_NORMAL](#LOG_TYPE_NORMAL) |  |
| [LOG_TYPE_WAR](#LOG_TYPE_WAR) |  |
| [NEXT_ONLY](#NEXT_ONLY) |  |
| [cellAppData](#cellAppData) |  |
| [component](#component) | 只读string |
| [entities](#entities) | Entities |
| [globalData](#globalData) | GlobalDataClient |

## 成员函数文档

<a id="addSpaceGeometryMapping"></a>

### def addSpaceGeometryMapping(spaceID, mapper, path, shouldLoadOnServer, params):

功能说明：
关联一个给定空间的几何映射，函数调用之后服务端和客户端都会加载相应的几何体数据。
在服务端上，从给定目录里加载所有的几何数据到指定的空间。这些数据可能被分成很多区块，不同区块是异步加载的，当所有的几何数据加载
完成的时候下面的通知方法会被调用：

```python
	def onAllSpaceGeometryLoaded( self, spaceID, mappingName ):
```

服务端仅加载场景的几何数据提供给导航和碰撞功能使用，客户端除了几何数据外还会加载纹理等数据。
3D场景当前默认使用的是recastnavigation插件所导出的数据，2D场景当前默认使用的是MapEditor编辑器导出的数据。
有一种可能会导致onAllSpaceGeometryLoaded()不被调用，就是如果在某一个
时刻多个CellApp同时调用这个方法来添加几何到相同的空间的时候CellAppMgr崩溃了。

参数：

| spaceID | uint32，空间的ID，指定在哪个空间操作 |
| --- | --- |
| mapper | 目前填None |
| path | 包含几何数据的目录路径 |
| shouldLoadOnServer | 可选的boolean参数，指定是否在服务端上加载几何。默认为True |
| params | 可选的PyDict参数，指定不同layer所使用的navmesh，例如： KBEngine.addSpaceGeometryMapping(self.spaceID, None, resPath, True, {0 : "srv_xinshoucun_1.navmesh", 1 : "srv_xinshoucun.navmesh"}) |

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

<a id="createEntity"></a>

### def createEntity(entityType, spaceID, position, direction, params):

功能说明：
createEntity在当前进程指定space中创建一个新的实体。
这个函数需要指定要创建的实体的类别，位置和方向，还可以选择性地设置实体的任意属性（属性在实体的.def文件里描述）。
例子：

```python
# 创建一个打开的门的实体与"thing"实体的位置一样
direction = ( 0, 0, thing.yaw )
properties = { "open":1 }

KBEngine
.
createEntity
( "Door", thing.space, thing.position, direction,
                       properties )
```

参数：

| entityType | string，要创建的实体名称，在/scripts/entities.xml文件里声明。 |
| --- | --- |
| spaceID | int32，要放置实体的空间的ID。 |
| position | 由3个float组成的序列，指定新实体的出生点，在世界中的坐标。 |
| direction | 由3个float组成的序列，指定新实体的初始朝向(roll, pitch, yaw)，相对于世界坐标系。 |
| params | 可选参数, 一个Python字典对象。 如果一个指定的键是一个Entity属性，他的值会用来初始化这个Entity实体的属性。 |

返回：

- 新实体。

<a id="debugTracing"></a>

### def debugTracing():

功能说明：
输出当前KBEngine追踪的Python扩展对象计数器。
扩展对象包括：固定字典、固定数组、Entity、EntityCall...
在服务端正常关闭时如果计数器不为零，此时说明泄露已存在，日志将会输出错误信息。
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): FixedArray : leaked(128)
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): EntityCall : leaked(8)

<a id="delSpaceData"></a>

### def delSpaceData(spaceID, key):

功能说明：
删除指定key的space数据（如果space分割成多个部分，将进行同步删除）。
space数据由用户通过[setSpaceData](#setSpaceData)设置。

参数：

| spaceID | int32，空间的ID。 |
| --- | --- |
| key | string，一个字符串关键字。 |

<a id="delWatcher"></a>

### def delWatcher(path):

功能说明：
与调试监视系统交互，允许用户在脚本删除监视的变量。

参数：

| path | 要删除的变量的路径。 |
| --- | --- |

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
注意：这个函数依赖于Cellapps服务进程启动参数gus，请正确设置启动参数保持唯一性。
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

| res | string，如果存在返回资源的绝对路径，否则返回空。 |
| --- | --- |

返回：

- string，资源的绝对路径。

<a id="getSpaceData"></a>

### def getSpaceData(spaceID, key):

功能说明：
获取指定key的space数据。
space数据由用户通过[setSpaceData](#setSpaceData)设置。

参数：

| spaceID | int32，空间的ID。 |
| --- | --- |
| key | string，一个字符串关键字。 |

返回：

- string，指定key的字符串数据。

<a id="getSpaceGeometryMapping"></a>

### def getSpaceGeometryMapping(spaceID):

功能说明：
返回一个指定空间的几何映射名称。

参数：

| spaceID | 要查询的空间的id。 |
| --- | --- |

返回：

- string，几何映射名称。

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
返回服务端是否正在关闭中。在onBaseAppShuttingDown回调函数被调用后，这个函数返回True。

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

<a id="raycast"></a>

### def raycast(spaceID, layer, src, dst):

功能说明：
在指定的space中指定的layer中由源坐标向目的坐标射出一道射线，返回碰撞到的坐标点。
注意：space必须使用[addSpaceGeometryMapping](#addSpaceGeometryMapping)加载过几何数据。
下面是一个例子:

```python
	>>> KBEngine.raycast( spaceID, entity.layer, (0, 10, 0), (0,-10,0) )
	((0.0000, 0.0000, 0.0000), ( (0.0000, 0.0000, 0.0000),
	(4.0000, 0.0000, 0.0000), (4.0000, 0.0000, 4.0000)), 0)
```

参数：

| spaceID | int32, space的id。 |
| --- | --- |
| layer | int8，几何层。一个space可以同时加载多个navmesh数据，不同的navmesh在不同的layer中，不同的layer可被抽象成地面、水面等等。 |

返回：

- list， 碰撞到的坐标点列表。

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
1）重载脚本仅仅能在Cellapp上执行， 用户应该确保所有的服务端组件加载完成。
2）自定义类型在脚本重载后应该确保内存中已经实例化的对象也被更新，下面是一个例子:

```python
for e in 
KBEngine
.
entities
.values():
   if type( e ) is Avatar.Avatar:
      e.customData.__class__ = CustomClass
```

当这个方法完成时[KBEngine](./KBEngine.md).[onInit](./KBEngine.md#onInit)( True ) 被调用。

参数：

| fullReload | 可选的boolean参数，指定是否同时重新加载实体定义。如果这个参数为False，则实体定义不会被重新加载。默认为True。 |
| --- | --- |

返回：

- 重新加载成功返回True，否则返回False。

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

<a id="setSpaceData"></a>

### def setSpaceData(spaceID, key, value):

功能说明：
设置指定key的space数据。
space数据可以通过[getSpaceData](#getSpaceData)获取。

参数：

| spaceID | int32，空间的ID。 |
| --- | --- |
| key | string，一个字符串关键字。 |
| value | string，字符串值。 |

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

<a id="onCellAppData"></a>

### def onCellAppData(key, value):

功能说明：
当前 `CellApp` 在收到其他 `CellApp` 对 `KBEngine.cellAppData` 的广播，或在启动阶段从 `dbmgr` 回放已有键值时，如果某个顶层 `key` 被新增或覆盖，就会回调此函数。
当前进程自己直接执行 `KBEngine.cellAppData[key] = value` 时，不会额外再回调一次本函数。

注意：

- 该回调接口必须实现在入口模块(`kbengine_defaults.xml -> entryScriptFile`)中。
- 只有顶层 `key -> value` 的新增或整体改写会触发；`value` 内部可变对象的局部修改不会触发。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

参数：

| key | 被改变数据的键。 |
| --- | --- |
| value | 被改变数据的值。 |

<a id="onCellAppDataDel"></a>

### def onCellAppDataDel(key):

功能说明：
当前 `CellApp` 在收到其他 `CellApp` 对 `KBEngine.cellAppData` 的删除广播时，会在对应顶层 `key` 从本地字典移除后回调此函数。
当前进程自己直接执行 `del KBEngine.cellAppData[key]` 时，不会额外再回调一次本函数。

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
当前 `CellApp` 在收到其他 `BaseApp` 或 `CellApp` 对 `KBEngine.globalData` 的广播，或在启动阶段从 `dbmgr` 回放已有键值时，如果某个顶层 `key` 被新增或覆盖，就会回调此函数。
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
当前 `CellApp` 在收到其他 `BaseApp` 或 `CellApp` 对 `KBEngine.globalData` 的删除广播时，会在对应顶层 `key` 从本地字典移除后回调此函数。
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

<a id="onSpaceData"></a>

### def onSpaceData(spaceID, key, value):

功能说明：
当space数据有改变的时候被调用。
space数据由用户通过[setSpaceData](#setSpaceData)设置。

参数：

| spaceID | 空间的ID。 |
| --- | --- |
| key | 被改变数据的键。 |
| value | 被改变数据的值。 |

<a id="onSpaceGeometryLoaded"></a>

### def onSpaceGeometryLoaded(spaceID, mapping):

功能说明：
空间所需求的网格碰撞等数据加载完毕。
由用户通过[addSpaceGeometryMapping](#addSpaceGeometryMapping)设置。

参数：

| spaceID | 空间的ID。 |
| --- | --- |
| mapping | 网格碰撞数据的映射值。 |

<a id="onAllSpaceGeometryLoaded"></a>

### def onAllSpaceGeometryLoaded(spaceID, isBootstrap, mapping):

功能说明：
空间所需求的网格碰撞等数据全部加载完毕。
由用户通过[addSpaceGeometryMapping](#addSpaceGeometryMapping)设置。

参数：

| spaceID | 空间的ID。 |
| --- | --- |
| isBootstrap | 如果一个空间被分割由多个cell共同负载，那么isBootstrap描述的是是否为加载请求的发起cell。 |
| mapping | 网格碰撞数据的映射值。 |

<a id="onReadyForLogin"></a>

### def onReadyForLogin(isBootstrap):

功能说明：
当引擎启动并初始化完成后会一直调用此接口询问脚本层是否准备完毕，如果脚本层准备完毕则loginapp允许客户端登录。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| isBootstrap | bool，是否为第一个启动的Cellapp。 |
| --- | --- |

返回：

- 返回值大于等于1.0则脚本层准备完成，否则返回准备的进度值0.0~1.0。

## 属性文档

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
这个常量目前在Cellapp未被使用。

<a id="cellAppData"></a>

cellAppData

说明：
`CellApp` 进程级共享字典，只在所有 `CellApp` 之间同步。
脚本执行 `KBEngine.cellAppData[key] = value` 或 `del KBEngine.cellAppData[key]` 时，请求会先发往 `dbmgr` 持有的权威副本，再广播给其他 `CellApp`。
写入方本地值会在本次脚本赋值/删除流程中直接更新，不会再收到一次自己的回环广播；其他 `CellApp` 收到广播后，才会触发 [onCellAppData](./KBEngine.md#onCellAppData) 或 [onCellAppDataDel](./KBEngine.md#onCellAppDataDel)。
新启动的 `CellApp` 在登录 `dbmgr` 时，还会收到当前全量快照，不只是后续增量。
键和值都必须能在目标组件上完成 `pickle/unpickle`。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

例子：

```python
KBEngine.cellAppData["hello"] = "there"
```

其余 `CellApp` 可以读取：

```python
print(KBEngine.cellAppData["hello"])
```

参看：[KBEngine](./KBEngine.md).[onCellAppData](./KBEngine.md#onCellAppData) 和 [KBEngine](./KBEngine.md).[onCellAppDataDel](./KBEngine.md#onCellAppDataDel)。

注意：

- 同步粒度是顶层 `key -> value`；只改 `value` 内部元素不会广播。
- 它不会同步到 `BaseApp`。
- 不要做下面这种“局部改列表元素”的写法：

```python
KBEngine.cellAppData["list"] = [1, 2, 3]

KBEngine.cellAppData["list"][1] = 7
```

这样本地看到的是 `[1, 7, 3]`，其他 `CellApp` 仍然是 `[1, 2, 3]`。

<a id="component"></a>

component

说明：
这是正运行在当前Python环境的组件。（至今为止）可能值有'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' 和 'editor'。

<a id="entities"></a>

entities

说明：
entities是一个字典对象，包含当前进程上所有的实体，包括ghost实体。
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

<a id="globalData"></a>

globalData

说明：
`BaseApp` 与 `CellApp` 共享的进程级字典。
脚本执行 `KBEngine.globalData[key] = value` 或 `del KBEngine.globalData[key]` 时，请求会先进入 `dbmgr` 持有的权威副本，再由 `dbmgr` 广播给所有其他 `BaseApp` 和 `CellApp`。
这里的权威方是 `dbmgr`，不是 `CellAppMgr`。
写入方本地不会收到一遍自己的同步回调；其他组件在完成反序列化和本地落地后，才会触发 [onGlobalData](./KBEngine.md#onGlobalData) 或 [onGlobalDataDel](./KBEngine.md#onGlobalDataDel)。
新启动的 `BaseApp` 或 `CellApp` 也会在登录 `dbmgr` 后收到当前全量快照。
键和值都必须能在目标组件上完成 `pickle/unpickle`。

源码解析：

- [网络与消息系统：`baseAppData` / `globalData` / `cellAppData` 的同步链与回调分发](/architecture/source-analysis/networking.html#global-data-dicts-sync)

例子：

```python
KBEngine.globalData["hello"] = "there"
```

其余 `CellApp` 或 `BaseApp` 可以读取：

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
