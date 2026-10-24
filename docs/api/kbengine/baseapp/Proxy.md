# Proxy class

[KBEngine module]

Proxy is part of theKBEnginemodule.[More...](#detailed_description)

```python
import KBEngine
```

## Parent

Entity

## Member function

- def [disconnect](#disconnect)( self ):
- def [getClientType](#getClientType)( self ):
- def [getClientDatas](#getClientDatas)( self ):
- def [giveClientTo](#giveClientTo)( self, proxy ):
- def [streamFileToClient](#streamFileToClient)( self, resourceName, desc="", id=-1 ):
- def [streamStringToClient](#streamStringToClient)( self, data, desc="", id=-1 ):

## Callbacks

- def [onClientDeath](#onClientDeath)( self ):
- def [onClientGetCell](#onClientGetCell)( self ):
- def [onClientEnabled](#onClientEnabled)( self ):
- def [onGiveClientToFailure](#onGiveClientToFailure)( self ):
- def [onLogOnAttempt](#onLogOnAttempt)( self, ip, port, password ):
- def [onStreamComplete](#onStreamComplete)( self, id, success ):

## Attributes

| [__ACCOUNT_NAME__](#__ACCOUNT_NAME__) | Read-onlystring |
| --- | --- |
| [__ACCOUNT_PASSWORD__](#__ACCOUNT_PASSWORD__) | Read-onlystring |
| [clientAddr](#clientAddr) | Read-only |
| [clientEnabled](#clientEnabled) | Read-onlybool |
| [hasClient](#hasClient) | Read-onlybool |
| [roundTripTime](#roundTripTime) | Read-only |
| [timeSinceHeardFromClient](#timeSinceHeardFromClient) | Read-only |

<a id="detailed_description"></a>

## A detailed description

AProxyis a special type ofEntity.
It inherits fromEntityand has an associated client. By itself, it is a
proxy client entity that handles all server-to-client updates. Cannot
createProxyclass objects directly script.

## Member functions documentation

<a id="disconnect"></a>

### def disconnect(self):

Disconnect the client.

<a id="getClientType"></a>

### def getClientType(self):

Function description:
This function returns the client type.

returns:

- UNKNOWN_CLIENT_COMPONENT_TYPE = 0, CLIENT_TYPE_MOBILE = 1, // Mobile phone CLIENT_TYPE_WIN = 2, // PC, typically EXE clients CLIENT_TYPE_LINUX = 3 // Linux Application program CLIENT_TYPE_MAC = 4 // Mac Application program CLIENT_TYPE_BROWSER = 5, // Web applications, HTML5, Flash CLIENT_TYPE_BOTS = 6, // bots CLIENT_TYPE_MINI = 7, // Mini-Client CLIENT_TYPE_END = 8 // end

<a id="getClientDatas"></a>

### def getClientDatas(self):

Function description:
This function returns the data attached to the client when logging in and
registering.
This data can be used to expand the operating system. If a third-party
account service is connected, this data is sent to the third-party service
system through the interfaces process.

returns:

- tuple, a tuple of 2 elements (login data bytes, registration data bytes), the first element is the datas parameter passed in when the client invokes the login, and the second element is passed in when the client registers. Since they can store arbitrary binary data, they all exist as bytes.

<a id="giveClientTo"></a>

### def giveClientTo(self, proxy):

Function description:
The client's controller is transferred to another Proxy, the current Proxy
must have a client and the target Proxy must have no associated client,
otherwise it will cause an error.

See also:

- Proxy.onGiveClientToFailure

parameters:

| proxy | Control will be transferred to this entity. |
| --- | --- |

<a id="streamFileToClient"></a>

### def streamFileToClient(self, resourceName, desc="", id=-1):

Function description:
This function is similar tostreamStringToClient()and sends a resource file to the client. The sending process operates
on different threads so it does not compromise the main thread.

See also:

- Proxy.onStreamComplete

parameters:

| resourceName | The name of the resource to send, including the path. |
| --- | --- |
| desc | An optional string that describes the resource sent to the client. |
| id | A 16-bit id whose value depends entirely on the caller. If the incoming -1 system will select an unused id in the queue. The client can make resource judgments based on this id. |

returns:

- The id associated with this download.

<a id="streamStringToClient"></a>

### def streamStringToClient(self, data, desc="", id=-1):

Function description:
Sends some data to the client bound to the current entity. If the client
port data is cleared, this function can only be called when the client
binds to the entity again. The 16-bit id is entirely up to the caller.
If the caller does not specify this ID then the system will allocate an unused id. The client can make resource judgments based on this id.
You can define a callback function (onStreamComplete) in a Proxy-derived class. This callback function is called when all data is successfully sent to the client or when the download fails.
See also:Proxy.onStreamComplete, clientEntity.onStreamDataStarted,Entity.onStreamDataRecv, andEntity.onStreamDataCompleted.

parameters:

| data | The string to send |
| --- | --- |
| desc | An optional description string sent. |
| id | A 16-bit id whose value depends entirely on the caller. If the incoming -1 system will select an unused id in the queue. |

returns:

- The id associated with this download.

## Callback functions documentation

<a id="onClientDeath"></a>

### def onClientDeath(self):

If this callback is implemented in a script, this method will be called when the client disconnects. This method has no parameters.

<a id="onClientGetCell"></a>

### def onClientGetCell(self):

If this callback is implemented in a script,
the callback is called when the client can call the entity'scellattribute

<a id="onClientEnabled"></a>

### def onClientEnabled(self):

If this callback is implemented in the
script, it is invoked when the entity is available (various initializations
and communication with the client). This method has no parameters.
Note: giveClientTo also assigns control to the entity and causes the callback to be called.

<a id="onGiveClientToFailure"></a>

### def onGiveClientToFailure(self):

If this callback is implemented in a script, it is called when the entity
fails to call[giveClientTo](#onStreamComplete).
This method has no parameters.

<a id="onLogOnAttempt"></a>

### def onLogOnAttempt(self, ip, port, password):

If this callback is implemented in a
script, it is invoked when a client attempts to log in using the current
account entity.
This situation usually happens when the entity that exists in memory is in a valid
state, the most obvious example is user A logs in with this account,
and user B tries to use the same account to log in, triggering this callback.
This callback function can return the following constant values:
KBEngine.LOG_ON_ACCEPT:
Allows the new client to bind to the entity. If the entity has bound a
client, the previous client will be kicked out.
KBEngine.LOG_ON_REJECT:
Reject new client entity binding.
KBEngine.LOG_ON_WAIT_FOR_DESTROY:
Wait for the entity to be destroyed before the client binds.

parameters:

| ip | The IP address of the client trying to log in. |
| --- | --- |
| port | The port to which the client attempted to log in. |
| password | The MD5 password used when the user logs in. |

<a id="onStreamComplete"></a>

### def onStreamComplete(self, id, success):

If you implement this callback in a
script, when a user usesProxy.streamStringToClient()
orProxy.streamFileToClient()
and is completed, this callback is invoked.

parameters:

| id | The id associated with the download. |
| --- | --- |
| success | Success or failure |

## Attributes documentation

<a id="__ACCOUNT_NAME__"></a>

__ACCOUNT_NAME__

Note:
If the proxy is an account, you can access __ACCOUNT_NAME__ to get the account name.

<a id="__ACCOUNT_PASSWORD__"></a>

__ACCOUNT_PASSWORD__

Note:
If the proxy is an account, you can access __ACCOUNT_PASSWORD__ to get the MD5 password.

<a id="clientAddr"></a>

clientAddr

This is a tuple object that contains the client's ip and port.

<a id="clientEnabled"></a>

clientEnabled

Whether the entity is already available. The script cannot communicate with the client until the entity is available.

<a id="hasClient"></a>

hasClient

Proxy is bound to a client connection.

<a id="roundTripTime"></a>

roundTripTime

The average round-trip time for client communication between the server and this Proxy over a period of time. This property only takes effect under Linux.

<a id="timeSinceHeardFromClient"></a>

timeSinceHeardFromClient

The time (in seconds) that has passed since the client packet was last received.
