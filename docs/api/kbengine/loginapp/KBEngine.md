# KBEngine module

ThisKBEnginemodule provides Python scripts
control over the loginapp process to handle entity login registration.

## Member functions

- def [addTimer](#addTimer)( initialOffset, repeatOffset=0, callbackObj=None ):
- def [delTimer](#delTimer)( id ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## Callback functions

- def [onLoginAppReady](#onLoginAppReady)( ):
- def [onLoginAppShutDown](#onLoginAppShutDown)( ):
- def [onRequestLogin](#onRequestLogin)( loginName, password, clientType, datas ):
- def [onLoginCallbackFromDB](#onLoginCallbackFromDB)( loginName, accountName, errorno, datas ):
- def [onRequestCreateAccount](#onRequestCreateAccount)( accountName, password, datas ):
- def [onCreateAccountCallbackFromDB](#onCreateAccountCallbackFromDB)( accountName, errorno, datas ):

## Member functions documentation

<a id="addTimer"></a>

### def addTimer(initialOffset, repeatOffset=0, callbackObj=None):

Function description:
Register a timer. The timer is triggered by the callback function
callbackObj. The callback function will be executed the first time
after "initialOffset" seconds, and then will be executed once every
"repeatOffset" seconds.
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

- integer, the internal id of the timer. This id can be used to remove the timer using[delTimer](#delTimer)

<a id="delTimer"></a>

### def delTimer(id):

Function description:
The function delTimer is used to remove a registered timer. The removed
timer is no longer executed. Single-shot timers are automatically removed
after the callback is executed, and it is not necessary to use delTimer
to remove it. If the delTimer function uses an invalid id (for example,
has been removed), it will generate an error
A use case for the KBEngine.addTimer reference timer.

parameters:

| id | integer, timer id to remove |
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

<a id="onLoginAppReady"></a>

### def onLoginAppReady():

Function description:
This function is called back when the current process is ready.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onLoginAppShutDown"></a>

### def onLoginAppShutDown():

Function description:
Process shutdown calls this function back.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onRequestLogin"></a>

### def onRequestLogin(loginName, password, clientType, datas):

Function description:
Called back when the client requests the server login account.
Here you can do some administrative control on user login. For example:
Use this interface to truncate the user's login here, record the request
and queue it, and return an error code to tell the client the queue
status.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| loginName | string, the name of the account submitted when logging in. |
| --- | --- |
| password | string, MD5 password. |
| clientType | integer, client type, given when the client logs in. |
| datas | bytes, the data attached to the client request, can forward data to a third-party platform. |

returns:

- Tuple, the return value is (error code, real account name, password, client type, data data submitted by the client), if there is no need to extend the modification, the return value is usually to destroy the incoming value (KBEngine.SERVER_SUCCESS , loginName, password, clientType, datas).

<a id="onLoginAppReady"></a>

### def onLoginAppReady():

Function description:
This function is called back when the current process is ready.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onLoginAppShutDown"></a>

### def onLoginAppShutDown():

Function description:
Process shutdown calls this function back.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onLoginCallbackFromDB"></a>

### def onLoginCallbackFromDB(loginName, accountName, errorno, datas):

Function description:
The callback returned by dbmgr after the client requests the server
login account.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| loginName | string, the name of the account submitted when logging in. |
| --- | --- |
| accountName | string, the real account name (obtained from the the query at dbmgr) |
| errorno | integer, error code, if it is not KBEngine.SERVER_SUCCESS, login failed. |
| datas | bytes, which may be any data, such as data returned by a third-party platform or data returned by dbmgr and interfaces when processing the login. |

<a id="onRequestCreateAccount"></a>

### def onRequestCreateAccount(accountName, password, data):

Function description:
Callback when the client requests the server to create an account.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| accountName | string, the name of the account submitted by the client. |
| --- | --- |
| password | string, MD5 password. |
| datas | bytes, the data attached to the client request, can forward data to a third-party platform. |

returns:

- Tuple, the return value is (error code, real account name, password, data data submitted by the client), if there is no need to extend the modified value is usually returned to destroy the incoming value (KBEngine.SERVER_SUCCESS, loginName, password , datas).

<a id="onCreateAccountCallbackFromDB"></a>

### def onCreateAccountCallbackFromDB(accountName, errorno, datas):

Function description:
The callback returned by dbmgr after the client requests the server to create an account.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| accountName | string, the name of the account submitted by the client. |
| --- | --- |
| errorno | integer, error code, if it is not KBEngine.SERVER_SUCCESS, login failed. |
| datas | bytes, which may be any data, such as data returned by a third-party platform or data returned by dbmgr and interfaces when processing the login. |
