# Keywords Used

<a id="EntityCall"></a>

### EntityCall:

This is the conventional means of remote interaction between entities in
the script layer. (other references:allClients,otherClients,clientEntity).
The EntityCall object is very simple to implement in the underlying C++, it contains only the ID of the entity,
the address of the destination, the entity type, and type of EntityCall.
When a remote call is requested by the user,
the engine first finds a description of the entity definition through the entity type,
and checks the data input by the user against the description of the entity definition.
If the check is legal, then the data is packaged and sent to the destination
according to the protocol.
Note: An EntityCall can only be used to call methods declared in its corresponding
def file. It cannot call any of KBEngine's basic Entity class functions or
access entity attributes.
An Entity can contain up to three parts:
Client: When an entity includes a client part (usually a
player), the entity's client (EntityCall) property can be accessed on the
server side.
Base: When an entity includes a baseapp part, the base (EntityCall)
attribute of the entity can be accessed in a non-current baseapp.
Cell: When an entity includes a cellapp part, the cell (EntityCall)
attribute of the entity can be accessed in a non-current cellapp.
Example:
Client remote method defined in Avatar.def:

```python
	[ClientMethods]
		[hello]
		[/hello]
	[/ClientMethods]
```

client\Avatar.py

```python
	class Avatar:
		def hello(self):
			print("hello")
```

Enter in the Debug page input box of the GUIConsole tool (check the
process to be debugged first in the list on the left):
First find the ID of the player entity (Avatar) in the log of the server's
Baseapp, and then get the player entity (Avatar) or EntityCall through
the entity ID:
>>> KBEngine.entities[Player ID].client.hello()
At this point, the client log file will output "hello" and a remote call
process will be completed.

<a id="KBE_ROOT"></a>

### KBE_ROOT:

This is a KBEngine environment variable that describes the root directory
where KBEngine is located.

<a id="KBE_RES_PATH"></a>

### KBE_RES_PATH:

This is a KBEngine environment variable that describes the resource
directory that KBEngine engine can read.

<a id="KBE_HYBRID_PATH"></a>

### KBE_HYBRID_PATH:

This is a KBEngine environment variable that describes the directory
where the KBEngine engine executable file is located.

<a id="entities.xml"></a>

### entities.xml:

All valid entity types on the server must be registered here. When the
engine is initialized, the description of the entity is loaded according
to the order.

<a id="kbengine_defaults.xml"></a>

### kbengine_defaults.xml:

Server-side default configuration, where users can modify all component
configurations such as[cellapp](./cellapp/index.md),[baseapp](./baseapp/index.md), and[loginapp](./loginapp/index.md).
Note: You may often need to upgrade the engine. Modifying directly may
cause conflicts during the upgrade, and it is not suitable for multiple
projects in the same KBEngine environment.
It is recommended that you modify the overload in[kbengine.xml](#kbengine.xml). You only need to rewrite the
parts you want to modify according to the format in xml.

<a id="kbengine.xml"></a>

### kbengine.xml:

Server configuration file, where users can modify all component configurations
such as[cellapp](./cellapp/index.md),[baseapp](./baseapp/index.md), and[loginapp](./loginapp/index.md).
For details, please refer to[kbengine_defaults.xml](#kbengine_defaults.xml)

<a id="Entity"></a>

### entity

Entity is defined as the most basic object of the server, similar to
Python's base object.
When and how should you define an entity? See[http://www.kbengine.org/docs/programming/entitydef.html](http://www.kbengine.org/docs/programming/entitydef.html)

<a id="View"></a>

### View

Each client entity connected to the server will have a View. It allows
the client entity to communicate events in its View to its own client.
View is related to space, and each View can be set to an independent size
range.
Note: The space described here is an abstract concept and does not
necessarily need to be bound to the concept of physical space (except
for MMORPGs). For the core gameplay of a card game, players in a room can
also be considered to be in a logical space.
Events include: entity movement, property change of client broadcast type,
destruction on death, and so on.

<a id="Witness"></a>

### Witness

Eyewitness.
Only Witnesses bound to[cell](#cell)entity Views take effect.
In other words, Witness is a[cell](#cell)proxy of the client.
The cellapp continuously synchronizes the information in the View to the
client through the Witness.
When an NPC on the server is seen by a witness, it call the onWitness
callback of the entity. The server can rely on this feature to reduce CPU
consumption. When an entity is not witnessed, the server can stop any of
its behavior.

<a id="Space"></a>

### Space

A space KBEngine allocates on the cellapp, which is isolated
from other spaces. Views, traps, entity collisions, etc. only interact with each
other in the current space. The space is defined by the user. It can be
a scene, copy, room...

<a id="cell"></a>

### cell

There are two different meanings of cell in this documentation.
Usually, when referring to theEntity.cellattribute, you are actually describing the entity's[CellEntityCall](#EntityCall).
If a cell is described as part of a[space](#Space), it refers to[cellapp](./cellapp/index.md)'s load
balancing technique. A space in the cellapp may be divided into n parts,
each called a cell, and each cell is maintained by a different process.

<a id="base"></a>

### base

Usually refers to the Base entity on the[baseapp](./baseapp/index.md)or a[BaseEntityCall](#EntityCall)that points to the Base entity.
For example:Entity.base

<a id="client"></a>

### client

Usually refers to the client or an[EntityCall](#EntityCall)that points to the client entity.
For example:Entity.client

<a id="cellapp"></a>

### cellapp

The Cellapp process in mainly responsible for position-related game logic,
View, AI, scene rooms, and so on.
See also:[cellapp](./cellapp/index.md)

<a id="baseapp"></a>

### baseapp

The Baseapp process is mainly responsible for communication with the client,
position-independent game logic (guild manager, chat system, game lobby,
leaderboard, etc.), archiving, backup, and so on.
See also:[baseapp](./baseapp/index.md)

<a id="real"></a>

### real

Refers to an entity in a cell that is actually present in the cell at that
time. (As opposed to a[ghost](#ghost)entity broadcast there by another cell)

<a id="ghost"></a>

### ghost

This kind of entity is a projected copy generated by cellapp's dynamic load
balancing mechanism which divides a space into N shares and splits the cells
between different processes.
Space is divided into multiple regions. To make the client unable to
perceive the existence of the boundaries between them, we synchronize a
certain range of entities in each cell's boundary to an adjacent cell's
boundary. The entity has a part of its data synchronized over
(CELL_PUBLIC, cell broadcast types of attributes) to a ghost entity. In
this way the entity can interact seamlessly with the other side of the boundary
and both cells simultaneously.
Non-ghost entities are called[real](#real)entities.

<a id="vector3"></a>

### vector3

Describe and manage 3D space vectors.
There are three properties of x, y, and z that represent different axial directions.
Example in script: import Math v = Math.Vector3()
