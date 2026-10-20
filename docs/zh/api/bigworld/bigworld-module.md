# BigWorld Module

> This page is the source-extracted entry for the BigWorld `BigWorld` Python module. Concrete functions will later be split out by component: BaseApp, CellApp, DBApp, Bots, and so on.

## Main source files

| Component | Source location | Description |
|-----------|-----------------|-------------|
| BaseApp | `server/baseapp/bigworld.mpp` | Overview of the `BigWorld` module on the Base side |
| BaseApp | `server/baseapp/script_bigworld.cpp` | Module functions for Base creation, DBID lookup, services, hotfix, and shutdown |
| CellApp | `server/cellapp/bigworld.mpp` | Overview of the `BigWorld` module on the Cell side |
| CellApp | `server/cellapp/cellapp.cpp` | Module functions for Cell-side entity creation, time, load, hotfix, and shutdown |
| CellApp | `server/cellapp/space.cpp` | Space-level module functions for SpaceData, space geometry, recording, and time |
| Common | `lib/server/app_script_timers.cpp` | App-level `BigWorld.addTimer()` / `BigWorld.delTimer()` |

## Function groups to fill in first

- `BigWorld.reloadScript(fullReload=True)`
- `BigWorld.addTimer(initialOffset, repeatOffset, callback, userArg=0)`
- `BigWorld.delTimer(timerID)`
- `BigWorld.createBase*`
- `BigWorld.createEntity*`
- `BigWorld.addSpaceData` / `delSpaceData` / `getSpaceData` / `setSpaceData`
- `BigWorld.globalData` / `baseAppData` / `cellAppData`

## Relationship to KBEngine

The BigWorld module is named `BigWorld`, while the KBEngine module is named `KBEngine`. Many function names are similar, but the implementation boundaries are not identical. In particular, app-level timers, `reloadScript`, global data, service registration, and SpaceData should be compared item by item instead of being mapped by name alone.