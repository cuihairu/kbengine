# Entity class

[KBEngine module]

Entity is part of theKBEnginemodule.[More...](#detailed_description)

```python
import KBEngine
```

## Member functions

- def [baseCall](#baseCall)( self, methodName, methodArgs ):
- def [cellCall](#cellCall)( self, methodName, methodArgs ):
- def [isPlayer](#isPlayer)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## Callbacks

- def [onDestroy](#onDestroy)( self ):
- def [onEnterWorld](#onEnterWorld)( self ):
- def [onLeaveWorld](#onLeaveWorld)( self ):
- def [onEnterSpace](#onEnterSpace)( self ):
- def [onLeaveSpace](#onLeaveSpace)( self ):

## Attributes

| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| --- | --- |
| [id](#id) | Read-only Integer |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | Read-only uint32 |
| [isOnGround](#isOnGround) | Read-only bool |
| [inWorld](#inWorld) | Read-only bool |
| [className](#className) | Read-only string |

<a id="detailed_description"></a>

## A detailed description

Instances of classEntityrepresent
game objects on the client.
AnEntitycan call methods on its equivalent
entity in the base and cell applications viaENTITYCALL.
This requires a set of remotely-invoked functions (specified in the entity's .def file).
It also works the other way around, and a Client can have its functions remotely invoked
by the entity's base and cell parts (must be specified in the [ClientMethods] section of the entity's .def file).
Client entities can havecellattribute changes broadcast to them by using
any of the *_CLIENT* broadcast flags on properties in the entity's def file on the server side.
If a property is set to be broadcast, set_[property]() is called
on the client entity when acellattribute is changed.
See[http://kbengine.org/docs/programming/entitydef.html](http://kbengine.org/docs/programming/entitydef.html)for more info.

## Member function documentation

<a id="baseCall"></a>

### def baseCall(self, methodName, methodArgs):

Function description:
The method to call the base part of the entity.
Note: the entity must have a base part on the server side. Only client
entities controlled by the client can access this method.
Example:
js plugin: entity.baseCall("reqCreateAvatar", roleType, name);
c# plugin: entity.baseCall("reqCreateAvatar", new object[]{roleType, name});

parameters:

| methodName | string, method name. |
| --- | --- |
| methodArgs | objects, method parameter list. |

return:

- Because it is a remote call, it is not possible to block waiting for a return, so there is no return value.

<a id="cellCall"></a>

### def cellCall(self, methodName, methodArgs):

Function description:
The method to call the cell part of this entity.
Note: The entity must have a cell part on the server. Only client entities
controlled by the client can access this method.
Example:
js plugin: entity.cellCall("xxx", roleType, name);
c# plugin: entity.cellCall("xxx", new object[]{roleType, name});

parameters:

| methodName | string, method name. |
| --- | --- |
| methodArgs | objects, method parameter list. |

return:

- Because it is a remote call, it is not possible to block waiting for a return, so there is no return value.

<a id="isPlayer"></a>

### def isPlayer(self):

Function description:
This function returns whether theEntityis the player controlled by the current client.

returns:

- bool, If the player controlled by the current client returns true, otherwise it returns false.

<a id="isPlayer"></a>

### def isPlayer(self):

Function description:
This function returns whether theEntityis the player controlled by the current client.

returns:

- bool, If the player controlled by the current client returns true, otherwise it returns false.

<a id="getComponent"></a>

### def getComponent(self, componentName, all):

Function description:
Gets a component instance of the specified type attached to the entity.

parameters:

| componentName | string, The component type name. |
| --- | --- |
| all | bool, if True, Returns all instances of the same type of component, otherwise only returns the first or empty list. |

<a id="fireEvent"></a>

### def fireEvent(self, eventName, *args):

Function description:
Trigger entity events.

parameters:

| eventName | string, the name of the event to trigger. |
| --- | --- |
| args | The event datas to be attached, variable parameters. |

<a id="registerEvent"></a>

### def registerEvent(self, eventName, callback):

Function description:
Register entity events.

parameters:

| eventName | string, the name of the event to be registered for listening. |
| --- | --- |
| callback | The callback method used to respond to the event when the event fires. |

<a id="deregisterEvent"></a>

### def deregisterEvent(self, eventName, callback):

Function description:
Deregister entity events.

parameters:

| eventName | string, the name of the event to be deregister. |
| --- | --- |
| callback | The callback method to deregister of the listener. |

## Callback function documentation

<a id="onDestroy"></a>

### def onDestroy(self):

Called when the entity is destroyed

<a id="onEnterWorld"></a>

### def onEnterWorld(self):

If the entity is not client-controlled, it indicates that the
entity has entered the view scope of the client-controlled entity on the
server, at which point the client can see the entity.
If the entity is client controlled, it indicates that the entity
has created a cell on the server and entered the Space.

<a id="onLeaveWorld"></a>

### def onLeaveWorld(self):

If the entity is not client-controlled, it indicates that the entity has
left the view scope of the client-controlled entity on the server side,
and the client cannot see this entity at this time.
If the entity is client controlled, it indicates that the entity has
already destroyed the cell on the server and left the Space.

<a id="onEnterSpace"></a>

### def onEnterSpace(self):

The client-controlled entity enters a new space.

<a id="onLeaveSpace"></a>

### def onLeaveSpace(self):

The client-controlled entity leaves the current space.

## Attribute documentation

<a id="className"></a>

className

The class name of the entity.

Type:

- Read-only, string

<a id="position"></a>

position

The coordinates (x,y,z) of this entity in world space.
The data is synchronized from the server to the client.

Type:

- Vector3

<a id="direction"></a>

direction

This attribute describes the orientation of theEntityin world space. Data is synchronized from the server to the client.

Type:

- Vector3, which contains (roll, pitch, yaw) in radians.

<a id="isOnGround"></a>

isOnGround

If the value of this attribute is True, theEntityis on the ground, otherwise it is False.
If it is a client-controlled entity, this attribute will be synchronized
to the server when changed, and other entities will be synchronized
to the client by the server. The client can determine this value to
avoid the overhead of accuracy.

Type:

- Read-write, bool
