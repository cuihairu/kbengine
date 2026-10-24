# KBEngine module

ThisKBEnginemodule provides the parts of the
logical script layer access toEntity,
as well as the data of other clients in the current process, and so on.

## Classes

- Entity
- PyClientApp

## Member functions

- def [addBots](#addBots)( reqCreateAndLoginTotalCount, reqCreateAndLoginTickCount=0, reqCreateAndLoginTickTime=0 ):
- def [callback](#callback)( initialOffset, callbackObj ):
- def [cancelCallback](#cancelCallback)( id ):
- def [genUUID64](#genUUID64)( ):
- def [getWatcher](#getWatcher)( path ):
- def [getWatcherDir](#getWatcherDir)( path ):
- def [scriptLogType](#scriptLogType)( logType ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## Callbacks

- def [onInit](#onInit)( isReload ):
- def [onFinish](#onFinish)( ):

## Attributes

| [bots](#bots) | bots[component](#component)Read-onlystring |
| --- | --- |

## Member functions documentation

<a id="addBots"></a>

### def addBots(reqCreateAndLoginTotalCount, reqCreateAndLoginTickCount=0, reqCreateAndLoginTickTime=0):

Function description:
Add a bot to the server.
Example:

```python
# Here is an example of using addBots
        import 
KBEngine

 
        # Add 5 robots to the server at one time (instantaneously).
        KBEngine.addBots( 5 )
 
        # Add a total of 1000 robots to the server, 5 at a time, at an interval of 10 seconds.
        KBEngine.addBots( 1000, 5, 10 )
```

parameters:

| reqCreateAndLoginTotalCount | integer, integer, the total number of bots to add to the server. |
| --- | --- |
| reqCreateAndLoginTickCount | integer, the number of bots added to the server each interval |
| reqCreateAndLoginTickTime | integer, the interval of time (in seconds) between adding bots. |

<a id="callback"></a>

### def callback(initialOffset, callbackObj):

Function description:
Registers a callback, on the callbackObj function, which will be executed
once after "initialOffset" seconds.
Example:

```python
# Here is an example of using callback
        import 
KBEngine

 
        # Add a timer and execute it after 1 second
        KBEngine.callback( 1, onCallbackfun )
 
    def onCallbackfun( ):
        print "onCallbackfun called"
	
```

parameters:

| initialOffset | float, time, in seconds, to wait before triggering the callback. |
| --- | --- |
| callbackObj | function, the specified callback function object. |

returns:

- integer, this function returns the internal id of the callback. This id can be used with[cancelCallback](#cancelCallback)to remove the callback.

<a id="cancelCallback"></a>

### def cancelCallback(id):

Function description:
This function is used to remove a registered but not yet triggered callback.
The removed callback will not be executed. If this function is passed an
invalid id (for example, timer was removed), an error will be generated.
A use case is in the KBEngine.[callback](#callback)example.

parameters:

| id | integer, specifies the callback id to remove. |
| --- | --- |

<a id="genUUID64"></a>

### def genUUID64():

Function description:
This function generates a 64-bit unique ID.
Note: This function is dependent on the startup argument 'gus' of the
Cellapps service process. Please set the startup arguments to be unique.
In addition, if gus exceeds 65535, the function can only remain unique on
the current process.
Usage:
Unique item IDs are generated on multiple service processes and do not
conflict when combined.
A room ID is generated on multiple service process and no uniqueness
verification is required.

returns:

- A 64-bit integer.

<a id="getWatcher"></a>

### def getWatcher(path):

Function description:
Gets the value of a watch variable from the KBEngine debug system.
Example: In the Python console of baseapp1 enter:
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
Example: In the Python console of baseapp1 enter::
>>>KBEngine.getWatcher("/root")
('stats', 'objectPools', 'network', 'syspaths', 'ThreadPool', 'cprofiles', 'scripts', 'numProxices', 'componentID', 'componentType', 'uid', 'numClients', 'globalOrder', 'username', 'load', 'gametime', 'entitiesSize', 'groupOrder')

parameters:

| path | string, the absolute path of the variable including the variable name (can be viewed on the GUIConsole watcher page). |
| --- | --- |

returns:

- A list of elements in the directory (directories, variable names).

<a id="scriptLogType"></a>

### def scriptLogType(logType):

Function description:
Sets the type of information output by the current Python.print
(Reference: KBEngine.LOG_TYPE_*)

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

<a id="onInit"></a>

### def onInit(isReload):

Function description:
This function is called after all scripts have been initialized since
the engine started.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

parameters:

| isReload | bool, whether it was triggered after rewriting the loading script. |
| --- | --- |

<a id="onFinish"></a>

### def onFinish():

Function description:
This function is called back when the process shuts down.
Note: This callback function must be implemented in the portal module (kbengine_defaults.xml->entryScriptFile).

## Attributes documentation

<a id="bots"></a>

bots

Description:
bots is a dictionary object that contains all client objects on the current
process.

Types:

- PyBots

<a id="component"></a>

component

Description:
This is the component that is running in the current scripting environment.
(So far) Possible values are 'cellapp', 'baseapp', 'client', 'dbmgr', 'bots',
and 'editor'.
