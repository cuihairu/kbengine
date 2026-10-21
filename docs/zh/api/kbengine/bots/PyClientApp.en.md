# PyClientApp class

[KBEngine module]

PyClientApp is a part of theKBEngineModule. It is is client object created when a client is simulated from
the bottom of C++. It cannot be created in the script layer directly.

## Member functions

- def [getSpaceData](#getSpaceData)( key ):
- def [player](#player)( ):

## Callbacks

## Attributes

| [id](#id) | Read-only Integer |
| --- | --- |
| [entities](#entities) | Entities |

<a id="detailed_description"></a>

## A detailed description

Instances of classEntityrepresent
game objects on the client.
AnEntitycan access its equivalent
entities in the base and cell applications viaENTITYCALL.
This requires a set of remotely-invoked functions (specified in the entity's .def file).

## Member functions documentation

<a id="getSpaceData"></a>

### def getSpaceData(key):

Function description:
Get the space data of the specified key.
The space data is set by the user on the server throughsetSpaceData.

parameters:

| key | string, a string keyword. |
| --- | --- |

returns:

- string, string data for the key

<a id="player"></a>

### def player():

Function description
Gets the entity that the current client controls.

## Callback functions documentation

## Attributes documentation

<a id="entities"></a>

entities

Description:
entities is a dictionary object that contains all the entities in the
current process.

Types:

- Entities
