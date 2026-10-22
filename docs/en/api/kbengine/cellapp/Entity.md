# Entity

[KBEngine module]

Entity is part of theKBEnginemodule.[More...](#detailed_description)

```python
import KBEngine
```

## Member functions

- def [accelerate](#accelerate)( self, accelerateType, acceleration ):
- def [addYawRotator](#addYawRotator)( self, targetYaw, velocity, userArg ):
- def [addProximity](#addProximity)( self, range, userArg ):
- def [addTimer](#addTimer)( self, start, interval=0.0, userData=0 ):
- def [cancelController](#cancelController)( self, controllerID ):
- def [clientEntity](#clientEntity)( self, destID ):
- def [canNavigate](#canNavigate)( self ):
- def [debugView](#debugView)( self ):
- def [delTimer](#delTimer)( self, id ):
- def [destroy](#destroy)( self ):
- def [destroySpace](#destroySpace)( self ):
- def [entitiesInView](#entitiesInView)( self, pending):
- def [entitiesInRange](#entitiesInRange)( self, range, entityType=None, position=None ):
- def [isReal](#isReal)( self ):
- def [moveToEntity](#moveToEntity)( self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos ):
- def [moveToPoint](#moveToPoint)( self, destination, velocity, distance, userData, faceMovement, moveVertically ):
- def [getViewRadius](#getViewRadius)( self ):
- def [getViewHystArea](#getViewHystArea)( self ):
- def [getRandomPoints](#getRandomPoints)( self, centerPos, maxRadius, maxPoints, layer ):
- def [navigate](#navigate)( self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData ):
- def [navigatePathPoints](#navigatePathPoints)( self, destination, maxSearchDistance, layer ):
- def [setViewRadius](#setViewRadius)( self, radius, hyst=5 ):
- def [teleport](#teleport)( self, nearbyMBRef, position, direction ):
- def [writeToDB](#writeToDB)( self, shouldAutoLoad, dbInterfaceName ):
- def [getWitnesses](#getWitnesses)( self ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## Callbacks

- def [onDestroy](#onDestroy)( self ):
- def [onEnterTrap](#onEnterTrap)( self, entity, rangeXZ, rangeY, controllerID, userArg ):
- def [onEnteredView](#onEnteredView)( self, entity ):
- def [onGetWitness](#onGetWitness)( self ):
- def [onLeaveTrap](#onLeaveTrap)( self, entity, rangeXZ, rangeY, controllerID, userArg ):
- def [onLoseControlledBy](#onLoseControlledBy)( self, id ):
- def [onLoseWitness](#onLoseWitness)( self ):
- def [onMove](#onMove)( self, controllerID, userData ):
- def [onMoveOver](#onMoveOver)( self, controllerID, userData ):
- def [onMoveFailure](#onMoveFailure)( self, controllerID, userData ):
- def [onRestore](#onRestore)( self ):
- def [onSpaceGone](#onSpaceGone)( self ):
- def [onTurn](#onTurn)( self, controllerID, userData ):
- def [onTeleport](#onTeleport)( self ):
- def [onTeleportFailure](#onTeleportFailure)( self ):
- def [onTeleportSuccess](#onTeleportSuccess)( self, nearbyEntity ):
- def [onTimer](#onTimer)( self, timerHandle, userData ):
- def [onUpdateBegin](#onUpdateBegin)( self ):
- def [onUpdateEnd](#onUpdateEnd)( self ):
- def [onWitnessed](#onWitnessed)( self, isWitnessed ):
- def [onWriteToDB](#onWriteToDB)( self ):

## Attributes

| [allClients](#allClients) | Read-only PyClient |
| --- | --- |
| [base](#base) | Read-onlyBaseEntityCall |
| [client](#client) | Read-onlyClientEntityCall |
| [controlledBy](#controlledBy) | BaseEntityCall |
| [className](#className) | Read-onlystring |
| [direction](#direction) | Tuple of 3 floats as (roll, pitch, yaw) |
| [hasWitness](#hasWitness) | boolean |
| [id](#id) | Read-only Integer |
| [isDestroyed](#isDestroyed) | Read-only bool |
| [isWitnessed](#isWitnessed) | Read-only bool |
| [layer](#layer) | int8 |
| [otherClients](#otherClients) | Read-only PyClient |
| [position](#position) | Vector3 |
| [spaceID](#spaceID) | Read-only uint32 |
| [topSpeed](#topSpeed) | float |
| [topSpeedY](#topSpeedY) | float |
| [volatileInfo](#volatileInfo) | float |

<a id="detailed_description"></a>

## A detailed description

Instances of classEntityrepresent
game objects in the cell.
AnEntitycan either be "real" or "ghosted".
A "ghost"Entityis a copy of a
"real"Entityliving on a neighboring cell.
There is a unique "real"Entityinstance for each entity, and 0 or more "ghost"Entityinstances.
AnEntityinstance controls the location
data of the entity, including its position and rotation. It also controls
how often this data is sent to the client (if possible). The location data
can be updated by a unique client and modified by the controller object
using the teleport member function. Controllers are non-python objects
that can be used to change the location data over time on cell entities.
They are added toEntitythrough
member functions such as "trackEntity" and "turnToYaw" and can be removed
via "cancelController".
Area of Interest or "View" is an important concept for allKBEngineentities that belong to a client. The view of an entity is the area that
the client (if it has a client) can perceive around this entity.
This is used to select the amount of data sent to the client. The actual
shape of the View is defined by the range of distances on the x-axis and
around the z-axis, and there is a lag region that extends outward
like a shape. AnEntityenters
anotherEntity's view, but does
not leave it until it leaves the lag area. AnEntitycan modify its View size via "setViewRadius". You can find all entities
within a specific distance with "entitiesInRange" and set traps to capture
all entities that enter the trap with "addProximity".
The newEntityon cellapp can be
created usingKBEngine.createEntity.
An entity can also be created by the baseapp remote calls to theKBEngine.createCellEntityfunction.
AnEntitycan access it equivalent
entities on the base and client applications viaENTITYCALL.
This requires a set of remotely-invoked functions (specified in the entity's .def file)

## Member function documentation

<a id="accelerate"></a>

### def accelerate(self, accelerateType, acceleration):

Function description:
Accelerate the current movement of the entity.

The activities that can be accelerated include:

- Entity.moveToEntity
- Entity.moveToPoint
- Entity.navigate
- Entity.addYawRotator

parameters:

| accelerateType | string, the type of movement affected such as: "Movement", "Turn". |
| --- | --- |
| velocity | float, acceleration per second, use negative to decelerate |

returns:

- The current speed of the affected entity.

<a id="addYawRotator"></a>

### def addYawRotator(self, targetYaw, velocity, userArg):

Function description:
The control entity rotates around yaw.Entity.onTurnis called when the rotation completes.
To remove it, useEntity.cancelControllerwith the controller ID or useEntity.cancelController("Movement")to remove it.

See:

- Entity.cancelController

parameters:

| targetYaw | float, the given target yaw radians. |
| --- | --- |
| velocity | float, the arc per second when rotated. |
| userArg | Optional integer that is common to all controllers. If this value is not 0, it is passed to the callback function. It is recommended to set the default value to 0 in the callback prototype. |

<a id="addProximity"></a>

### def addProximity(self, rangeXZ, rangeY, userArg):

Function description:
Create an area trigger that will notify theEntitywhen other entities enter or leave the trigger area. This area is a square (for efficiency).
If another entity is within a given distance on the x-axis and z-axis,
it is considered to be within the range. ThisEntityis notified via the onEnterTrap and onLeaveTrap functions, which can be
defined as follows:

```python
	def onEnterTrap( self, entityEntering, rangeXZ, rangeY, controllerID, userArg = 0 ):
	def onLeaveTrap( self, entityLeaving, rangeXZ, rangeY, controllerID, userArg = 0 ):
```

Because the scope trigger is a controller, useEntity.cancelControllerwith the controller ID to delete it.
It should be noted that the callback may be triggered immediately, even
before the call to addProximity() returns.

See:

- Entity.cancelController

parameters:

| rangeXZ | float, the size of the xz axis area of the trigger, must be greater than or equal to zero. |
| --- | --- |
| rangeY | float, the height of the y-axis of the trigger, must be greater than or equal to zero. It should be noted that for this parameter to take effectkbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y must be set to true. Open y-axis management will increase CPU consumption, because some games have a large number of entities at the same y-axis height or all on the ground which is almost completely flat. Because of this, the collision becomes very dense. 3D space games or small room-type games are more suitable for this option. |
| userArg | Optional integer that is common to all controllers. If this value is not 0, it is passed to the callback function. It is recommended to set the default value to 0 in the callback prototype. |

returns:

- The ID of the created controller.

<a id="addTimer"></a>

### def addTimer(self, start, interval=0.0, userData=0):

Function description:
Register a timer. The timer triggers the callback function[onTimer](#onTimer)The callback function will be executed the first time after "initialOffset"
seconds, and then it will be executed once every "repeatOffset" seconds
with the "userArg" parameter. (integer only)
The[onTimer](#onTimer)function must be defined in the
cell part of the entity with two parameters. The first is an integer, the
timer's ID (which can be used to remove the timer's "[delTimer](#delTimer)"
function), and the second is the user parameter "userArg".
Example:

```python
# Here is an example of using addTimer
import 
KBEngine

 
class MyCellEntity( 
KBEngine
.
Entity
 ):
 
    def __init__( self ):
        
KBEngine
.
Entity
.__init__( self )
 
        # Add a timer, perform the first time after 5 seconds, and execute once every 1 second. The user parameter is 9.
        self.addTimer( 5, 1, 9 )
 
        # Add a timer and execute it after 1 second. The default user parameter is 0.
        self.addTimer( 1 )
 
    # 
Entity
 timer callback "onTimer" is called
    def onTimer( self, id, userArg ):
        print "MyCellEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # if this is a repeated timer, when it is no longer needed, call the following function to remove it:
        #     self.delTimer( id )
```

parameters:

| initialOffset | float, specifies the time interval (in seconds) for the timer to execute the first callback. |
| --- | --- |
| repeatOffset | float, specifies the time interval (in seconds) between each callback after the first callback. The timer must be removed with the function[delTimer](#delTimer), otherwise it will be repeated. Values less than or equal to 0 will be ignored. |
| userArg | integer, specifies the value of the "userArg" parameter when calling "[onTimer](#onTimer)". |

returns:

- integer, returns the internal ID of the timer. This ID can be used to remove the timer using[delTimer](#delTimer).

<a id="cancelController"></a>

### def cancelController(self, controllerID):

Function description:
The function cancelController stops the effect of a controller onEntity.
It can only be called on arealentity.

parameters:

| controllerID | integer, the index of the controller to cancel. A special controller type string can also be used as its type. For example, only one mobile/navigation controller can be activated at a time. This can be cancelled with entity.cancelController( "Movement" ). |
| --- | --- |

<a id="clientEntity"></a>

### def clientEntity(self, destID):

Function description:
This method can access the method of an entity in its own client
(the current entity must be bound to the client). Only the entities in
the View scope will be synchronized to the client. It can only be called
on arealentity.

parameters:

| destID | integer, the ID of the target entity. |
| --- | --- |

<a id="canNavigate"></a>

### def canNavigate(self):

Function description:
This method determines whether the current entity can use the navigation (Entity.navigate)
feature. It can only be called on arealentity.
Usually it can use navigation when the entity's Space usesKBENgine.addSpaceGeometryMappingto load valid navigation collision data (Navmesh or 2D tile data) and the
entity is available in the effective navigation area.

returns:

- bool, returns True if the entity can use the Navigate function in the current space, otherwise it returns False.

<a id="debugView"></a>

### def debugView(self):

Function description:
debugView outputs theEntity's View
details to the cell's debug log. A description of the workings of the View
system can be found in theEntityclass documentation.
A sample of information is as follows:

```python
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 size=4, Seen=4, Pending=0, ViewRadius=50.000, ViewHyst=5.000
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(102), position(771.586.211.002.776.55), dist=0
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Monster(1028), position(820.834.211.635.768.749), dist=49.8659
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 NPC(1025), position(784.024.210.95.782.273), dist=13.6915
  INFO cellapp [0x00001a1c] [2014-11-04 00:28:41,409] - Avatar::debugView: 100 Avatar(106), position(771.586.211.002.776.55), dist=0
```

The first line of information tells us:

- It is entity #1000's data.
- There are 4 entities in its View area and all have been synchronized to the client.
- There are 0 entities in its view Area that are waiting to be synchronized to the client.
- The radius of the View is 50.000
- The lag area of the View extends 5.000 outward.

<a id="delTimer"></a>

### def delTimer(self, id):

Function description:
The delTimer function is used to remove a registered timer. The removed
timer is no longer executed. Single shot timers are
automatically removed after the callback is executed, and it is not
necessary to use the delTimer to remove it. If the delTimer function
uses an invalid ID (for example, it has been removed), an error will be
generated.

parameters:

| id | integer, which specifies the timer ID to remove. If the parameter is the string "All", all timers are removed at once. |
| --- | --- |

<a id="destroy"></a>

### def destroy(self):

Function description:
This function destroys its localEntityinstance. If the entity has aghostpart on other processes, it will also notify for their destruction. This
function is best called by the entity itself, and throws an exception if
the entity is aghost. If the
callback function onDestroy() is implemented, it is executed.

<a id="destroySpace"></a>

### def destroySpace(self):

Function description:
Destroys thespacethis entity is in.

<a id="entitiesInView"></a>

### def entitiesInView(self, pending):

Function description:
Get a list of entities in theViewscope of this entity.

parameters:

| pending | bool, optional parameter, the default is False, only all entities visible to the client are returned, otherwise all entities visible to the server but not synchronized to the client are returned. |
| --- | --- |

<a id="entitiesInRange"></a>

### def entitiesInRange(self, range, entityType=None, position=None):

Function description:
Search forentitieswithin a given distance.
This is a spherical search. The distances of the three axes must be
measured. This can findentitiesthat are
outside theViewscope of this
entity, but cannot find entities in othercells.
Example:

```python
  self.entitiesInRange( 100, 'Creature', (100, 0, 100) )
```

Searches for a list of entities of type 'Creature' (an instantiated entity
of a subclass of 'Creature'). The center point is (100, 0, 100) and the
search radius is 100 meters.

```python
  [ e for e in self.entitiesInRange( 100, None, (100,0,100) ) if isinstance( e, BaseType ) ]
```

Gives a list of entities instantiated from subclasses of 'BaseType'.

parameters:

| range | Search distance around this entity, float type |
| --- | --- |
| entityType | An optional string parameter, the entity's type name, used to match entities. If the entity type is a valid class name (valid entities are ones listed in/scripts/entities.xml) only this type of entity will be returned, otherwise all entities in this range will be returned. |
| position | OptionalVector3type parameter, which is the center of the search radius is centered on the entity itself by default. |

returns:

- A list ofEntityobjects in a given range.

<a id="isReal"></a>

### def isReal(self):

Function description:
This function returns whether theEntityisrealor aghost.
This function is rarely used but is useful for debugging.

returns:

- bool, True if real, otherwise False.

<a id="moveToEntity"></a>

### def moveToEntity(self, destEntityID, velocity, distance, userData, faceMovement, moveVertically, offsetPos):

Function description:
Moves theEntitystraight to anotherEntityposition.
AnyEntitycan only have one motion controller
at any time. Repeatedly calling any move function will terminate the previous
move controller.
This function will return a controller ID that can be used to cancel this move.
For example,Entity.cancelController( movementID ).
You can also cancel the move usingEntity.cancelController( "Movement" ).
The callback function will not be called if the move is cancelled.

```python
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):
```

References:

- Entity.cancelController

parameters:

| destEntityID | int, the ID of the targetEntity |
| --- | --- |
| velocity | float, speed of theEntitymove, in m/s |
| distance | float, distance target that when reached the entity will stop moving, if the value is 0, it moves to the target position. |
| userData | object, optional parameter, when the callback function is invoked the userData parameter will be this value. |
| faceMovement | bool, optional parameter, True if the entity faces the direction of the move. If it is other mechanism, it is False. |
| moveVertically | bool, optional parameter, set to True to move in a straight line, set to False means to move in a straight line parallel to the ground. |
| offsetPos | Vector3, optional parameter, Set a certain offset value, such as moving the target position to the left of the entity. |

returns:

- int, newly created controller ID.

<a id="moveToPoint"></a>

### def moveToPoint(self, destination, velocity, distance, userData, faceMovement, moveVertically):

Function description:
Move theEntityto the given
coordinate point in a straight line. The callback function is invoked on
success or failure.
AnyEntitycan only have one motion controller
at any time. Repeatedly calling any move function will terminate the
previous move controller.
Returns a controller ID that can be used to cancel this move.
For example:
Entity.cancelController( movementID ).
You can also cancel the move withEntity.cancelController( "Movement" ).
The callback function will not be called if the move is cancelled.
The callback function is defined as follows:

```python
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):
```

See:

- Entity.cancelController

parameters:

| destination | Vector3, the target point to which theEntityis to be moved |
| --- | --- |
| velocity | float,Entity's moving speed, in m/s |
| distance | float, distance target that when reached the entity will stop moving, if the value is 0, it moves to the target position. |
| userData | object, data passed to the callback function |
| faceMovement | bool, True if the entity faces the direction of the move. If it is other mechanism, it is false. |
| moveVertically | bool, set to True to move in a straight line, set to False means to move in a straight line parallel to the ground. |

returns:

- int, newly created controller ID.

<a id="getViewRadius"></a>

### def getViewRadius(self):

Function description:
This function returns the current View radius value of thisEntity.
Data can be set viaEntity.setViewRadius( radius, hyst ).

returns:

- float, View radius

<a id="getViewHystArea"></a>

### def getViewHystArea(self):

Function description:
This function returns the current lag area value of thisEntityView.
Data can be set viaEntity.setViewRadius( radius, hyst ).

returns:

- float, The current lag area value of this Entity's View.

<a id="getRandomPoints"></a>

### def getRandomPoints(self, centerPos, maxRadius, maxPoints, layer):

Function description:
This function is used to get an array of random coordinate point thatEntity.navigatecan reach in a certain area centered on a certain coordinate point.

parameters:

| centerPos | Vector3,Entitycenter coordinates |
| --- | --- |
| maxRadius | float, the maximum search radius |
| maxPoints | uint32, the maximimum number of random coordinate points returned. |
| layer | int8, layer of navmesh to search. |

returns:

- tuple, an array of one or more coordinates.

<a id="navigate"></a>

### def navigate(self, destination, velocity, distance, maxMoveDistance, maxSearchDistance, faceMovement, layer, userData):

Function description:
Use the navigation system to move thisEntityto a target point. A callback will be invoked on success or failure.
KBEnginecan have several pre-generated
navigation meshes with different mesh sizes (leading to different navigation paths).
AnyEntitycan only have one motion controller
at any time. Repeatedly calling any move function will terminate the
previous move controller.
Returns a controller ID that can be used to cancel this move.
For example:
Entity.cancelController( movementID ).
You can also cancel the movement controller withEntity.cancelController( "Movement" ).
The callback function will not be called if the move is cancelled.
The callback functions are defined as follows:

```python
	def onMove( self, controllerID, userData ):
	def onMoveOver( self, controllerID, userData ):
	def onMoveFailure( self, controllerID, userData ):
```

See:

- Entity.cancelController

parameters:

| destination | Vector3, the target point where theEntitymoves. |
| --- | --- |
| velocity | float,Entity's move speed, in m/s |
| distance | float, distance target that when reached the entity will stop moving, if the value is 0, it moves to the target position. |
| maxMoveDistance | float, the maximum move distance |
| maxSearchDistance | float, the maximum search distance from the navigatio ndata. |
| faceMovement | bool, True if the entity faces the direction of the move (default). Otherwise False. |
| layer | int8, navmesh layer to search |
| userData | object, the data passed to the callback function |

returns:

- int, the newly created controller ID.

<a id="navigatePathPoints"></a>

### def navigatePathPoints(self, destination, maxSearchDistance, layer):

Function description:
This functions returns a list of path points from the currentEntitylocation to the destination.

parameters:

| destination | Vector3, target point where theEntitymoves |
| --- | --- |
| maxSearchDistance | float, the maximum search distance |
| layer | int8, navmesh layer to search for a path on. |

<a id="setViewRadius"></a>

### def setViewRadius(self, radius, hyst=5):

Function description:
Specifies the size of theEntity'sView.
This function can only be used byWitnessrelated entities.
Note: You can set the default View radius by setting thekbengine.xmlconfiguration option 'cellapp/defaultViewRadius'.
The View radius can't be greater than 500.0 by default. For details, please see thekbengine_defaults.xmlconfiguration option 'cellapp/entity_posdir_updates'.
Data can be obtained withEntity.getViewRadius( ) andEntity.getViewHystArea( ).

parameters:

| radius | float, specifies the radius of the View area |
| --- | --- |
| hyst | float, specifies the size of the lag area of the View. A reasonable setting of the lag area will reduce the sensitivity of View collisions and reduce CPU consumption. Views where one entity enters another entity must span the View radius area, but entities that leave the View area need to move out of the View radius area including the lag area. |

returns:

- None

<a id="teleport"></a>

### def teleport(self, nearbyMBRef, position, direction):

Function description:
Instantly move anEntityto a
specified space. This function allows you to specify the position and
orientation of the entity after is has been moved.
If you need to jump in differentspaces(usually for different scene or room jumps), you can pass aCellEntityCallto this function (the entity corresponding to the entityCall must be
in the destinationSpace).
This function can only be called onrealentities.

parameters:

| nearbyMBRef | ACellEntityCall(the entity corresponding to this entityCall must be in the destinationSpace) that determines whichSpaceanEntityis to jump to. It is considered to be the transfer destination. This can be set to None, in which case it will teleport on the current cell. |
| --- | --- |
| position | A sequence of 3 floats (x, y, z), the coordinates of where to teleport theEntity. |
| direction | A sequence of 3 floats (roll, pitch, yaw), the orientation of theEntityafter teleportation. |

<a id="writeToDB"></a>

### def writeToDB(self, shouldAutoLoad, dbInterfaceName):

Function description:
This function saves the data related to this entity to the database,
including the data of the base entity. The onWriteToDB function of the
base entity is called before the data is passed to the database.
The data of the cell entity is also backed up in the base entity to
ensure that the data is up-to-date when crash recovery data is encountered
This function can only be called onrealentities, and the entity must exist in the base section.

parameters:

| shouldAutoLoad | bool, optional parameter, specifies whether this entity needs to be loaded from the database when the service starts. Note: The entity is automatically loaded when the server starts. The default is to call createEntityAnywhereFromDBID to create an entity to a minimally loaded baseapp. The entire process will be completed before the first started baseapp calls onBaseAppReady. The script layer can reimplement the entity creation method in a customized script (kbengine_defaults.xml->baseapp->entryScriptFile definition), for example: def onAutoLoadEntityCreate(entityType, dbid): KBEngine.createEntityFromDBID(entityType, dbid) |
| --- | --- |
| dbInterfaceName | string, optional parameter, specified by a database interface, uses the interface name "default" by default. The database interface is defined in kbengine_defaults.xml->dbmgr->databaseInterfaces. |

<a id="getWitnesses"></a>

### def getWitnesses(self):

Function description:
This function returns all other entities(Players) that observe thisEntity.

returns:

- tuple, an array of zero or moreEntity.

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

<a id="onDestroy"></a>

### def onDestroy(self):

If this function is implemented in a
script, it is called afterEntity.destroy()
destroys this entity. This function has no parameters.

<a id="onEnterTrap"></a>

### def onEnterTrap(self, entity, rangeXZ, rangeY, controllerID, userArg):

When a scope trigger is registered usingEntity.addProximityand another entity enters the trigger, this callback function is called.

parameters:

| entity | Entity that has entered the area |
| --- | --- |
| rangeXZ | float, the size of the xz axis of the trigger, must be greater than or equal to zero. |
| rangeY | float, the size of the y-axis height of the trigger, must be greater than or equal to zero. It should be noted that for this parameter to take effect you must enablekbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y Opening y-axis management will increase CPU consumption, because some games have a large number of entities at the same y-axis height or on the ground at nearly the same height. Because of this, the collision becomes very dense. 3D space games or small room-type games are more suitable for enabling this option. |
| controllerID | The controller id of this trigger. |
| userArg | The value of the parameter given by the user when callingaddProximity, the user can decide how to use this parameter. |

<a id="onEnteredView"></a>

### def onEnteredView(self, entity):

If this function is implemented in a
script, when an entity enters theViewscope of the current entity, this callback is triggered.

parameters:

| entity | The entity which has entered the View scope. |
| --- | --- |

<a id="onGetWitness"></a>

### def onGetWitness(self):

If this function is implemented in a script,
it is called when the entity has aWitnessbound to it.
You can also access the entity property[Entity.hasWitness](#hasWitness)to get the current state of the entity.

<a id="onLeaveTrap"></a>

### def onLeaveTrap(self, entity, rangeXZ, rangeY, controllerID, userArg):

If this function is implemented in a
script, it is triggered when an entity leaves the trigger area registered
by the current entity. The scope trigger is registered withEntity.[addProximity](#addProximity).

parameters:

| entity | The entity that has left the trigger area. |
| --- | --- |
| rangeXZ | float, the size of the xz axis of the trigger, must be greater than or equal to zero. |
| rangeY | float, the size of the y-axis height of the trigger, must be greater than or equal to zero. It should be noted that for this parameter to take effect you must enablekbengine_defaults.xml->cellapp->coordinate_system->rangemgr_y Opening y-axis management will increase CPU consumption, because some games have a large number of entities at the same y-axis height or on the ground at nearly the same height. Because of this, the collision becomes very dense. 3D space games or small room-type games are more suitable for enabling this option. |
| controllerID | The controller ID of this trigger. |
| userArg | The value of the parameter given by the user when callingaddProximity, the user can decide how to use this parameter. |

<a id="onLoseControlledBy"></a>

### def onLoseControlledBy(self, id):

If this function is implemented in a
script, this callback is triggered when this entity loses theEntity.controlledByentity.

parameters:

| id | ID of the controlledBy entity. |
| --- | --- |

<a id="onLoseWitness"></a>

### def onLoseWitness(self):

If this function is implemented in a
script, the callback is triggered whe this entity loses aWitness.
You can also access that[Entity.hasWitness](#hasWitness)property
to get the current state.

<a id="onMove"></a>

### def onMove(self, controllerID, userData):

If this function is implemented in the
script, the callback is invoked each frame when moved after a call toEntity.moveToPoint,Entity.moveToEntity, orEntity.navigate.

parameters:

| controllerID | The controller ID associated with the move. |
| --- | --- |
| userData | The parameter given by the user when requesting to move the entity. |

<a id="onMoveOver"></a>

### def onMoveOver(self, controllerID, userData):

If this callback function is implemented in a
script, it is invoked after a call toEntity.moveToPoint,Entity.moveToEntity, orEntity.navigatewhen this entity reaches the target point.

parameters:

| controllerID | The controller ID associated with the move. |
| --- | --- |
| userData | This parameter value is given by the user when requesting to move an entity. |

<a id="onMoveFailure"></a>

### def onMoveFailure(self, controllerID, userData):

If this function is implemented in the
script, this callback is invoked after a call toEntity.moveToPoint,Entity.moveToEntity, orEntity.navigateif the movement has failed.

parameters:

| controllerID | The controller ID associated with the move. |
| --- | --- |
| userData | This parameter value is given by the user when requesting to move an entity. |

<a id="onRestore"></a>

### def onRestore(self):

If this callback function is implemented in a
script, it is invoked when the Cell application crashes and recreates
the entity on another cellapp. This function has no arguments.

<a id="onSpaceGone"></a>

### def onSpaceGone(self):

If this callback function is implemented in the
script, it will be called when the current entity'sSpaceis destroyed. This function has no parameters.

<a id="onTurn"></a>

### def onTurn(self, controllerID, userData):

If this callback function is implemented
in a script, it will be called after reaching the specified yaw. (related toEntity.addYawRotator)

parameters:

| controllerID | The controller ID returned byEntity.addYawRotator. |
| --- | --- |
| userData | This parameter value is given by user when requesting to move an entity. |

<a id="onTeleport"></a>

### def onTeleport(self):

If this callback function is implemented
in a script, it will be called at the moment before the (Real) entity is
transmitted in the entity transfer that occurs through the baseapp's
Entity.teleport call.
Note: Calling teleport on the entity's cell section does not trigger
this callback, if you need this feature please invoke this callback after
a call toEntity.teleport.

<a id="onTeleportFailure"></a>

### def onTeleportFailure(self):

If this callback function is implemented in a
script, it will be called after a call toEntity.teleportif the teleport has failed.

<a id="onTeleportSuccess"></a>

### def onTeleportSuccess(self, nearbyEntity):

If this callback function is implemented
in a script, it is invoked after a succesful call toEntity.teleport

parameters:

| nearbyEntity | This parameter is given by the user when callingEntity.teleport. This is a real entity. |
| --- | --- |

<a id="onTimer"></a>

### def onTimer(self, timerHandle, userData):

Function description:
This function is called when a timer associated with this entity is triggered.
A timer can be added using theEntity.[addTimer](#addTimer)function.

parameters:

| timerHandle | The ID of the timer. |
| --- | --- |
| userData | integer, given by the user when callingEntity.[addTimer](#addTimer). |

<a id="onUpdateBegin"></a>

### def onUpdateBegin(self):

Invoked when a synchronization frame begins.

<a id="onUpdateEnd"></a>

### def onUpdateEnd(self):

Invoked after a synchronization frame
has completed.

<a id="onWitnessed"></a>

### def onWitnessed(self, isWitnessed):

If this callback function is implemented
in a script, it is called when this entity enters the View area of
another entity bound to a Witness (also can be understood as when this
entity is observed by a client). This function can be used to activate
the entity's AI when it is observed, and stopping AI execution when the
entity ceases to be observed, thus reducing CPU consumption of the server
to increase efficiency.

parameters:

| isWitnessed | bool, True if the entity is observed and False when the entity is not observed. You can also access the entity property[Entity.isWitnessed](#isWitnessed)to get the current state of the entity. |
| --- | --- |

<a id="onWriteToDB"></a>

### def onWriteToDB(self):

If this callback function is implemented
in a script, it is called when the entity is about to be archived into
the database.

## Attributes documentation

<a id="allClients"></a>

allClients

By calling the entity's remote client
methods through this attribute, the engine broadcasts the message to all
other entities bound to a client that are within this entity's View area
(including its own client, and the entity bound to the client is usually
the player)
Example:
Avatar has player A, player B, and monster C in the View range.
avatar.allClients.attack(monsterID, skillID, damage)
At this point, the player himself, player A's, and player B's clients will
all call the entity's attack method, and their client can invoke the
specified skill's attack action to perform.

Other references:

- Entity.clientEntity Entity.otherClients

<a id="base"></a>

base

base is the entityCall used to contact
the baseEntity. This attribute
is read-only and is None if the entity has no associated baseEntity.

Other references:

- Entity.clientEntity Entity.allClients Entity.otherClients

Type:

- Read-only,ENTITYCALL

<a id="className"></a>

className

The class name of the entity.

Type:

- Read-only, string

<a id="client"></a>

client

client is the entityCall used to contact
associated client. This attribute is read-only, and is None if this entity
does not have an associated client.

Other references:

- Entity.clientEntity Entity.allClients Entity.otherClients

Type:

- Read-only,ENTITYCALL

<a id="controlledBy"></a>

controlledBy

If this attribute is set to the
BaseEntityCall of the server-side entity associated with a client, this
entity is controlled by the corresponding client to move. If the attribute
is None, the entity is moved by the server. When the client logs in and
calls giveClientTo on this entity, this attribute is automatically set to
its own BaseEntityCall.
Scripts can flexibly control the movement of the entity by the server or
by the client (its own client or give control to other clients).

Other references:

- Entity.onLoseControlledBy

Type:

- BaseEntityCall

<a id="direction"></a>

direction

This attribute describes the orientation
of theEntityin world space. Users
can change this attribute and the data will be synchronized to the client.

Example:
self.direction.y = 1.0
self.direction.z = 1.0

Type:

- Vector3, which contains (roll, pitch, yaw) in radians.

<a id="hasWitness"></a>

hasWitness

If this read-only attribute is True,
it means that the entity has already bound aWitness.
If the entity is bound toWitness,
the client can obtain information from the entity's view scope. Otherwise, False.

Type:

- Read-only, bool

<a id="id"></a>

id

id is the id of theEntityobject. This id is an integer that is the same between base, cell,
and client associated entities. This attribute is read-only.

Type:

- Read-only, int32

<a id="isDestroyed"></a>

isDestroyed

If this attribute is True, thisEntityhas already been destroyed.

Type:

- Read-only, bool

<a id="isOnGround"></a>

isOnGround

If the value of this attribute is True,
theEntityis on the ground, otherwise
it is False.

Type:

- Read-only, bool

<a id="isWitnessed"></a>

isWitnessed

If the current entity is in the View
scope of another entity bound to Witness (can also be understood as an
entity observed by a client), this property is True, otherwise it is False.
Other references:

- Entity.onWitnessed

Type:

- Read-only, bool

<a id="layer"></a>

layer

A space can load multiple navmesh data
at the same time. Different navmesh can be in different layers. Different
layers can be abstracted into the ground, the water surface, and so on.
This attribute determines which layer an entity exists in.
Reference:
KBEngine.addSpaceGeometryMapping

Type:

- int8

<a id="otherClients"></a>

otherClients

By calling the entity's remote client
methods through this property, the engine broadcasts the message to all
other entities bound to the cliend within this entity's View scope (Not
including its own client. The entity bound to the client is usually the player.).
Example:
avatar has player A, player B, and monster C in the View range.
avatar.otherClients.attack(monsterID, skillID, damage)
At this point, player A's and player B's client will call the entity
attack method, and their client can invoke the specified skill's attack
action to perform.

Other references:

- Entity.clientEntity Entity.otherClients

<a id="position"></a>

position

The coordinates of this entity in world
space (x, y, z). This attribute can be changed by the user and will be
synchronized to the client after the change. It is important to note that this
attribute should not be referenced. Referencing this attribute is likely
to incorrectly modify the real coordinates of the entity.
Example:

```python
self.position.y = 10.0
```

If you want to copy this attribute value you can do the following:

```python
	import Math
	self.copyPosition = Math.
Vector3
( self.position )
```

Type:

- Vector3

<a id="spaceID"></a>

spaceID

This attribute is the ID of thespacein which the entity is located. The cell and client ids are the same.

Type:

- Read-only, Integer

<a id="topSpeed"></a>

topSpeed

The maximum xz movement speed
of the entity (m/s). This attribute is usually larger than the actualy
movement speed. The server checks the client's movement legality through
this attribute. If the movement distance excedes the speed limit, it is
forced back to the previous position.

Other references:

- Entity.topSpeedY

Type:

- float

<a id="topSpeedY"></a>

topSpeedY

The maximum y-axis movement speed
of the entity (m/s). This attribute is usually larger than the actualy
movement speed. The server checks the client's movement legality through
this attribute. If the movement distance excedes the speed limit, it is
forced back to the previous position.

Other references:

- Entity.topSpeed

Type:

- float

<a id="volatileInfo"></a>

volatileInfo

This attribute specifies theEntity's
volatile data synchronization policy.
Volatile data includes the coordinate position of the entity and the
orientation of the entity. Since volatile data is easily changed, the
engine uses a set of optimized solutions to synchronize it to the
client.
This attribute is four floats (position, yaw, pitch, roll) that represents
the distance value, and the server synchronizes the relevant data to it
when an entity reaches a close distance. If the distance value is larger
than the View radius, it means that it is always synchronized
There is also a special bool attribute that is optimized. Its role is to
control whether or not the server is optimized for synchronization. The
current main optimization is the Y axis.
If true, the server does not synchronize the y-axis coordinates of the
entity when some actions (e.g., navigate) cause the server to determine
the entity is on the ground. This can save a lot of bandwidth when
synchronizing a large number of entities. The default is true.
Users can also set the synchronization policies for different entities in .def:

```python
<Volatile>
    <position/>           <!-- always synchronize -->
    <yaw/>                <!-- always synchronize -->
    <pitch>20</pitch>     <!-- synchronize within 20m or less -->
    <optimized> true </optimized>   
</Volatile>               <!-- roll is always synchronized if not specified  -->
```

Type:

- sequence, four floats (float, float, float, float)
