# BigWorld BWPersonality

> `BWPersonality` is the process-level callback entry for BigWorld server scripts. Its role is close to the `onInit`, `onReadyForLogin`, `on*AppReady`, and `on*ShutDown` callback sets found in the KBEngine component modules.

## Main source files

| Component | Source location |
|-----------|-----------------|
| Common | `lib/server/bwpersonality.mpp` |
| BaseApp | `server/baseapp/bwpersonality.mpp` |
| CellApp | `server/cellapp/bwpersonality.mpp` |

## Recognized callback groups

- Common lifecycle: `onInit`, `onFini`
- BaseApp: `onBaseAppReady`, `onAppReady`, `onBaseAppShuttingDown`, `onBaseAppShutDown`
- CellApp: `onCellAppReady`, `onAppReady`, `onCellAppShuttingDown`
- Component death notifications: `onBaseAppDeath`, `onCellAppDeath`, `onServiceAppDeath`
- Global data: `onBaseAppData`, `onDelBaseAppData`, `onCellAppData`, `onDelCellAppData`, `onGlobalData`, `onDelGlobalData`
- Space events: `onSpaceData`, `onSpaceDataDeleted`, `onSpaceGeometryLoaded`

## Next steps

The BigWorld `BWPersonality` should be organized as a process-level script entry of its own instead of being folded into the `BigWorld` module function page. That keeps the distinction clear between "APIs the script calls" and "hooks the engine calls into scripts".