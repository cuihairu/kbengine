# KBEngine module

ThisKBEnginemodule provides the Python script
access to theentity's base part, in particular
it provides the registration and removal of timers, as well as the creation
ofentities.

## Classes

- Entity
- Proxy

## Member functions

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

## Callback functions

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

## Attributes

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
| [component](#component) | Read-onlystring |
| [entities](#entities) | Entities |
| [baseAppData](#baseAppData) | GlobalDataClient |
| [globalData](#globalData) | GlobalDataClient |

## Member functions documentation

<a id="addWatcher"></a>

### def addWatcher(path, dataType, getFunction):

Function description:
Interacts with the debug monitoring system, allowing the user to register
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
The function countPlayers is called when the watcher observes.

parameters:

| path | Create a monitored path. |
| --- | --- |
| dataType | The value type of the monitor variable. Reference:Basic data types |
| getFunction | This function is called when the observer retrieves the variable. This function returns a value representing a watch variable without arguments. |

<a id="address"></a>

### def address():

Function description:
Returns the address of the internal network interface.

<a id="MemoryStream"></a>

### def MemoryStream():

Function description:
Returns a new MemoryStream object.
The MemoryStreamobject stores binary information. This type is provided
to allow the user to easily serialize and deserialize the Python base
types following KBEngine underlying serialization rules.
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

<a id="charge"></a>

### def charge(ordersID, dbID, byteDatas, pycallback):

Function description:
Billing interface.

parameters:

| ordersID | string, order ID. |
| --- | --- |
| dbID | uint64, thedatabaseIDof the entity. |
| byteDatas | bytes, with data, which is parsed and defined by the developer. |
| pycallback | Billing callback. Billing callback prototype: (When calling KBEngine.chargeResponse in interfaces, the callback is called if an order is set to callback) def on*ChargeCB(self, orderID, dbID, success, datas): ordersID: string, OrderID dbID: uint64, usually thedatabaseIDof the entity. success: bool, whether the order succeeded datas: bytes, with data, parsed and defined by the developer. |

<a id="createEntity"></a>

### def createEntity():

Function description:
KBEngine.createEntityLocallyalias.

<a id="createEntityAnywhere"></a>

### def createEntityAnywhere(entityType, params, callback):

Function description:
Create a new baseEntity. The server
can choose anyBaseappto create anEntity.
This method should be preferred overKBEngine.createEntityLocallyso the server has the flexibility to choose a suitableBaseappto create an entity.
The function parameters need to provide the type of entity created, and there is
also a Python dictionary as a parameter to initialize the entities value.
The Python dictionary does not require the user to provide all of the
properties, and the default values provided by the entity definition file
".def" are defaults.
Example:

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

parameters:

| entityType | string, specifies the type ofEntityto create. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| params | optional parameter, a Python dictionary object. If a specified key is anEntityattribute, its value will be used to initialize the properties of thisEntity. If the key is aCellattribute, it will be added to the 'cellData' attribute of theEntity. ThiscellData' attribute is a Python dictionary and will be used later to initialize the attributes of the cell entity. |
| callback | An optional callback function that is called when the entity is created. The callback function takes one argument, when theEntityis created successfully it is the entity'sentityCall, on failure it is None. |

returns:

- Returns theentityCallof theEntitythrough the callback.

<a id="createEntityRemotely"></a>

### def createEntityRemotely(entityType, baseMB, params, callback):

Function description:
Create a newEntityon the specified
baseapp through the baseMB parameter.
KBEngine.createEntityAnywhereshould be preferred over this method to allow the server to decide which is the most suitableBaseappto create the entity on for load balancing purposes.
The function parameters need to provide the type of the created entity,
and there is also a Python dictionary as a parameter to initialize the
entity's value.
This Python dictionary does not require the user to provide all of the
properties, and the default values provided by the entity definition
file ". def" are defaults.
Example:

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

parameters:

| entityType | string, specifies the type ofEntityto create. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| baseMB | BaseEntityCall which is a baseEntityEntityCall. The entity will be created on the baseapp process corresponding to this entity. |
| params | Optional parameters, a Python dictionary object. If a specified key is anEntityattribute, its value will be used to initialize the properties of thisEntity. If this key is aCellattribute, it will be added to theEntity's 'cellData' attribute. This 'cellData' attribute is a Python dictionary and will be used later to initialize the attributes of the cell entity. |
| callback | An optional callback function that is called when the entity is created. The callback takes one argument, on success it is anEntityʵ���entityCall, on failure it is None. |

returns:

- Returns theEntity'sentityCallthrough the callback.

<a id="createEntityFromDBID"></a>

### def createEntityFromDBID(entityType, dbID, callback, dbInterfaceName):

Function description:
Create anEntityby loading data
from the database. The newEntitywill
be created on theBaseappthat called this function.
If theEntityhas been checked out
from the database, a reference to this existing entity will be returned.

parameters:

| entityType | string, specifies theEntitytype to load. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| dbID | Specifies the database ID of the entity to create. The database ID of this entity is stored in the entity'sdatabaseIDattribute. |
| callback | An optional callback function that is called when the operation is complete. The callback function has three parameters: baseRef, databaseID, and wasActive. If the operation is successful, baseRef will be anentityCallor a direct reference to the newly createdEntity. The databaseID will be the database ID of the entity. wasActive will be True if baseRef is a reference to an already existing entity (checked out from the database). If the operation fails the three parameters will be baseRef - None, databaseID - 0, wasActive - False. The most common reason for failure is that the entity does not exist in the database, but occasionally other errors such as timeouts or ID allocation failure. |
| dbInterfaceName | string, optional parameter, specified by a database interface, and the "default" interface is used by default. Database interfaces are defined in kbengine_defaults.xml->dbmgr->databaseInterfaces. |

<a id="createEntityAnywhereFromDBID"></a>

### def createEntityAnywhereFromDBID(entityType, dbID, callback, dbInterfaceName):

Function description:
Create anEntityby loading data
from the database. The server may choose anyBaseappto create theEntity.
Using this function will help BaseApps load balance.
If the entity has been checked out from the database, a reference to the
existingEntitywill be returned.

parameters:

| entityType | string, specifies theEntitytype to load. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| dbID | Specifies the database ID of the entity to create. The database ID of this entity is stored in the entity'sdatabaseIDattribute. |
| callback | An optional callback function that is called when the operation is complete. The callback function has three parameters: baseRef, databaseID, and wasActive. If the operation is successful, baseRef will be anentityCallor a direct reference to the newly createdEntity. The databaseID will be the database ID of the entity. wasActive will be True if baseRef is a reference to an already existing entity (checked out from the database). If the operation fails the three parameters will be baseRef - None, databaseID - 0, wasActive - False. The most common reason for failure is that the entity does not exist in the database, but occasionally other errors such as timeouts or ID allocation failure. |
| dbInterfaceName | string, optional parameter, specified by a database interface, and the "default" interface is used by default. Database interfaces are defined in kbengine_defaults.xml->dbmgr->databaseInterfaces. |

returns:

- TheEntity'sentityCallthrough the callback.

<a id="createEntityRemotelyFromDBID"></a>

### def createEntityRemotelyFromDBID(entityType, dbID, baseMB, callback, dbInterfaceName):

Function description:
Load data from the database and create anEntityon the baseapp specified via the baseMB parameter.
If the entity has been checked out from the database, a reference to the
existingEntitywill be returned.

parameters:

| entityType | string, specifies theEntitytype to load. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| dbID | Specifies the database ID of the entity to create. The database ID of this entity is stored in the entity'sdatabaseIDattribute. |
| callback | An optional callback function that is called when the operation is complete. The callback function has three parameters: baseRef, databaseID, and wasActive. If the operation is successful, baseRef will be anentityCallor a direct reference to the newly createdEntity. The databaseID will be the database ID of the entity. wasActive will be True if baseRef is a reference to an already existing entity (checked out from the database). If the operation fails the three parameters will be baseRef - None, databaseID - 0, wasActive - False. The most common reason for failure is that the entity does not exist in the database, but occasionally other errors such as timeouts or ID allocation failure. |
| dbInterfaceName | string, optional parameter, specified by a database interface, and the "default" interface is used by default. Database interfaces are defined in kbengine_defaults.xml->dbmgr->databaseInterfaces. |

returns:

- Returns theEntity'sentityCallthrough the callback.

<a id="createEntityLocally"></a>

### def createEntityLocally(entityType, params):

Function description:
Create a newEntity.
The function parameters need to provide the type of the created entity,
and there is also an optional Python dictionary as paramater to initialize
the entity's values.
The Python dictionary does not require the user to provide all of the properties, and the default values provided by the entity definition file ".def" are defaults.
KBEngine.createEntityAnywhereshould be preferred over this method to allow the server to decide which is the most suitableBaseappto create the entity on for load balancing purposes.
It should be noted that this method returns the entity instantly without
a callback, and is also guaranteed to return a direct reference to theEntityobject,
rather than itsEntityCall.
It is suitable to use this method overKBEngine.createEntityAnywherewhen you need to manage the entities life cycle (such as control when
destroy is called on the entity) or access the entities attributes from
the creating entity, because as described in theEntityCalldocumentation, it is not possible to access attributes or call methods not
listed in the entity's def file using theEntityCall.
This method is also necessary to use when you need a direct reference to an entity
(as it's not possible to get one on a different baseapp). Many functions take
an EntityCall as a parameter, but some require a direct reference to the
entity (such asProxy.giveClientTo).
Example:

```python
params = {
	"name" : "kbe", # base, BASE_AND_CLIENT
	"HP" : 100,	# cell, ALL_CLIENT, in cellData
	"tmp" : "tmp"	# baseEntity.tmp
}

baseEntity = createEntityLocally("Avatar", params)
```

parameters:

| entityType | string, specifies the type of entity to create. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| params | optional parameter, a Python dictionary object. If a specified key is anEntityattribute, its value will be used to initialize the properties of thisEntity. If the key is aCellattribute, it will be added to the 'cellData' attribute of theEntity. ThiscellData' attribute is a Python dictionary and will be used later to initialize the attributes of the cell entity. |

returns:

- The newly createdEntity.

<a id="debugTracing"></a>

### def debugTracing():

Function description:
Outputs the Python extended object counter currently tracked by KBEngine.
Extended objects include: fixed dictionary, fixed array, Entity, EntityCall...
If the counter is not zero when the server is shut down normally, it means
that the leak already exists and the log will output an error message.
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): FixedArray : leaked(128)
ERROR cellapp [0x0000cd64] [2014-11-12 00:38:07,300] - PyGC::debugTracing(): EntityCall : leaked(8)

<a id="delWatcher"></a>

### def delWatcher(path):

Function description:
Interacts with the debug monitoring system, allowing users to delete
monitored variables in the script.

parameters:

| path | The path to the variable to delete. |
| --- | --- |

<a id="deleteEntityByDBID"></a>

### def deleteEntityByDBID(entityType, dbID, callback, dbInterfaceName):

Function description:
Deletes the specified entity (including the child table data generated
by the attribute) from the database. If the entity is not checked out
from the database, the deletion is successful. If the entity has been
checked out from the database,KBEnginewill fail to delete and return theEntity'sentityCallin the callback.

parameters:

| entityType | string, specifies the type of Entity to delete. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| dbID | Specifies the database ID of the entity to delete. The database ID of the entity is stored in the entity's databaseID attribute.databaseID���ԡ� |
| callback | An optional callback, with only one parameter. When the entity has not been checked out from the database it will be deleted successfully and the parameter will be True. If the entity has been checked out from the database then the parameter is theEntity'sentityCall. |
| dbInterfaceName | String, optional parameter, specifies a database interface. By default it uses the "default" interface. Database interfaces are defined by kbengine_defaults.xml->dbmgr->databaseInterfaces. |

<a id="deregisterReadFileDescriptor"></a>

### def deregisterReadFileDescriptor(fileDescriptor):

Function description:
Unregisters the callback registered withKBEngine.registerReadFileDescriptor.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | socket descriptor/file descriptor |
| --- | --- |

<a id="deregisterWriteFileDescriptor"></a>

### def deregisterWriteFileDescriptor(fileDescriptor):

Function description:
Unregisters the callback registered withKBEngine.registerWriteFileDescriptor.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | socket descriptor/file descriptor. |
| --- | --- |

<a id="executeRawDatabaseCommand"></a>

### def executeRawDatabaseCommand(command, callback, threadID, dbInterfaceName):

Function description:
This script function executes a database command on the database, which
is directly parsed by the relevant database.
Please note that using this function to modify entity data may not be
effective because if the entity has been checked out, the modified data
will still be archived by the entity and cause overwriting.
This function is strongly not recommended for reading or modifying entity data.

parameters:

| command | This database command will be different for different database configuration scenarios. For a MySQL database it is an SQL query. |
| --- | --- |
| callback | Optional parameter, callback object (for example, a function) called back with the command execution result. This callback has 4 parameters: result set, number of rows affected, auto value, error message. Example: def sqlcallback(result, rows, insertid, error): print(result, rows, insertid, error) As the above example shows, the result parameter corresponds to the "result set", and the result set parameter is a row List. Each line is a list of strings containing field values. If the command execution does not return a result set (for example, a DELETE command), or the command execution encounters an error, the result set is None. The rows parameter is the "number of rows affected", which is an integer indicating the number of rows affected by the command execution. This parameter is only relevant for commands that do not return results (such as DELETE). This parameter is None if there is a result set return or if there is an error in the command execution. The insertid is a long value, similar to an entity's databaseID. When successfully inserting data int a table with an auto long type field, it returns the data at the time of insertion. More information can be found in mysql's mysql_insert_id() method. In addition, this parameter is only meaningful when the database type is mysql. Error corresponds to the "error message", when the command execution encounters an error, this parameter is a string describing the error. This parameter is None when the command execution has not occurred. |
| threadID | int32, optional parameter, specifies a thread to process this command. Users can use this parameter to control the execution order of certain commands (dbmgr is multi-threaded). The default is not specified. If threadId is the ID of an entity, it will be added to the entity's archive queue and written by the thread one by one. |
| dbInterfaceName | string, optional parameter, specifies a database interface. By default it uses the "default" interface. Database interfaces are defined by kbengine_defaults.xml->dbmgr->databaseInterfaces. |

<a id="genUUID64"></a>

### def genUUID64():

Function description:
This function generates a 64-bit unique ID.
Note: This function depends on the baseapp server process startup parameter
'gus'. Please set the startup parameters to be unique.
In addition, if gus exceeds 65535, this function can only remain unique
for the current process.
Usage:
Unique IDs can be generated on multiple service processes and do not
conflict.
A room ID can be generated on multiple service processes and no
uniqueness verification is required.

returns:

- Returns a 64-bit integer.

<a id="getResFullPath"></a>

### def getResFullPath(res):

Function description:
Get the absolute path of a resource.
Note: Resource must be accessible underKBE_RES_PATH.

parameters:

| res | string, the relative path of the resource |
| --- | --- |

returns:

- string, if there is an absolute path to the given resource, otherwise returns null.

<a id="getWatcher"></a>

### def getWatcher(path):

Function description:
Gets the value of a watch variable from the KBEngine debugging system.
Example: In the Python console of baseapp1:
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
Example: In the Python console of baseapp1 enter:
>>>KBEngine.getWatcher("/root")
('stats', 'objectPools', 'network', 'syspaths', 'ThreadPool', 'cprofiles', 'scripts', 'numProxices', 'componentID', 'componentType', 'uid', 'numClients', 'globalOrder', 'username', 'load', 'gametime', 'entitiesSize', 'groupOrder')

parameters:

| path | string, the absolute path to this variable (can be viewd on the GUIConsole watcher page). |
| --- | --- |

returns:

- Monitors the list of elements in the directory (directory, variable name).

<a id="getAppFlags"></a>

### def getAppFlags():

Function description:
Get the flags of the current engine APP, Reference:KBEngine.setAppFlags.

returns:

- KBEngine.APP_FLAGS_*

<a id="hasRes"></a>

### def hasRes(res):

Function description:
Use this interface to determine if a relative path exists.
Note: Resource must be accessible underKBE_RES_PATH.
Example:
>>>KBEngine.hasRes("scripts/entities.xml")
True

parameters:

| res | string, the relative path of the resource |
| --- | --- |

returns:

- bool, True if relative path exists, otherwise False.

<a id="isShuttingDown"></a>

### def isShuttingDown():

Function description:
Returns whether the server is shutting down.
After theonBaseAppShutDown(state=0)is called, this function returns True.

returns:

- True if the server is shutting down, otherwise False.

<a id="listPathRes"></a>

### def listPathRes(path, extension):

Function description:
Get a list of resources in a resource directory
Note: Resources must be accessible underKBE_RES_PATH.
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

| res | string, the relative path of the resource directory |
| --- | --- |
| extension | string, optional parameter, file extension to filter by |

returns:

- Tuple, resource list.

<a id="lookUpEntityByDBID"></a>

### def lookUpEntityByDBID(entityType, dbID, callback, dbInterfaceName):

Function description:
Queries whether an entity is checked out of the database, and if the
entity has been checked out of the database,KBEnginewill return theEntity'sentityCallin the callback.

parameters:

| entityType | string, specifies the type of Entity to query. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| dbID | Specifies the database ID of theEntityto be queried. The database ID is stored in the entity'sdatabaseIDattribute. |
| callback | A callback with one parameter, True when the entity is not checked out from the database, if it is checked out then it is theEntity'sentityCall. False in any other case. |
| dbInterfaceName | string, optional parameter, specifies a database interface. Uses the "default" interface by default. Database interfaces are defined in kbengine_defaults.xml->dbmgr->databaseInterfaces. |

<a id="matchPath"></a>

### def matchPath(res):

Function description:
Get the absolute path of a resource from its relative path.
Note: Resources must be accessible underKBE_RES_PATH.
Examples:
>>>KBEngine.matchPath("scripts/entities.xml")
'/home/kbe/kbengine/demo/res/scripts/entities.xml'

parameters:

| res | string, the relative path of the resource (including its name). |
| --- | --- |

returns:

- string, the absolute path of the resource.

<a id="open"></a>

### def open(res, mode, encoding):

Function description:
Use this function to open resources with their relative paths.
Note: Resource must be accessible underKBE_RES_PATH.

parameters:

| res | string, the relative path of the resource. |
| --- | --- |
| mode | string, optional parameter, the default is 'r', file operation mode: r Open in only read mode, w Open in write mode, a Open in append mode (Start from EOF, create new file if necessary) r+ Open w+ in read/write mode Open in read/write mode (see w) a+ Open in read/write mode (See a) rb Opens wb in binary read mode Opens in binary write mode (see w) ab Opens in binary append mode (see a) rb+ Opens in binary read and write mode (see r+) wb+ Opens in binary read and write mode (see w+ ) ab+ opens in binary read/write mode (see a+) |
| encoding | string, optional parameter, the name of the encoding used to decode or encode the file, the default encoding is platform dependent. |

<a id="publish"></a>

### def publish():

Function description:
This function returns the server's current release mode.

returns:

- int8, 0: debug, 1: release, others can be customized.

<a id="quantumPassedPercent"></a>

### def quantumPassedPercent():

Function description:
Returns the percentage of the current tick that takes one clock cycle.

returns:

- Returns the percentage of the current tick that takes one clock cycle.

<a id="registerReadFileDescriptor"></a>

### def registerReadFileDescriptor(fileDescriptor, callback):

Function description:
Registers a callback function that is called when the file descriptor is
readable.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | socket descriptor/file descriptor. |
| --- | --- |
| callback | A callback function with the socket descriptor/file descriptor as its only parameter. |

<a id="registerWriteFileDescriptor"></a>

### def registerWriteFileDescriptor(fileDescriptor, callback):

Function description:
Registers a callback function that is called when the socket descriptor/file
descriptor is writable.
Example:
[http://www.kbengine.org/assets/other/py/Poller.py](http://www.kbengine.org/assets/other/py/Poller.py)

parameters:

| fileDescriptor | socket descriptor/file descriptor |
| --- | --- |
| callback | A callback function with the socket descriptor/file descriptor as its only parameter. |

<a id="reloadScript"></a>

### def reloadScript(fullReload):

Function description:
Reloads Python modules related to entity and custom data types. The
current entity's class is set to the newly loaded class. This method
should only be used for development mode and not for product mode. The
following points should be noted:
1) The overloaded script can only be executed onBaseapp,
and the user should ensure that all server components are loaded.
2) The custom type should ensure that the objects already instantiated in
memory are updated after the script is reloaded. Here is an example:

```python
for e in 
KBEngine
.
entities
.values():
   if type( e ) is Avatar.Avatar:
      e.customData.__class__ = CustomClass
```

When this mehod completes,KBEngine.onInit( True )
is called.

parameters:

| fullReload | bool, optional parameter that specifies whether to reload entity definitions at the same time. If this parameter is False, the entity definition will not be reloaded. The default is True. |
| --- | --- |

returns:

- True if the reload succeeds, otherwise False.

<a id="scriptLogType"></a>

### def scriptLogType(logType):

Function description:
Set the type of information output by the current Python.print (Reference: KBEngine.LOG_TYPE_*).

<a id="setAppFlags"></a>

### def setAppFlags(flags):

Function description:
Set the flags of the current engine APP.
KBEngine.APP_FLAGS_NONE // Default (not set)
KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING //Do not participate in load balancing
Example:
KBEngine.setAppFlags(KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING | KBEngine.APP_FLAGS_*)

<a id="time"></a>

### def time():

Function description:
This method returns the current game time (number of cycles).

returns:

- uint32, the time of the current game. This refers to the number of cycles. The period is affected by the frequency. The frequency is determined by the configuration filekbengine.xmlorkbengine_defaults.xml->gameUpdateHertz.

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

<a id="onBaseAppReady"></a>

### def onBaseAppReady(isBootstrap):

Function description:
This callback function is called when the currentBaseappprocess is ready.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| isBootstrap | bool, True if this is the firstBaseappstarted |
| --- | --- |

<a id="onBaseAppShutDown"></a>

### def onBaseAppShutDown(state):

Function description:
TheBaseappshutdown procedure will call
this function.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| state | If state is 0, it means that it is before all clients are disconnected, if state is 1, it means that it is before all entities are written to the database, if state is 2, it mean all entities have been written to the database. |
| --- | --- |

<a id="onCellAppDeath"></a>

### def onCellAppDeath(addr):

Function description:
This callback function will be called on the death of a cellapp.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| addr | Dead cellapp address. tuple:(ip, port) Network byte order |
| --- | --- |

<a id="onFini"></a>

### def onFini():

Function description:
This callback function is called after the engine is officially shut down.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

<a id="onBaseAppData"></a>

### def onBaseAppData(key, value):

Function description:
This function is called back when KBEngine.baseAppData changes.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | The key of the changed data. |
| --- | --- |
| value | The value of the changed data. |

<a id="onBaseAppDataDel"></a>

### def onBaseAppDataDel(key):

Function description:
This function is called back when KBEngine.baseAppData is deleted.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | Deleted data key |
| --- | --- |

<a id="onGlobalData"></a>

### def onGlobalData(key, value):

Function description:
This function is called back when KBEngine.globalData changes.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | The key of the changed data |
| --- | --- |
| value | The value of the changed data |

<a id="onGlobalDataDel"></a>

### def onGlobalDataDel(key):

Function description:
This function is called back when KBEngine.globalData is deleted.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| key | Deleted data key. |
| --- | --- |

<a id="onInit"></a>

### def onInit(isReload):

Function description:
This function is called back after all scripts have been initialized after
the engine started.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| isReload | bool, whether it was triggered after rewriting the loading script. |
| --- | --- |

<a id="onLoseChargeCB"></a>

### def onLoseChargeCB(orderID, dbID, success, datas):

Function description:
This function is called back whenKBEngine.chargeResponseis called in
and the order is lost or unknown.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| ordersID | string, order ID. |
| --- | --- |
| dbID | uint64, the database ID of the entity, see:Entity.databaseID. |
| success | bool, is it successful? |
| datas | bytes, with information |

<a id="onReadyForLogin"></a>

### def onReadyForLogin(isBootstrap):

Function description:
When the engine is started and initialized, it will always call this function
to ask whether the script layer is ready. If the script layer is ready,
loginapp allows the client to log in.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| isBootstrap | bool, True if this is the firstBaseappstarted. |
| --- | --- |

returns:

- If the return value is greater than or equal to 1.0, the script layer is ready; otherwise, return a value from 0 to less than 1.0.

<a id="onReadyForShutDown"></a>

### def onReadyForShutDown():

Function description:
If this callback function is implemented in the script, it is called when
the process is ready to exit.
You can use this callback to control when the process exits.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

returns:

- bool if it returns True, it allows the process to exit. Returning other values will cause the process to ask again after a period of time.

<a id="onAutoLoadEntityCreate"></a>

### def onAutoLoadEntityCreate(entityType, dbID):

Function description:
Called when an automatically loaded entity is created. If the script layer
implements this callback, the entity is created by the script layer,
otherwise the engine defaults to create the entity using
createEntityAnywhereFromDBID.
This callback is envoked becauseEntity.writeToDBwas set to automatically load the entity.
Note: this callback takes precedence over onBaseAppReady execution
and can be checked for onBaseAppReady when the entity is loaded.

parameters:

| entityType | string, specifies the type of entity to query. Valid entity types are listed in/scripts/entities.xml. |
| --- | --- |
| dbID | specifies the database ID of theEntityto be queried. The database ID of this entity is stored in the entity'sdatabaseIDattribute. |

## Attributes documentation

<a id="LOG_ON_ACCEPT"></a>

LOG_ON_ACCEPT

Description:
This constant is returned byProxy.onLogOnAttempt,
and means that the new client is allowed to bind to aProxy entity.
If theProxy entityalready has a
client binding, the previous client will be kicked out.

<a id="LOG_ON_REJECT"></a>

LOG_ON_REJECT

Description:
This constant is returned byProxy.onLogOnAttempt,
which means that the current client is bound to theProxy entity.

<a id="LOG_ON_WAIT_FOR_DESTROY"></a>

LOG_ON_WAIT_FOR_DESTROY

Description:
This constant is returned byProxy.onLogOnAttempt.
The current requesting client will wait until theProxy entityis completely destroyed and the underlying layer will complete the
subsequent binding process. Before this returns,Proxy.destroyorProxy.destroyCellEntityshould be invoked.

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
This constant is used for theEntity.shouldAutoBackupandEntity.shouldAutoArchiveattributes and means that the entity is backed up automatically next time
it is deemed acceptable, and then the attribute is automatically set to false (0).

<a id="component"></a>

component

Description:
This is the component that is running in the current Python environment.
(So far) Possible values are 'cellapp', 'baseapp', 'client', 'dbmgr', 'bots', and 'editor'.

<a id="entities"></a>

entities

Description:
entities is a dictionary object that contains all the entities in the
current process.
Debugging leaked entities (instances that call destroy without releasing
memory, usually due to being referenced):

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

<a id="baseAppData"></a>

baseAppData

Description:
This attribute contains a dictionary-like object that is automatically
synchronized across all BaseApps. When a value in the dictionary is
modified, the change is broadcast to all BaseApps.
Example:

```python
KBEngine
.
baseAppData
[ "hello" ] = "there"
```

The rest of the BaseApps can access the following:

```python
print 
KBEngine
.
baseAppData
[ "hello" ]
```

Keys and values can be of any type, but these types must be encapsulated
and unpacked on all target components.
When a value is changed or deleted, a callback function is called on all
components.
See:KBEngine.onBaseAppDataandKBEngine.onDelBaseAppData.
Note: Only top-level value changes will be broadcast. If you have a value
(such as a list) that changes an internal value (such as just changing a
number), this information will not be broadcast.
Do not do the following:

```python
KBEngine
.
baseAppData
[ "list" ] = [1, 2, 3]

KBEngine
.
baseAppData
[ "list" ][1] = 7
```

The local access is [1, 7, 3] and the remote access is [1, 2, 3].

<a id="globalData"></a>

globalData

Description:
This attribute contains a dictionary-like object that is automatically
synchronized across all BaseApps and CellApps. When a value in the dictionary is
modified, the change is broadcast to all BaseApps and CellApps.
example:

```python
KBEngine
.
globalData
[ "hello" ] = "there"
```

The otherBaseappsandCellappscan access the following:

```python
print 
KBEngine
.
globalData
[ "hello" ]
```

Keys and values can be of any type, but these types must be encapsulated
and unpacked on all target components.
When a value is changed or deleted, a callback function is called on all
components.
See:KBEngine.onGlobalDataandKBEngine.onGlobalDataDel.
Note: Only top-level value changes will be broadcast. If you have a value
(such as a list) that changes an internal value (such as just changing a
number), this information will not be broadcast.
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

The local access is [1, 7, 3] and the remote access is [1, 2, 3].
