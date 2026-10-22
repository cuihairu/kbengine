# KBEngine module

ThisKBEnginemodule mainly
handles access of third-party platforms for the KBEngine server.

## Member functions

- def [addTimer](#addTimer)( initialOffset, repeatOffset=0, callbackObj=None ):
- def [accountLoginResponse](#accountLoginResponse)( commitName, realAccountName, extraDatas, errorCode ):
- def [createAccountResponse](#createAccountResponse)( commitName, realAccountName, extraDatas, errorCode ):
- def [chargeResponse](#chargeResponse)( orderID, extraDatas, errorCode ):
- def [delTimer](#delTimer)( id ):
- def [executeRawDatabaseCommand](#executeRawDatabaseCommand)( command, callback, threadID, dbInterfaceName ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## Callbacks

- def [onInterfaceAppReady](#onInterfaceAppReady)( ):
- def [onInterfaceAppShutDown](#onInterfaceAppShutDown)( ):
- def [onRequestCreateAccount](#onRequestCreateAccount)( registerName, password, datas ):
- def [onRequestAccountLogin](#onRequestAccountLogin)( loginName, password, datas ):
- def [onRequestCharge](#onRequestCharge)( ordersID, entityDBID, datas ):

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

<a id="accountLoginResponse"></a>

### def accountLoginResponse(commitName, realAccountName, extraDatas, errorCode):

Function description:
After onRequestAccountLogin is called back, the script needs to call this
function to give the result of the login processing.

parameters:

| commitName | string, the name submitted by the client when requested. |
| --- | --- |
| realAccountName | string, returns the real account name (if there are no special requirements it is ually commitName, this is available when logging in with various alias accounts). |
| extraDatas | bytes, the data attached to the client's request. Can forward the data to a third-party platform and provide an opportunity to modify it. This parameter can be read in the script via the getClientDatas interface of the base entity. |
| errorCode | integer, error code. If you need to interrupt the user's behavior, you can set the error code here. The error code can be referenced (KBEngine.SERVER_ERROR_*, described in kbengine/kbe/res/server/server_errors.xml), otherwise submitting KBEngine.SERVER_SUCCESS represents permitting the login. |

<a id="createAccountResponse"></a>

### def createAccountResponse(commitName, realAccountName, extraDatas, errorCode):

Function description:
After onRequestCreateAccount is called back, the script needs to call
this function to give an account creation processing result.

parameters:

| commitName | string, the name submitted by the client when requested. |
| --- | --- |
| realAccountName | string, returns the real account name (if there are no special requirements it is ually commitName, this is available when logging in with various alias accounts). |
| extraDatas | bytes, the data attached to the client's request. Can forward the data to a third-party platform and provide an opportunity to modify it. This parameter can be read in the script via the getClientDatas interface of the base entity. |
| errorCode | integer, error code. If you need to interrupt the user's behavior, you can set the error code here. The error code can be referenced (KBEngine.SERVER_ERROR_*, described in kbengine/kbe/res/server/server_errors.xml), otherwise submitting KBEngine.SERVER_SUCCESS represents permitting the login. |

<a id="chargeResponse"></a>

### def chargeResponse(orderID, extraDatas, errorCode):

Function description:
After onRequestCharge is called back, the script needs to call this
function to give the billing result.

parameters:

| ordersID | string, the ID of the order |
| --- | --- |
| extraDatas | bytes, the data attached to the client's request. Can forward the data to a third-party platform and provide an opportunity to modify it. This parameter can be read in the script via the getClientDatas interface of the base entity. |
| errorCode | integer, error code. If you need to interrupt the user's behavior, you can set the error code here. The error code can be referenced (KBEngine.SERVER_ERROR_*, described in kbengine/kbe/res/server/server_errors.xml), otherwise submitting KBEngine.SERVER_SUCCESS represents permitting the login. |

<a id="delTimer"></a>

### def delTimer(id):

Function description:
The delTimer function is used to remove a registered timer. The removed timer is
no longer executed. Single-shot timers are automatically removed after
the callback is executed, and it is not necessary to use the delTimer
function to remove it. If the delTimer function is passed an invalid id
(for example, timer was removed), it will generate an error.
A use case is shown in theKBEngine.[addTimer](#addTimer)example.

parameters:

| id | integer, which specifies the timer id to remove. |
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

<a id="onInterfaceAppReady"></a>

### def onInterfaceAppReady():

Function description:
This function is called back when the current process is ready.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onInterfaceAppShutDown"></a>

### def onInterfaceAppShutDown():

Function description:
This function is called back when the process shuts down.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onRequestCreateAccount"></a>

### def onRequestCreateAccount(registerName, password, datas):

Function description:
This callback is called when the client requests the server to create
an account.
The data can be checked and modified within this function, and the final
result is submitted to the engine through KBEngine.createAccountResponse.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| registerName | string, the name submitted by the client when requested. |
| --- | --- |
| password | string, password |
| datas | bytes, the data attached to the client's request, can forward data to a third-party platform. |

<a id="onRequestAccountLogin"></a>

### def onRequestAccountLogin(loginName, password, datas):

Function description:
This callback is called when the client requests the server to login an
account.
The data can be checked and modified within this function, and the final
result is submitted to the engine through KBEngine.accountLoginResponse.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| loginName | string, the name submitted by the client when requested. |
| --- | --- |
| password | string, password. |
| datas | bytes, the data attached to the client request, can forward data to a third-party platform. |

<a id="onRequestCharge"></a>

### def onRequestCharge(ordersID, entityDBID, datas):

Function description:
This callback is invoked when billing is requested (usually KBEngine.charge
is called on baseapp).
Data can be checked and modified within this function, and the final result
is submitted to the engine via KBEngine.chargeResponse.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| ordersID | uint64, the ID of the order. |
| --- | --- |
| entityDBID | uint64, the entity DBID of the submitted order. |
| datas | bytes, the data attached to the client request, can forward data to a third-party platform. |
