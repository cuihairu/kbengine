# KBEngine module

ThisKBEnginemodule provides
the Python script access to theentity's
cell part, in particular it provides the registration and removal of timers,
as well as the creation ofentities.

## Classes

- Entity

## Member functions

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

## Callback

- def [onCellAppData](#onCellAppData)( key, value ):
- def [onCellAppDataDel](#onCellAppDataDel)( key ):
- def [onGlobalData](#onGlobalData)( key, value ):
- def [onGlobalDataDel](#onGlobalDataDel)( key ):
- def [onInit](#onInit)( isReload ):
- def [onSpaceData](#onSpaceData)( spaceID, key, value ):
- def [onSpaceGeometryLoaded](#onSpaceGeometryLoaded)( spaceID, mapping ):
- def [onAllSpaceGeometryLoaded](#onAllSpaceGeometryLoaded)( spaceID, isBootstrap, mapping ):
- def [onReadyForLogin](#onReadyForLogin)( isBootstrap ):

## Attributes

| [LOG_TYPE_DBG](#LOG_TYPE_DBG) |  |
| --- | --- |
| [LOG_TYPE_ERR](#LOG_TYPE_ERR) |  |
| [LOG_TYPE_INFO](#LOG_TYPE_INFO) |  |
| [LOG_TYPE_NORMAL](#LOG_TYPE_NORMAL) |  |
| [LOG_TYPE_WAR](#LOG_TYPE_WAR) |  |
| [NEXT_ONLY](#NEXT_ONLY) |  |
| [cellAppData](#cellAppData) |  |
| [component](#component) | Read-onlystring |
| [entities](#entities) | Entities |
| [globalData](#globalData) | GlobalDataClient |

## Member functions documentation

<a id="addSpaceGeometryMapping"></a>

### def addSpaceGeometryMapping(spaceID, mapper, path, shouldLoadOnServer, params):

Function description:
Associate a geometric mapping of a given space. After the function is
called, the server and client will load the corresponding geometry data.
On the server, all geometry data is loaded from the given directory into
the specified space. These data may be divided into many blocks. Different
blocks are loaded asynchronously. The following callback methods are
called when all the geometry data is loaded:

```python
	def onAllSpaceGeometryLoaded( self, spaceID, mappingName ):
```

The server only loads the geometric data of the scene for use by the
navigation and collision functions. In addition to the geometric data,
the client also loads data such as textures.
3D scenes currently use the data exported by the recastnavigation
plugin-in by default. 2D scenes currently use the data exported by
MapEditor by default.
There is a possibility that onAllSpaceGeometryLoaded() will not be
invoked, that is, if multiple Cellapps call this method at the same time
to add geometry to the same space, cellappmgr crashes.

parameters:

| spaceID | uint32, ID of the space, specifies in which space to operate |
| --- | --- |
| mapper | Not yet implemented |
| path | Directory path containing geometry data |
| shouldLoadOnServer | Optional boolean parameter that specifies whether to load geometry on the server. Default is True. |
| params | Optional PyDict parameter, specifies the navmesh used by different layers, for example: KBEngine.addSpaceGeometryMapping(self.spaceID, None, resPath, True, {0 : "srv_xinshoucun_1.navmesh", 1 : "srv_xinshoucun.navmesh"}) |

<a id="addWatcher"></a>

### def addWatcher(path, dataType, getFunction):

Function description:
Interacts with the debug monitoring system to allow users to register
a monitoring variable with the monitoring system.
Example:

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

This function adds a watch variable under the "scripts/players" watch path.
The function countPlayers is called when the watcher observes a change.

parameters:

| path | The path to create a watcher. |
| --- | --- |
| dataType | The value type of the monitored variable. Reference:Basic data types |
| getFunction | This function is called when the observer retrieves the variable. This function returns a value representing a watch variable without arguments. |

<a id="address"></a>

### def address():

Function description:
Returns the address of the internal network interface.

<a id="MemoryStream"></a>

### def MemoryStream():

Function description:
Return a new MemoryStream object.
The MemoryStream object stores binary information. This type is provided
to allow the user to easily serialize and deserialize the Python base types
following the same KBEngine underlying serialization rules.
For example, you can use this object to construct a network packet that
KBEngine can parse.
Usage:

```python
>>> s = KBEngine.MemoryStream()
>>> s
>>> b''
>>> s.append("UINT32", 1)
>>> s.pop("UINT32")
>>> 1
```

The types that MemoryStream currently supports are only basic data types.
Reference:Basic data types

<a id="createEntity"></a>

### def createEntity(entityType, spaceID, position, direction, params):

Function description:
createEntity creates a new entity in the specified space of the current
process.
When calling this function you must specifiy the type, location, and direction
of the entity to be created. Optionally, any attribute of the entity can
be set with the params Python dictionary parameter. (the attributes are described in the entity's .def file).
Example:

```python
# Create an open Door entity in the same space as the "thing" entity
direction = ( 0, 0, thing.yaw )
properties = { "open":1 }

KBEngine
.
createEntity
( "Door", thing.space, thing.position, direction,
                       properties )
```

parameters:

| entityType | string, the name of the entity to create, declared in the/scripts/entities.xmlfile. |
| --- | --- |
| spaceID | int32, the ID of the space to place the entity |
| position | A sequence of 3 floats that specify the creation point of the new entity, in world coordinates. |
| direction | A sequence of 3 floats that specify the initial orientation (roll, pitch, yaw) of the new entity in world coordinates. |
| params | Optional parameters, a Python dictionary object. If a specified key is anEntityattribute, its value will be used to initialize the properties of the newEntity. |

returns:

- The newEntity.

<a id="debugTracing"></a>

### def debugTracing():

Function description:
Outputs the Python extension object counter that outputs KBEngine trace.
Extended objects include: fixed dictionary, fixed array, Entity, EntityCall...
If the counter is not zero when the server is shut down normally, it means
that the leak already exists and the log will output an error message.
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): FixedArray : leaked(128)
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): EntityCall : leaked(8)

<a id="delSpaceData"></a>

### def delSpaceData(spaceID, key):

Function description:
Deletes the space data of the specified key (if space is divided into
multiple parts, it will be deleted synchronously).
The space data is set by the user via[setSpaceData](#setSpaceData).

parameters:

| spaceID | int32, the ID of the space |
| --- | --- |
| key | string, a string keyword |

<a id="delWatcher"></a>

### def delWatcher(path):

Function description:
Interacts with the debug monitoring system, allowing users to delete
watcher variables in the script.

parameters:

| path | The path of the variable to delete. |
| --- | --- |

<a id="deregisterReadFileDescriptor"></a>

### def deregisterReadFileDescriptor(fileDescriptor):

Function description:
Deregisters the callback registered withKBEngine.registerReadFileDescriptor.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | Socket descriptor/file descriptor |
| --- | --- |

<a id="deregisterWriteFileDescriptor"></a>

### def deregisterWriteFileDescriptor(fileDescriptor):

Function description:
Deregisters the callback registered withKBEngine.registerWriteFileDescriptor.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | Socket descriptor/file descriptor |
| --- | --- |

<a id="executeRawDatabaseCommand"></a>

### def executeRawDatabaseCommand(command, callback, threadID, dbInterfaceName):

Function description:
This script function executes a database command on the database,
which will be directly parsed by the relevant database.
Please note that using this function to modify the entity data may not
be effective because if the entity has been checked out, the modified
entity data will still be archived by the entity and cause overwriting.
This function is strongly not recommended for reading or modifying entity data.

parameters:

| command | This database command will be different for different database configurations. For MySQL databases it will be an SQL query statement. |
| --- | --- |
| callback | Optional parameter, callbacks object (for example, a function) with the command's execution result. This callback has 4 parameters: result set, number of rows affected, auto value, and error message. Example: def sqlcallback(result, rows, insertid, error): print(result, rows, insertid, error) As the above example shows, the result parameter corresponds to the "result set", and the result set parameter is a row. List. Each line is a list of strings containing field values. The command execution does not return a result set (for example, a DELETE command), or the result set is None if the command execution has an error. The rows parameter is the "number of rows affected", which is an integer indicating the number of rows affected by the command execution. This parameter is only relevant for commands that do not return results (such as DELETE). This parameter is None if there is a result set return or if there is an error in the command execution. The insertid corresponds to a "long value", similar to the entity's databaseID. When successfully inserting data into a table with an auto long type field, it returns the data at the time of insertion. Assigned value. More information can be found in mysql's mysql_insert_id() method. In addition, this parameter is only meaningful when the database type is mysql. error corresponds to the "error message", when the command execution error, this parameter is a string describing the error. This parameter is None when the command execution has not occurred.threadIDint32, optional parameter, specifies a thread to process this command. Users can use this parameter to control the execution order of certain commands (dbmgr is multi-threaded). The default is not specified. If threadID is the ID of the entity, it will be added to the entity's archive queue and written by the thread one by one. dbInterfaceNamestring, optional parameter, specified by a database interface, defaults to "default" interface. The database interface is defined by kbengine_defaults.xml->dbmgr->databaseInterfaces. |

<a id="genUUID64"></a>

### def genUUID64():

Function description:
This function generates a 64-bit unique ID.
Note: This function is dependent on the 'gus' startup argument of the
Cellapp service process. Please set the startup arguments to be unique.
In addition, if gus exceeds 65535, this function can only remain unique
on the current process.
Usage
A unique item ID is generated on multiple service processes and there is
no conflict when combined.
A room ID is generated on multiple service processes and no uniqueness
verification is required.

returns:

- 64-bit integer

<a id="getResFullPath"></a>

### def getResFullPath(res):

Function description:
Get the absolute path of the resource.
Note: Resoures must be accessible underKBE_RES_PATH.

parameters:

| res | string, if there is an absolute path to return the resource, otherwise it returns null. |
| --- | --- |

returns:

- string, the absolute path to the resource.

<a id="getSpaceData"></a>

### def getSpaceData(spaceID, key):

Function description:
Get the space data of the specified key.
The space data is set by the user via[setSpaceData](#setSpaceData).

parameters:

| spaceID | int32, the ID of the space |
| --- | --- |
| key | string, a string keyword |

returns:

- string, string data for the given key

<a id="getSpaceGeometryMapping"></a>

### def getSpaceGeometryMapping(spaceID):

Function description:
Returns the geometry map name of a specified space.

parameters:

| spaceID | The ID of the space to be queried |
| --- | --- |

returns:

- string, the name of the geometry map.

<a id="getWatcher"></a>

### def getWatcher(path):

Function description:
Gets the value of a watch variable from the KBEngine debug system.
Example: In the baseapp1 Python console, enter:
>>>KBEngine.getWatcher("/root/stats/runningTime")
12673648533
>>>KBEngine.getWatcher("/root/scripts/players")
32133

parameters:

| path | string, the absolute path of the variable including the variable name (can be viewed on the GUIConsole watcher page). |
| --- | --- |

returns:

- The value of the variable.

<a id="getWatcherDir"></a>

### def getWatcherDir(path):

Function description:
Get a list of elements (directories, variable names) under the watch
directory from the KBEngine debugging system.
Example: In baseapp1 Python console, enter:
>>>KBEngine.getWatcher("/root")
('stats', 'objectPools', 'network', 'syspaths', 'ThreadPool', 'cprofiles', 'scripts', 'numProxices', 'componentID', 'componentType', 'uid', 'numClients', 'globalOrder', 'username', 'load', 'gametime', 'entitiesSize', 'groupOrder')

parameters:

| path | string, the absolute path of the variable including the variable name (can be viewed on the GUIConsole watcher page). |
| --- | --- |

returns:

- The list of elements (directory, variable name) under the Watch directory.

<a id="getAppFlags"></a>

### def getAppFlags():

Function description:
Get the flags of the current engine APP, Reference:KBEngine.setAppFlags��

returns:

- KBEngine.APP_FLAGS_*

<a id="hasRes"></a>

### def hasRes(res):

Function description:
Use this interface to determine if a relative path exists.
Note: Resources must be accessible underKBE_RES_PATH.
Example:
>>>KBEngine.hasRes("scripts/entities.xml")
True

parameters:

| res | string, the relative path of the resource |
| --- | --- |

returns:

- BOOL, if it exists return True, otherwise False.

<a id="isShuttingDown"></a>

### def isShuttingDown():

Function description:
Returns whether the server is shutting down. After the onBaseAppShuttingDown
callback function is called, this function returns True.

returns:

- BOOL, if the server is shutting down True, otherwise False.

<a id="listPathRes"></a>

### def listPathRes(path, extension):

Function description:
Get a list of resources in a resource directory
Note: Resources must be accesible underKBE_RES_PATH.
Example:
>>>KBEngine.listPathRes("scripts/cell/interfaces")
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/New Text Document.txt')
>>>KBEngine.listPathRes("scripts/cell/interfaces", "txt")
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/New Text Document.txt')
>>>KBEngine.listPathRes("scripts/cell/interfaces", "txt|py")
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/New Text Document.txt')
>>>KBEngine.listPathRes("scripts/cell/interfaces", ("txt", "py"))
('/home/kbe/kbengine/demo/res/scripts/cell/interfaces/AI.py', '/home/kbe/kbengine/demo/res/scripts/cell/interfaces/New Text Document.txt')

parameters:

| res | string, the relative path of the resource. |
| --- | --- |
| extension | string, optional parameter, file extension. |

returns:

- Tuple, resource list.

<a id="matchPath"></a>

### def matchPath(res):

Function description:
Use the relative path of the resource to get its absolute path.
Note: Resources must be accessible underKBE_RES_PATH.
Example:
>>>KBEngine.matchPath("scripts/entities.xml")
'/home/kbe/kbengine/demo/res/scripts/entities.xml'

parameters:

| res | string, the relative path to the resource (including the resource name) |
| --- | --- |

returns:

- string, the absolute path of the resource.

<a id="open"></a>

### def open(res, mode, encoding):

Function description:
Use this interface to open resources using relative paths.
Note: Resources must be accessible underKBE_RES_PATH.

parameters:

| res | string, the relative path of the resource. |
| --- | --- |
| mode | string, optional parameter, the default is 'r', file operation mode: r Open in only read mode, w Open in write mode, a Open in append mode (Start from EOF, create new file if necessary) r+ Open w+ in read/write mode Open in read/write mode (see w) a+ Open in read/write mode (See a) rb Opens wb in binary read mode Opens in binary write mode (see w) ab Opens in binary append mode (see a) rb+ Opens in binary read and write mode (see r+) wb+ Opens in binary read and write mode (see w+ ) ab+ Open in binary read/write mode (see a+) |
| encoding | string, optional parameter, the name of the encoding used to decode or encode the file, the default encoding is platform dependent. |

<a id="publish"></a>

### def publish():

Function description:
This interface returns the current server release mode.

returns:

- int8, 0: debug, 1: release, others can be customized.

<a id="raycast"></a>

### def raycast(spaceID, layer, src, dst):

Function description:
In the specified layer of the specified space, a ray is emitted from the
source coordinates to the destination coordinates, and the collided
coordinate point is returned.
Note: Space must load geometry using[addSpaceGeometryMapping](#addSpaceGeometryMapping).
Below is an example:

```python
	>>> KBEngine.raycast( spaceID, entity.layer, (0, 10, 0), (0,-10,0) )
	((0.0000, 0.0000, 0.0000), ( (0.0000, 0.0000, 0.0000),
	(4.0000, 0.0000, 0.0000), (4.0000, 0.0000, 4.0000)), 0)
```

parameters:

| spaceID | int32, space ID |
| --- | --- |
| layer | int8, geometric layer. A space can load multiple navmesh data at the same time. Different navmesh can be in different layers. Different layers can be abstracted into the ground, the water surface and so on. |

returns:

- list, list of coordinate points collided

<a id="registerReadFileDescriptor"></a>

### def registerReadFileDescriptor(fileDescriptor, callback):

Function description:
Registers a callback function that is called when the file descriptor is readable.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | Socket descriptor/file descriptor |
| --- | --- |
| callback | A callback function with the socket descriptor/file descriptor as its only parameter. |

<a id="registerWriteFileDescriptor"></a>

### def registerWriteFileDescriptor(fileDescriptor, callback):

Function description:
Registers a callback function that is called when the socket descriptor/file descriptor is writable.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | Socket descriptor/file descriptor |
| --- | --- |
| callback | A callback function with the socket descriptor/file descriptor as its only parameter. |

<a id="reloadScript"></a>

### def reloadScript(fullReload):

Function description:
Reloads Python modules related to entity and custom data types.
The current entity class is set to the newly loaded class.
This method should only be used for development mode and not for
product mode. The following points should be noted:
1) The overloaded script can only be executed onCellapp.
The user should ensure that all server components are loaded.
2) The custom type should ensure that the objects already instantiated
in memory are updated after the script is reloaded. Here is an example:

```python
for e in 
KBEngine
.
entities
.values():
   if type( e ) is Avatar.Avatar:
      e.customData.__class__ = CustomClass
```

When this method completesKBEngine.onInit( True )
is called.

parameters:

| fullReload | Optional boolean parameter that specifies whether to reload entity definitions at the same time. If this parameter is False, the entity definition will not be reloaded. The default is True. |
| --- | --- |

returns:

- True if the reload succeeds, False otherwise.

<a id="scriptLogType"></a>

### def scriptLogType(logType):

Function description:
Set the type of information output by the current Python.print (Reference: KBEngine.LOG_TYPE_*).

<a id="setAppFlags"></a>

### def setAppFlags(flags):

Function description:
Set the flags of the current engine APP.
KBEngine.APP_FLAGS_NONE // Default (not set)
KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING // Do not participate in load balancing
Example:
KBEngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING | KBEngine.APP_FLAGS_*)

<a id="setSpaceData"></a>

### def setSpaceData(spaceID, key, value):

Function description:
Sets the space data for the specified key.
The space data can be obtained via[getSpaceData](#getSpaceData).

parameters:

| spaceID | int32, the ID of the space. |
| --- | --- |
| key | string, a string keyword |
| value | string, the string value. |

<a id="time"></a>

### def time():

Function description:
This method returns the current game time (number of cycles).

returns:

- uint32, the current time of the game. This refers to the number of cycles. The period is affected by the frequency. The frequency is determined by the configuration filekbengine.xmlorkbengine_defaults.xml-> gameUpdateHertz.

<a id="urlopen"></a>

### def urlopen(url, callback, postData, headers):

Function description:
This script function is providing an external HTTP/HTTPS asynchronous request.

parameters:

| url | A valid HTTP/HTTPS URL. |
| --- | --- |
| callback | Optional parameter with a callback object (for example, a function) that requests execution results. This callback takes five parameters: the HTTP request return code (eg: 200), the returned content, the returned HTTP protocol header, whether it succeeded, and the requested URL. Example: def onHttpCallback(httpcode, data, headers, success, url): print(httpcode, data, headers, success, url) As the above example shows:&lt;br&gt;httpcode:The parameter corresponds to the "HTTP request return code", is an integer.&lt;br&gt;data:The parameter is “ returned content & rdquo;, it is a string.&lt;br&gt;headers:The parameter is the HTTP protocol header returned by the server, such as:{"Content-Type": "application/x-www-form-urlencoded"}, is an dict.&lt;br&gt;success:Whether the execution is successful or not, when the request execution has an error, it is False, and the error information can be further judged by httpcode.&lt;br&gt;url:Is the URL used by the request. |
| postData | Optional parameter, the default is GET mode request server. If you need POST mode, please provide the content that needs POST. The engine will automatically request the server using POST, is an bytes. |
| headers | Optional parameter, HTTP header used when requesting, such as：{"Content-Type": "application/x-www-form-urlencoded"}, is an dict. |

## Callback functions documentation

<a id="onCellAppData"></a>

### def onCellAppData(key, value):

Function description:
This function is called back when KBEngine.cellAppData changes.
Note: This callback interface must be implemented in the portal
module (kbengine_defaults.xml->entryScriptFile ).

parameters:

| key | The key of the changed data. |
| --- | --- |
| value | The value of the changed data. |

<a id="onCellAppDataDel"></a>

### def onCellAppDataDel(key):

Function description:
This function is called back when KBEngine.cellAppData is deleted.
Note: This callback interface must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | Deleted data key. |
| --- | --- |

<a id="onGlobalData"></a>

### def onGlobalData(key, value):

Function description:
This function is called back when KBEngine.globalData changes.
Note: This callback interface must be implemented in the portal moodule (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | The key of the changed data. |
| --- | --- |
| value | The value of the changed data. |

<a id="onGlobalDataDel"></a>

### def onGlobalDataDel(key):

Function description:
This function is called back when KBEngine.globalData is deleted.
Note: This callback interface must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | Deleted data key. |
| --- | --- |

<a id="onInit"></a>

### def onInit(isReload):

Function description:
This interface is called after all scripts have been initialized since
the engine started.
Note: This callback interface must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| isReload | Bool, whether it was triggered after reloading the loading the script. |
| --- | --- |

<a id="onSpaceData"></a>

### def onSpaceData(spaceID, key, value):

Function description:
Called when there is a change in the space data.
The space data is set by the user via[setSpaceData](#setSpaceData).

parameters:

| spaceID | The ID of the space. |
| --- | --- |
| key | The key of the changed data. |
| value | The value of the changed data. |

<a id="onSpaceGeometryLoaded"></a>

### def onSpaceGeometryLoaded(spaceID, mapping):

Function description:
The space required by the grid collision data is loaded.
Set by user through[addSpaceGeometryMapping](#addSpaceGeometryMapping).

parameters:

| spaceID | The ID of the space. |
| --- | --- |
| mapping | The map value of the grid collision data. |

<a id="onAllSpaceGeometryLoaded"></a>

### def onAllSpaceGeometryLoaded(spaceID, isBootstrap, mapping):

Function description:
The space required for grid collision and other data is completely loaded.
Set by user through[addSpaceGeometryMapping](#addSpaceGeometryMapping).

parameters:

| spaceID | The ID of the space. |
| --- | --- |
| isBootstrap | If a space is partitioned by multiple cells, isBootstrap describes whether it is the originating cell of the loading request. |
| mapping | The map value of grid collision data. |

<a id="onReadyForLogin"></a>

### def onReadyForLogin(isBootstrap):

Function description:
When the engine is started and initialized, it will always call this function
to ask whether the script layer is ready. If the script layer is ready,
loginapp allows the client to log in.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| isBootstrap | bool, True if this is the firstCellappstarted. |
| --- | --- |

returns:

- If the return value is greater than or equal to 1.0, the script layer is ready; otherwise, return a value from 0 to less than 1.0.

## Attribute documentation

<a id="LOG_TYPE_DBG"></a>

LOG_TYPE_DBG

Description:
The log output type is debug.
Set by[scriptLogType](#scriptLogType).

<a id="LOG_TYPE_ERR"></a>

LOG_TYPE_ERR

Description:
The log output type is error.
Set by[scriptLogType](#scriptLogType).

<a id="LOG_TYPE_INFO"></a>

LOG_TYPE_INFO

Description:
The log output type is general information.
Set by[scriptLogType](#scriptLogType).

<a id="LOG_TYPE_NORMAL"></a>

LOG_TYPE_NORMAL

Description:
The log output type is normal.
Set by[scriptLogType](#scriptLogType).

<a id="LOG_TYPE_WAR"></a>

LOG_TYPE_WAR

Description:
The log output type is warning.
Set by[scriptLogType](#scriptLogType).

<a id="NEXT_ONLY"></a>

NEXT_ONLY

Description:
This constant is currently unused in Cellap.

<a id="cellAppData"></a>

cellAppData

Description:
This property contains a dictionary-like object that is automatically
synchronized across all CellApps. When a value in the dictionary is
modified, this change is broadcast to all Cellapps.
Example:

```python
KBEngine
.
cellAppData
[ "hello" ] = "there"
```

The rest of Cellap can access the following:

```python
print 
KBEngine
.
cellAppData
[ "hello" ]
```

Keys and values can be of any type, but these types must be encapsulated
and unpacked on all target components.
When a value is changed or deleted, a callback function is called on all
components. See:KBEngine.onCellAppDataandKBEngine.onDelCellAppData.
Note: Only the top-level value will be broadcast. If you have a value
(such as a list) that changes the internal value (such as just changing
a number), this information will not be broadcast.
Do not do the following:

```python
KBEngine
.
cellAppData
[ "list" ] = [1, 2, 3]

KBEngine
.
cellAppData
[ "list" ][1] = 7
```

This will cause the local access to read [1, 7, 3] and the remote [1, 2, 3]

<a id="component"></a>

component

Description:
This is the component that is running in the current Python environment. (So far) Possible values are 'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' and 'editor'.

<a id="entities"></a>

entities

Description:
entities is a dictinary object that contains all entities in the current
process, includingghostentities.
Debugging leaked entities: (instances that call destroy without releasing
memory, usually as a result of the reference not being released)

```python
>>> KBEngine.entities.garbages.items()
[(1025, Avatar object at 0x7f92431ceae8.)]


>>> e = _[0][1]
>>> import gc
>>> gc.get_referents(e)
[{'spacesIsOk': True, 'bootstrapIdx': 1}, 
]
```

Debugging a leaked KBEngine-encapsulated Python object:
[KBEngine.debugTracing](#debugTracing)

Types:

- Entities

<a id="globalData"></a>

globalData

Description:
This attribute contains a dictionary-like object that is automatically
copied between all BaseApps and CellApps. When a value in a dictionary is
modified, this change is broadcast to all BaseApps and CellApps.
CellAppMgr solves completion conditions and ensures the authority of
information replication.
Example:

```python
KBEngine
.
globalData
[ "hello" ] = "there"
```

The rest ofCellapporBaseappcan access the following:

```python
print 
KBEngine
.
globalData
[ "hello" ]
```

Keys and values can be of any type, but these types must be encapsulated and unpacked on all target components.
When a value is changed or deleted, a callback function is called on all components.
See:KBEngine.onGlobalDataandKBEngine.onGlobalDataDel.
Note: Only the top-level value will be broadcast. If you have a volatile value (such as a list) that changes the internal value (such as just changing a number), this information will not be broadcast.
Do not do the following:

```python
KBEngine
.
globalData
[ "list" ] = [1, 2, 3]

KBEngine
.
globalData
[ "list" ][1] = 7
```

This will cause the local access to read [1, 7, 3] and the remote [1, 2, 3]
