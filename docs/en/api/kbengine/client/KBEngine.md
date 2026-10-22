# KBEngine module

ThisKBEnginemodule provides a part of the
logic script layer access toentity,
as well as current space data and so on.

## Class

- Entity

## Member function

- def [login](#login)( username, password ):
- def [createAccount](#createAccount)( username, password ):
- def [reloginBaseapp](#reloginBaseapp)():
- def [player](#player)( ):
- def [resetPassword](#resetPassword)(username):
- def [bindAccountEmail](#bindAccountEmail)( emailaddress ):
- def [newPassword](#newPassword)( oldpassword, newpassword ):
- def [findEntity](#findEntity)( entityID ):
- def [getSpaceData](#getSpaceData)( key ):

## Callback

## Attributes

| [component](#component) | Read onlystring |
| --- | --- |
| [entities](#entities) | Entities[entity_uuid](#entity_uuid)uint64[entity_id](#entity_id)int32[spaceID](#spaceID)int32 |

## Member functions documentation

<a id="login"></a>

### def login(username, password):

Function description:
Login account to KBEngine server.
Note: If the plug-in and the UI layer use event interaction mode, do
not call directly from the UI layer. Please trigger a "login" event to the
plug-in. The event is accompanied by the data username and password.

parameter

| username | string, username. |
| --- | --- |
| password | string, password. |

<a id="createAccount"></a>

### def createAccount(username, password):

Function description
Request to create a login account on the KBEngine server.
Note: If the plug-in and the UI layer use the event interaction mode,
do not call directly from the UI layer. Please trigger a "createAccount"
event to the plug-in. The event is accompanied by the data username and
password.

parameters:

| username | string, username. |
| --- | --- |
| password | string, password. |

<a id="reloginBaseapp"></a>

### def reloginBaseapp():

Function description
Requests to re-login to the KBEngine server (usually used after a dropped
connection in order to connect to the server more quickly and continue to
control the server role).
Note: If the plug-in and the UI layer use event interaction mode, do
not call directly from the UI layer, please trigger a "reloginBaseapp"
event to the plug-in, and the incidental data is empty.

<a id="player"></a>

### def player():

Function description
Gets the entity that the current client controls.

return:

- Entity, return controlled entity, if it does not exist (e.g.: failed to connect to the server) returns null.

<a id="resetPassword"></a>

### def resetPassword(username):

Function description
Asks loginapp to reset the password of the account. The server will send
a password reset email (usually the forgotten password function) to the
email address to which the account is bound.

parameters:

| username | string, username. |
| --- | --- |

<a id="bindAccountEmail"></a>

### def bindAccountEmail(emailaddress):

Function description
Requests Baseapp to bind the email address of the account.

parameters:

| emailaddress | string, email address. |
| --- | --- |

<a id="newPassword"></a>

### def newPassword(oldpassword, newpassword):

Function description
Requests to set a new password for the account.

parameters:

| oldpassword | string, old password |
| --- | --- |
| newpassword | string, new password |

<a id="findEntity"></a>

### def findEntity(entityID):

Function description
Finds an instance object of an entity by its ID.

parameters:

| entityID | int32, entity ID. |
| --- | --- |

returns:

- EntityAn entity instance is returned. There can be no return null.

<a id="getSpaceData"></a>

### def getSpaceData(key):

Function description
Gets the space data for the specified key.
The space data is set by the user on the server throughsetSpaceData.

parameters:

| key | string, a keyword |
| --- | --- |

returns:

- string, specifies the value at the key

## Callback function documentation

## Attributes documentation

<a id="component"></a>

component

Description:
This is the component that is running in the current scripting environment.
(So far) Possible values are 'cellapp', 'baseapp', 'client', 'dbmgr', 'bots'
and 'editor'.

<a id="entities"></a>

entities

Description:
entities is a dictionary object that contains all the entities in the current process.

Types:

- Entities

<a id="entity_uuid"></a>

entity_uuid

Description:
The uuid of the entity. Change the ID and entity to bind to this login.
When using the heavy login function, the server compares this ID and
determines the validity.

<a id="entity_id"></a>

entity_id

Description:
The ID of the entity controlled by the current client.

<a id="spaceID"></a>

spaceID

Description:
The ID of theSpacewhere the
entity controlled by the current client is located (also can be understood
as the corresponding scene, room, and copy).
