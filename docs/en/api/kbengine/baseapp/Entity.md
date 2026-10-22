# Entity class

[KBEngine module]

Entity is part of theKBEnginemodule.[More...](#detailed_description)

```python
import KBEngine
```

## Member functions

- def [addTimer](#addTimer)( self, initialOffset, repeatOffset=0, userArg=0 ):
- def [createCellEntity](#createCellEntity)( self, cellEntityCall ):
- def [createCellEntityInNewSpace](#createCellEntityInNewSpace)( self, cellappIndex ):
- def [delTimer](#delTimer)( self, id ):
- def [destroy](#destroy)( self, deleteFromDB, writeToDB ):
- def [destroyCellEntity](#destroyCellEntity)( self ):
- def [writeToDB](#writeToDB)( self, callback, shouldAutoLoad, dbInterfaceName ):
- def [getComponent](#getComponent)( self, componentName, all ):
- def [fireEvent](#fireEvent)( self, eventName, *args ):
- def [registerEvent](#registerEvent)( self, eventName, callback ):
- def [deregisterEvent](#deregisterEvent)( self, eventName, callback ):

## Callback functions

- def [onCreateCellFailure](#onCreateCellFailure)( self ):
- def [onDestroy](#onDestroy)( self ):
- def [onGetCell](#onGetCell)( self ):
- def [onLoseCell](#onLoseCell)( self ):
- def [onPreArchive](#onPreArchive)( self ):
- def [onRestore](#onRestore)( self ):
- def [onTimer](#onTimer)( self, timerHandle, userData ):
- def [onWriteToDB](#onWriteToDB)( self, cellData ):

## Attributes

| [cell](#cell) | Read-onlyCellEntityCall |
| --- | --- |
| [cellData](#cellData) | CELLDATADICT |
| [className](#className) | Read-onlystring |
| [client](#client) | Read-onlyClientEntityCall |
| [databaseID](#databaseID) | Read-onlyint64 |
| [databaseInterfaceName](#databaseInterfaceName) | Read-onlystring |
| [id](#id) | Read-onlyint32 |
| [isDestroyed](#isDestroyed) | bool |
| [shouldAutoArchive](#shouldAutoArchive) | True, False orKBEngine.NEXT_ONLY |
| [shouldAutoBackup](#shouldAutoBackup) | True, False orKBEngine.NEXT_ONLY |

<a id="detailed_description"></a>

## A detailed description

ThisEntityclass represents anEntitythat resides on theBaseapp.Entitiescan be created using theKBEngine.createEntityfunction (as well as functions prefixed by createEntity).
AnEntitycan also be remotely created using theCellappfunctionKBEngine.createEntityOnBaseApp.
A baseEntitycan be linked to an activecellentity and can be used to create an associatedcellentity.
This class allows you to create and destroycellentities, register a timer on the base entity, or access the contact
information for this object.
You can also access aCellEntityCallthrough which this base entity can communicate with itscellentity (the associatedcellentity
can be moved to a differentcellwhen KBEngine performs load balancing as a result of the movement of thecellentity).

## Member function documentation

<a id="addTimer"></a>

### def addTimer(self, initialOffset, repeatOffset=0, userArg=0):

Function description:
Register a timer. The timer is triggered by the callback function[onTimer](#onTimer), which will be executed the first
time after "initialOffset" seconds, and then executed once every "repeatOffset"
seconds. A "userArg" parameter can be set (integer only).
The[onTimer](#onTimer)function must be defined in the
base part of the entity with two parameters. The first is an integer,
the timer id (which can be used to remove the timer's "[delTimer](#delTimer)"
function), and the second is the user parameter "userArg".
Example:

```python
# Here is an example of using addTimer
import 
KBEngine

 
class MyBaseEntity( 
KBEngine
.
Entity
 ):
 
    def __init__( self ):
        
KBEngine
.
Entity
.__init__( self )
 
        # Add a timer, trigger for the first time after 5 seconds, and execute once per second afterwards. The user parameter is 9.
        self.addTimer( 5, 1, 9 )
 
        # Add a timer and execute it once after 1 second. The default user parameter is 0.
        self.addTimer( 1 )
 
    # 
Entity
 timer callback "onTimer" is called
    def onTimer( self, id, userArg ):
        print "MyBaseEntity.onTimer called: id %i, userArg: %i" % ( id, userArg )
        # If this is a repeated timer, when the timer is no longer needed, call the following function to remove it:
        #     self.delTimer( id )
```

parameters:

| initialOffset | float, specifies the time interval in seconds for the timer to trigger the first callback. |
| --- | --- |
| repeatOffset | float, specifies the time interval (in seconds) after each execution of the first callback execution. You must remove the timer with the function[delTimer](#delTimer), otherwise it will continue to repeat. Values less than or equal to 0 will be ignored. |
| userArg | integer, specifies the value of the userArg parameter when invoking the "[onTimer](#onTimer)" callback. |

returns:Z

- integer, the internal id of the timer. This id can be used to remove the timer using[delTimer](#delTimer).

<a id="createCellEntity"></a>

### def createCellEntity(self, cellEntityCall):

Function description:
Requests to create an associated entity in acell.
The information used to create thecellentity is stored in the entity's cellData attribute. The cellData attribute is a
dictionary that corresponds to the default value in the entity's .def file,
as well as the "position", "direction", and "spaceID" used to represent
the entity's position and orientation (roll, pitch, yaw).

parameters:

| cellEntityCall | CellEntityCallparameter that specifies which space to create this cell entity in. Only a directCellEntityCallmay be used. If you have an entity'sBaseEntityCall, you cannot pass itsbaseEntityCall.cellto this function. Instead, you must create a new function on the current entity's base that accepts a directCellEntityCallas a parameter and then calls this function using it. E.g. ```python baseEntityCallOfNearbyEntity.createCellNearSelf( self ) ``` On the nearby entity's base: ```python def createCellNearSelf( self, baseEntityCall ): baseEntityCall.createCellNearHere( self.cell ) ``` On the current entity's base: ```python def createCellNearHere( self, cellEntityCall ): self.createCellEntity( cellEntityCall ) ``` |
| --- | --- |

<a id="createCellEntityInNewSpace"></a>

### def createCellEntityInNewSpace(self, cellappIndex):

Function description:
Create a space on the cellapp and create thecellof this entity into the new space. It requests to complete through cellappmgr.
The information used to create the cell entity is stored in the entity's cellData attribute. This property is a dictionary. The default values in the corresponding entity's .def file also include
"position", "direction", and "spaceID" for representing the entity's position and orientation (roll, pitch, yaw).

parameters:

| cellappIndex | integer, if it is either None or 0, a cellapp is dynamically selected by the engine load balancer. If it is greater than 0, a space is created in the specified cellapp Example: If you expect to open four cellapps, then the cellappIndex needs to specify the index can be 1, 2, 3, 4, if the actual running cellapp is less than 4, for example, only 3, then the cellappIndex input 4 due to the number of overflow 4 1, 5 2. Tip: This feature can be used in conjunction with KBEngine.setAppFlags (KBEngine.APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING), for example: placing large map spaces in several fixed cellapps and setting these cellapps to not participate in load balancing, and other cellapps to place copy space. When the copyspace is created and the cellappIndex is set to 0 or None, the consumption of the copy map will not affect the large map process, thus ensuring the smoothness of the main scene. |
| --- | --- |

<a id="delTimer"></a>

### def delTimer(self, id):

Function description:
The function delTimer is used to remove a registered timer. The removed
timer is no longer executed. Single-shot timers are
automatically removed after the callback is executed, and it is not
necessary to use the delTimer to remove it. If the delTimer function
uses an invalid id (for example, has been removed), it will generate an
error.
A usage example is with theEntity.addTimerfunction.

parameters:

| id | integer, which specifies the timer id to remove. If the parameter is the string "All", all timers are removed at once. |
| --- | --- |

<a id="destroy"></a>

### def destroy(self, deleteFromDB, writeToDB):

Function description:
This function destroys the base parts of the entity. If the entity has a
cell part, then the user must first destroy the cell part, otherwise it
will generate an error. To destroy the cell part of the entity, call aEntity.destroyCellEntity.
It may be more appropriate to call self.destroy in the onLoseCell callback.
This ensures that the base part of the entity is destroyed.

parameters:

| deleteFromDB | If True, the entry associated with this entity in the database will be deleted. This parameter defaults to False. |
| --- | --- |
| writeToDB | If True, the archived attributes associated with this entity will be written to the database. Only if this entity is read for the database or usesEntity.writeToDBwill it be written to the database. This parameter is True by default, but will be ignore when deleteFromDB is True. |

<a id="destroyCellEntity"></a>

### def destroyCellEntity(self):

Function description:
destroyCellEntityrequests destruction of the associated cell
entity. This method will generate an error if there is no associated
cell entity.

<a id="writeToDB"></a>

### def writeToDB(self, callback, shouldAutoLoad, dbInterfaceName):

Function description:
This function saves the entity's archive attributes to the database so
that it can be loaded again when needed.
Entities can also be marked as automatically loaded so that the entity
will be re-created when the service is started.

parameters:

| callback | This optional parameter is a callback function when the database operation is complete. It has two parameters. The first is a success or failure boolean flag, and the second is the base entity. |
| --- | --- |
| shouldAutoLoad | This optional parameter specifies whether this entity needs to be loaded from the database when the service is started. Note: The entity is automatically loaded when the server starts. The default is to call createEntityAnywhereFromDBID to create an entity to a minimally loaded baseapp. The entire process will be completed before the first started baseapp calls onBaseAppReady. The script layer can reimplement the entity creation method in the personalization script (kbengine_defaults.xml->baseapp->entryScriptFile definition), for example: def onAutoLoadEntityCreate(entityType, dbid): KBEngine.createEntityFromDBID(entityType, dbid) |
| dbInterfaceName | string, optional parameter, specified by a database interface, default is to use the "default" interface. Database interfaces are defined in kbengine_defaults.xml->dbmgr->databaseInterfaces. |

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

<a id="onCreateCellFailure"></a>

### def onCreateCellFailure(self):

Function description:
If this function is implemented in the script, this function is called when the cell entity fails to create. This function has no parameters.

<a id="onDestroy"></a>

### def onDestroy(self):

Function description:
If this callback function is implemented in a script, it is called afterEntity.destroy()
actually destroys the entity. This function has no parameters.

<a id="onGetCell"></a>

### def onGetCell(self):

Function description:
If this function is implemented in the script, this function is called when it gets a cell entity. This function has no parameters.

<a id="onLoseCell"></a>

### def onLoseCell(self):

Function description:
If this function is implemented in the script, this function is called after its associated cell entity is destroyed. This function has no parameters.

<a id="onPreArchive"></a>

### def onPreArchive(self):

Function description:
If this function is implemented in a script, it is called before the
entity is automatically written to the database. This callback is called
before theEntity.onWriteToDBcallback. If the callback returns False, the archive operation is aborted.
This callback should return True to continue the operation. If this
callback does not exist, the archiving operation continues.

<a id="onRestore"></a>

### def onRestore(self):

Function description:
If this function is implemented in a script, it is called when thisEntity's
application crashes and theEntityis recreated on
other applications. This function has no parameters.

<a id="onTimer"></a>

### def onTimer(self, timerHandle, userData):

Function description:
This function is called when a timer associated with this entity is
triggered. A timer can be added using theEntity.addTimerfunction.

parameters:

| timerHandle | The id of the timer. |
| --- | --- |
| userData | integer, User data passed in onEntity.addTimer. |

<a id="onWriteToDB"></a>

### def onWriteToDB(self, cellData):

Function description:
If this function is implemented in the script, this function is called when the entity data is to be written into the database.
Note that calling writeToDB in this callback will result in an infinite loop.

parameters:

| cellData | Contains the cell properties that will be stored in the database.cellDatais a dictionary. |
| --- | --- |

## Attributes documentation

<a id="cell"></a>

cell

Description:
cell is theENTITYCALLused to
contact the cell entity. This property is read-only, and the property is
set to None if this base entity has no associated cell.

Type:

- Read-onlyENTITYCALL

<a id="cellData"></a>

cellData

Description:
cellData is a dictionary property. Whenever the base entity does not
create its cell entity, the attributes of the cell entity are stored here.
If the cell entity is created, these used values and[cellData](#cellData)attributes will be deleted. In addition to the attributes that the cell
entity specifies in the entity definition file, it also contains position,
direction, and spaceID.

Type:

- CELLDATADICT

<a id="className"></a>

className

Description:
The class name of the entity.

Type:

- Read-only string

<a id="client"></a>

client

Description:
client is the EntityCall used to contact the client. This attribute is
read-only and is set to None if this base entity has no associated client.

Type:

- Read-onlyENTITYCALL

<a id="databaseID"></a>

databaseID

Description:
databaseID is the entity's permanent ID (database id). This id is of type
uint64 and is greater than 0. If it is 0 then the entity is not permanent.

Type:

- Read-only int64

<a id="databaseInterfaceName"></a>

databaseInterfaceName

Description:
databaseInterfaceName is the database interface name where the entity
persists. The interface name is configured in kbengine_defaults->dbmgr.
The entity must be persistent (databaseID>0) for this attribute to be
available, otherwise an empty string is returned.

Type:

- Read-only string

<a id="id"></a>

id

Description:
id is the object id of the entity. This id is an integer that is the same between base, cell, and client associated entities. This attribute is read-only.

Type:

- Read-only int32

<a id="isDestroyed"></a>

isDestroyed

Description:
This attribute is True if theEntityhas been
destroyed.

Type:

- bool

<a id="shouldAutoArchive"></a>

shouldAutoArchive

Description:
This attribute determines the automatic archiving strategy. If set to True,
AutoArchive will be available, if set to False AutoArchive will not be available.
If set toKBEngine.NEXT_ONLY,
automatic archiving will be available at the next scheduled time. This
attribute will be set to false after the next archiving.

Type:

- True, False orKBEngine.NEXT_ONLY

<a id="shouldAutoBackup"></a>

shouldAutoBackup

Description:
This attribute determines the automatic bacup strategy. If set to True,
automatic backup will be available, if set to False, automatic backup
will not be available. If set toKBEngine.NEXT_ONLY,
automatic backup will be done at the next available predetermined time.
After the next backup, this attribute will be set to False.

Type:

- True, False orKBEngine.NEXT_ONLY
