# BigWorld Cell Entity

> The BigWorld Cell Entity is the spatial entity object that runs on the CellApp. It shares its origin with the KBEngine Cell Entity, but the BigWorld side carries more AOI, controller, navigation, and recording capabilities.

## Main source files

| Source location | Description |
|-----------------|-------------|
| `server/cellapp/entity.cpp` | Common Cell Entity methods and attributes |
| `server/cellapp/real_entity.cpp` | Attributes and methods specific to real entities |
| `server/cellapp/witness.cpp` | Witness, AOI, and client visibility methods |
| `server/cellapp/timer_controller.cpp` | Entity-level timers |
| `server/cellapp/proximity_controller.cpp` | Proximity controller |
| `server/cellapp/turn_controller.cpp` | Yaw rotator |
| `server/cellapp/visibility_controller.cpp` | Visibility controller |

## Recognized API groups

- Lifecycle and destruction: `destroy()`
- Timers and controllers: `addTimer()`, `cancel()`, `addProximity()`, `addYawRotator()`
- Witness / AOI: `dumpAoI()`, `setAoIRadius()`, `entitiesInAoI()`, `setAoIUpdateScheme()`
- Client call entry points: `client`, `ownClient`, `otherClients`, `allClients`
- Real entity attributes: `controlledBy`, `isWitnessed`, `hasWitness`, `shouldAutoBackup`

## Next steps

Witness/AOI and timers should be filled in first, because they are the easiest to misread against the KBEngine [CellApp Entity](../kbengine/cellapp/Entity.md).