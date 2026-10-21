# KBEngine module

ThisKBEnginemodule provides
Python scripts the ability to control, analyze, and dump certain types
of logs.

## Member functions

- def [addTimer](#addTimer)( initialOffset, repeatOffset=0, callbackObj=None ):
- def [delTimer](#delTimer)( id ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## Callbacks

- def [onLoggerAppReady](#onLoggerAppReady)( ):
- def [onLoggerAppShutDown](#onLoggerAppShutDown)( ):
- def [onLogWrote](#onLogWrote)( datas ):
- def [onReadyForShutDown](#onReadyForShutDown)( ):

## Member functions documentation

<a id="addTimer"></a>

### def addTimer(initialOffset, repeatOffset=0, callbackObj=None):

Function description:
Registers a timer. The timer triggers the callback function callbackObj.
The callback function will be executed the first time after "initialOffset"
seconds, and then will be executed once every "repeatOffset" seconds.
Example:

```python
# Here is an example of using addTimer
        import 
KBEngine

 
        # Add a timer, perform the first time after 5 seconds, and execute once every 1 second. The user parameter is 9
        KBEngine.addTimer( 5, 1, onTimer_Callbackfun )
 
        # Add a timer and execute it after 1 second. The default user parameter is 0.
        KBEngine.addTimer( 1, onTimer_Callbackfun )
 
    def onTimer_Callbackfun( id ):
        print "onTimer_Callbackfun called: id %i" % ( id )
        # If this is a repeated timer, it is no longer needed, call the following function to remove: 
        #     KBEngine.delTimer( id )
```

parameters:

| initialOffset | float, specifies the time interval in seconds for the timer to register from the first callback. |
| --- | --- |
| repeatOffset | float, specifies the time interval (in seconds) between each execution after the first callback execution. You must remove the timer with the function[delTimer](#delTimer), otherwise it will continue to repeat. Values less than or equal to 0 will be ignored. |
| callbackObj | function, the specified callback function object |

returns:

- integer, the internal id of the timer. This id can be used to remove the timer from [delTimer](#delTimer).

<a id="delTimer"></a>

### def delTimer(id):

Function description:
The delTimer function is used to remove a registered timer. The removed timer is
no longer executed. Single-shot timers are automatically removed after
the callback is executed, and it is not necessary to use the delTimer
function to remove it. If the delTimer function is passed an invalid id
(for example, timer was removed), it will generate an error.
A use case is shown in the KBEngine.[addTimer](#addTimer)example.

parameters:

| id | integer, which specifies the timer id to remove. |
| --- | --- |

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

<a id="onLoggerAppReady"></a>

### def onLoggerAppReady():

Function description:
This function is called back when the current process is ready.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onLoggerAppShutDown"></a>

### def onLoggerAppShutDown():

Function description:
This function is called back when the process shuts down.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onLogWrote"></a>

### def onLogWrote(datas):

Function description:
If this function is implemented in the script, it is invoked when the
logger process obtains a new log.
The database interface is defined inkbengine_defaults.xml->dbmgr->databaseInterfaces.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| datas | bytes, log data. |
| --- | --- |

<a id="onReadyForShutDown"></a>

### def onReadyForShutDown():

Function description:
If this function is implemented in the script, it is called when the
process is ready to exit.
You can use this callback to control when the process exits.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

returns:

- bool, if it returns True, it allows the process to exit. Returning other values will cause the process to ask again after a period of time.
