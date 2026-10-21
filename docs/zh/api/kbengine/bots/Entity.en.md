# Entity class

[KBEngine module]

Entity is part of theKBEnginemodule.[More...](#detailed_description)

```python
import KBEngine
```

## Member functions

- def [moveToPoint](#moveToPoint)( self, destination, velocity, distance, userData, faceMovement, moveVertically ):
- def [cancelController](#cancelController)( self, controllerID ):
- def [isPlayer](#isPlayer)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## Callbacks

- def [onEnterWorld](#onEnterWorld)( self ):
- def [onLeaveWorld](#onLeaveWorld)( self ):
- def [onEnterSpace](#onEnterSpace)( self ):
- def [onLeaveSpace](#onLeaveSpace)( self ):

## Attributes

| [base](#base) | Read-onlyENTITYCALL |
| --- | --- |
| [cell](#client) | Read-onlyENTITYCALL |
| [className](#className) | Read-onlystring |
| [clientapp](#clientapp) | Read-onlyPyClientApp |
| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| [id](#id) | Read-only Integer |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | Read-only uint32 |
| [isOnGround](#isOnGround) | Read-only bool |

<a id="detailed_description"></a>

## A detailed description

Instances of the classEntityrepresent game objects on the client.
AnEntitycan access its equivalent
entities in the base and cell applications viaENTITYCALL.
This requires a set of remotely-invoked functions (specified in the entity's
.def file).

## Member functions documentation

<a id="moveToPoint"></a>

### def moveToPoint(self, destination, velocity, distance, userData, faceMovement, moveVertically):

Function description:
Moves theEntityto the given
coordinate point in a straight line. The callback will be called on
success or failure.
AnyEntitycan only have one motion controller
at any time. Repeatedly calling any move function will terminate the
previous move controller.
Returns a controller ID that can be used to cancel this move.
Example:
You can useEntity.cancelController( movementID ) orEntity.cancelController( "Movement" )
to cancel the move. The callback will not be called if the move is cancelled.
The callback functions are defined as follows:

```python
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):
```

See also:

- Entity.cancelController

parameters:

| destination | Vector3, the target point to which theEntityis to be moved. |
| --- | --- |
| velocity | float, the speed to move theEntity(in m/s). |
| distance | float, the distance target which if it is within, movement is stopped. If the value is 0, it moves to the target position. |
| userData | object, user data passed to the callback function. |
| faceMovement | bool, True if the entity faces the direction of the move. If it is other mechanism, it is False. |
| moveVertically | bool, set to True means to move in a straight line directly to the point, and False means to move in a straight line paralell to the ground. |

returns:

- int, newly created controller ID.

<a id="cancelController"></a>

### def cancelController(self, controllerID):

Function description:
The cancelController function stops the effect of a controller on theEntity.
It can only be called on arealentity.

parameters:

| controllerID | integer, the index of the controller to cancel. A special controller type string can also be used. For example, only one movement/navigation controller can be activated at a time. This can be cancelled with entity.cancelController("Movement"). |
| --- | --- |

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

## Callback functions documentation

<a id="onEnterWorld"></a>

### def onEnterWorld(self):

If the entity is not a client-controlled
entity, it indicates that the entity has entered the View scope of the
entity controlled by the client on the server side. At this time, the
client can see this entity.
If the entity is a client-controlled entity, it indicates that the entity
has created a cell on the server and entered space.

<a id="onLeaveWorld"></a>

### def onLeaveWorld(self):

If the entity is not a client-side
control entity, it indicates that the entity has left the view scope of
the client-controlled entity on the server side, and the client cannot
see this entity at this time.
If the entity is a client-controlled entity, it indicates that the entity
has destroyed the cell on the server and left space.

<a id="onEnterSpace"></a>

### def onEnterSpace(self):

The client-controlled entity enters a
new space.

<a id="onLeaveSpace"></a>

### def onLeaveSpace(self):

The client-controlled entity leaves the
current space.

## Attributes documentation

<a id="base"></a>

base

base is the entityCall used to contact
the baseEntity. This attribute
is read-only, and is None if this entity does not have an associated baseEntity.

Other references:

- Entity.clientEntity Entity.allClients Entity.otherClients

Types:

- Read-only,ENTITYCALL

<a id="cell"></a>

cell

Description:
cell is theENTITYCALLused to
contact the cell entity. This attribute is Read-only, and is None if the
base entity has no associated cell.

Types:

- Read-onlyENTITYCALL

<a id="cellData"></a>

cellData

Description:
cellData is a dictionary property. Whenever the base entity has not created
its cell entity, the attributes of the cell entity are stored here.
If the cell entity is created, the values and[cellData](#cellData)attributes will be deleted. In addition to the attributes that the cell
entity specifies in the entity definition file, it also contains position,
direction, and spaceID.

Types:

- CELLDATADICT

<a id="className"></a>

className

Description:
The class name of the entity.

Types:

- Read-only, string

<a id="clientapp"></a>

clientapp

Description:
The client (object) to which the current entity belongs.

Types:

- Read-only,PyClientApp

<a id="position"></a>

position

The coordinates (x, y, z) of this
entity in world space. The data is synchronized from the server to the
client.

Types:

- Vector3

<a id="direction"></a>

direction

This attribute describes the orientation
of theEntityin world space. Data
is synchronized from the server to the client.

Types:

- Vector3, which contains (roll, pitch, yaw) in radians.

<a id="isOnGround"></a>

isOnGround

If the value of this attribute is True,
theEntityis on the ground, otherwise
it is False.
If it is a client-controlled entity, this attribute will be synchronizd
to the server at the time of change, and other entities will be synchronized
to the client by the server. The client can determine this value to reduce
the cost of accuracy.

Types:

- Read-write, bool
