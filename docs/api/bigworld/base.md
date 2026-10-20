# BigWorld Base

> BigWorld Base is the entity script object that runs on the BaseApp. It shares its origin with the KBEngine Base entity, but exposes a richer API surface and runtime semantics.

## Main source files

| Source location | Description |
|-----------------|-------------|
| `server/baseapp/base.hpp` | `/*~ class BigWorld.Base */` class description and attribute declarations |
| `server/baseapp/base.cpp` | Comments and implementations for `Base.destroy`, `createCellEntity`, `addTimer`, `writeToDB`, and other methods |

## Recognized API groups

- Lifecycle: `destroy()`, `onDestroy`
- Cell creation: `createCellEntity()`, `createInDefaultSpace()`, `createInNewSpace()`, `destroyCellEntity()`
- Timers: `addTimer()`, `delTimer()`
- Persistence: `writeToDB()`
- Global registration: `registerGlobally()`, `deregisterGlobally()`
- Attributes: `id`, `databaseID`, `baseType`, `className`, `cell`, `hasCell`, `cellData`, `isDestroyed`

## Next steps

This page should later extract signatures, parameters, return values, and caveats from the `/*~ function Base.* */` and `/*~ attribute Base.* */` comments, and compare them with the KBEngine [BaseApp Entity](api/kbengine/baseapp/Entity.md.