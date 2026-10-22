# KBEngine module

ThisKBEnginemodule provides Python scripts to
control the dbmgr process to handle entity login queries and data access.

## Member functions

- def [addTimer](#addTimer)( initialOffset, repeatOffset=0, callbackObj=None ):
- def [delTimer](#delTimer)( id ):
- def [executeRawDatabaseCommand](#executeRawDatabaseCommand)( command, callback, threadID, dbInterfaceName ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## Callbacks

- def [onDBMgrReady](#onDBMgrReady)( ):
- def [onDBMgrShutDown](#onDBMgrShutDown)( ):
- def [onReadyForShutDown](#onReadyForShutDown)( ):
- def [onSelectAccountDBInterface](#onSelectAccountDBInterface)( accountName ):

## Member functions documentation

<a id="addTimer"></a>

### def addTimer(initialOffset, repeatOffset=0, callbackObj=None):

Function description:
Registers a timer. The timer triggers the callback function specified
by callbackObj. The callback will be executed the first time after
"initialOffset" seconds, and then executed once every "repeatOffset"
seconds.
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

| initialOffset | float, specifies the time interval in seconds for the timer to register the first callback. |
| --- | --- |
| repeatOffset | float, specifies the time interval (in seconds) after each execution of the first callback execution. You must remove the timer with the function[delTimer](#delTimer), otherwise it will continue to repeat. Values less than or equal to 0 will be ignored. |
| callbackObj | function, the specified callback function object. |

returns:

- integer, this function returns the internal id of the timer. This id can be used to remove the timer using[delTimer](#delTimer).

<a id="delTimer"></a>

### def delTimer(id):

Function description:
The delTimer function is used to remove a registered timer. The removed
timer is no longer executed. Single-shot timers are automatically removed
after the callback is executed, and it is not necessary to use delTimer
to remove it. If the delTimer function receives an invalid id (for
example, it was removed), it will generate an error.
A use case is in the KBEngine.[addTimer](#addTimer)example.

parameters:

| id | integer, specifies the timer id to remove. |
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

<a id="onDBMgrReady"></a>

### def onDBMgrReady():

Function description:
This function is called back when the current process is ready.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onDBMgrShutDown"></a>

### def onDBMgrShutDown():

Function description:
This function is called when the process shuts down.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

<a id="onReadyForShutDown"></a>

### def onReadyForShutDown():

Function description:
If this function is implemented in a script, the callback function is called when the process is ready to exit.
You can use this callback to control when the process exits.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

returns:

- bool, if it returns True, it allows the process to exit. Returning other values will cause the process to ask again after a period of time.

<a id="onSelectAccountDBInterface"></a>

### def onSelectAccountDBInterface(accountName):

Function description:
When implemented in a script, this callback returns the database interface
corresponding to an account. After the interface is selected, the dbmgr
operations related to this account are completed by the corresponding
database interface.
Database interfaces are defined inkbengine_defaults.xml->dbmgr->databaseInterfaces.
Use this function to determine which database the account should be stored
in based on accountName.
Note: This callback interface must be implemented in the portal module ( kbengine_defaults.xml ->entryScriptFile).

parameters:

| accountName | string, the name of the account. |
| --- | --- |

returns:

- string, the database interface name (database interfaces are defined inkbengine_defaults.xml->dbmgr->databaseInterfaces).
