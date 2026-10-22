# API Classification and Audit Checklist

> This checklist organizes the pages under `docs/zh/api/kbengine` by responsibility and audits API wording, parameter semantics, return values, synchronization boundaries, and source implementation by category.

## Audit principles

- Verify the actual API semantics first, then parameters, return values, and call boundaries.
- API pages should retain only interface descriptions, cautions, and links; complex mechanisms belong in `architecture/source-analysis`.
- When the CHM source text, existing wording, and actual source behavior conflict, actual source behavior takes precedence, and the API page should link to the relevant source analysis.
- Concepts that are easy to confuse—`player identity`, `control`, `visibility`, `ownerOnly`, `sync/broadcast`, and `asynchronous callback`—must be explained separately.

## Classification overview

| Category | Page scope | Current focus | Status |
| --- | --- | --- | --- |
| Core concepts | [API overview](/api/kbengine/), [basic types](/api/kbengine/basetypes), [keywords](/api/kbengine/keywords) | Unify basic types and terms such as EntityCall and property descriptors | Not started |
| Client runtime | [client/KBEngine](/api/kbengine/client/KBEngine), [client/Entity](/api/kbengine/client/Entity), [bots/KBEngine](/api/kbengine/bots/KBEngine), [bots/Entity](/api/kbengine/bots/Entity), [bots/PyClientApp](/api/kbengine/bots/PyClientApp) | `player`, `isPlayer`, `isControlled`, visibility callbacks, client movement, and the `ownerOnly` boundary | In progress |
| Server entities | [baseapp/Entity](/api/kbengine/baseapp/Entity), [baseapp/Proxy](/api/kbengine/baseapp/Proxy), [cellapp/Entity](/api/kbengine/cellapp/Entity) | Base/Cell entity responsibilities, `controlledBy`, controllers, AOI, persistence, and client binding | In progress |
| Server global entries | [baseapp/KBEngine](/api/kbengine/baseapp/KBEngine), [cellapp/KBEngine](/api/kbengine/cellapp/KBEngine), [loginapp/KBEngine](/api/kbengine/loginapp/KBEngine), [dbmgr/KBEngine](/api/kbengine/dbmgr/KBEngine), [interfaces/KBEngine](/api/kbengine/interfaces/KBEngine), [logger/KBEngine](/api/kbengine/logger/KBEngine) | Global functions, global data dictionaries, process callbacks, and component boundaries | In progress |
| Cross-cutting topics | Concepts shared by multiple API pages | Player identity/control, lifecycle, timers, global data, property sync, spaces, and controllers | In progress |

## Recommended audit order

1. Client runtime
2. Server entities
3. Server global entries
4. Core concepts

The order is direct:

- Client APIs are most likely to be misread because a name resembles control semantics when it is actually an identity check.
- Server-entity APIs determine the real semantics of client APIs, so auditing them second is most useful.
- Process-level `KBEngine` entries are numerous but mostly concern entities, data dictionaries, and callbacks, so they belong after entity APIs.
- `basetypes` and `keywords` are shared glossaries, best saved for final consolidation.

## Classification details

### 1. Core concepts

Pages:

- [API overview](/api/kbengine/)
- [Basic data types](/api/kbengine/basetypes)
- [Keyword definitions](/api/kbengine/keywords)

Audit focus:

- Whether basic type definitions agree with `.def`, the script bridge, and serialization implementation
- Whether terms such as `ENTITYCALL`, `CELLDATADICT`, and `PyClient` are treated as surface names without runtime semantics
- Whether keyword definitions overlap or contradict one another

Related source analysis:

- [Entities and the entity system](/architecture/source-analysis/entity-system)
- [Scripting and the Python bridge](/architecture/source-analysis/scripting)

Current progress: the terminology matrix has not started yet.

### 2. Client runtime

Pages:

- [client/KBEngine](/api/kbengine/client/KBEngine)
- [client/Entity](/api/kbengine/client/Entity)
- [bots/KBEngine](/api/kbengine/bots/KBEngine)
- [bots/Entity](/api/kbengine/bots/Entity)
- [bots/PyClientApp](/api/kbengine/bots/PyClientApp)

Audit focus:

- The boundary between `player()`, `isPlayer()`, `controlledBy`, and `isControlled`
- The actual trigger semantics of `onEnterWorld()`, `onLeaveWorld()`, `onEnterSpace()`, and `onLeaveSpace()`
- Call restrictions for `baseCall()` and `cellCall()`, including the “only player entities can access them” boundary
- `isOnGround`, movement APIs, client-writable property direction, and synchronization
- Client visibility and callback behavior for `ownerOnly` properties

Related source analysis:

- [Networking and messaging](/architecture/source-analysis/networking#client-entity-isplayer-control)
- [Spaces, AOI, and visibility](/architecture/source-analysis/space-aoi)

Current progress: the identity/control semantic matrix is in progress; world/space callbacks and handle-table semantics have source-analysis links.

### 3. Server entities

Pages:

- [baseapp/Entity](/api/kbengine/baseapp/Entity)
- [baseapp/Proxy](/api/kbengine/baseapp/Proxy)
- [cellapp/Entity](/api/kbengine/cellapp/Entity)

Audit focus:

- Base entity creation of Cell entities, Base→Cell handoff, and callback ordering
- Whether `controlledBy`, `isControlled`, and client binding explain actual control transfer
- Proximity, movement, navigation, and controller ownership and cancellation semantics
- Persistence timing, database callbacks, and failure paths
- The client handoff and Witness-chain semantics behind `Proxy.giveClientTo()`

Related source analysis:

- [Base entity lifecycle](/architecture/source-analysis/base-entity-lifecycle)
- [Entity system](/architecture/source-analysis/entity-system)
- [Networking and messaging](/architecture/source-analysis/networking)
- [Spaces, AOI, and visibility](/architecture/source-analysis/space-aoi)

Current progress: `baseapp/Entity` and `baseapp/Proxy` have source-analysis links; `cellapp/Entity` movement/controller links are in progress.

### 4. Server global entries

Pages:

- [baseapp/KBEngine](/api/kbengine/baseapp/KBEngine)
- [cellapp/KBEngine](/api/kbengine/cellapp/KBEngine)
- [loginapp/KBEngine](/api/kbengine/loginapp/KBEngine)
- [dbmgr/KBEngine](/api/kbengine/dbmgr/KBEngine)
- [interfaces/KBEngine](/api/kbengine/interfaces/KBEngine)
- [logger/KBEngine](/api/kbengine/logger/KBEngine)

Audit focus:

- Entity creation, locality, callbacks, and failure semantics
- Global and process data dictionaries, synchronization timing, and callback dispatch
- Account, login, database, third-party integration, and logging boundaries
- Timer IDs, arguments, and cleanup constraints at the process level

Related source analysis:

- [Entry and bootstrap](/architecture/source-analysis/entry-and-bootstrap)
- [Process model](/architecture/source-analysis/process-model)
- [Networking and messaging](/architecture/source-analysis/networking)
- [Persistence](/architecture/source-analysis/persistence)

Current progress: BaseApp global-data entries and Proxy client handoff have source-analysis links; LoginApp, DBMgr, Interfaces, and Logger are in progress.

### 5. Cross-cutting topics

This layer advances by concepts that span multiple components rather than page by page.

Current topics:

- Player identity and control
- Entity lifecycle and visibility
- Timers and callbacks
- Global data dictionary synchronization
- Property sync, broadcast, and `ownerOnly`
- Movement, controllers, and navigation

Openings already connected:

- [client/KBEngine.player](/api/kbengine/client/KBEngine#player)
- [client/KBEngine.findEntity](/api/kbengine/client/KBEngine#findEntity)
- [client/Entity.baseCall](/api/kbengine/client/Entity#baseCall)
- [client/Entity.cellCall](/api/kbengine/client/Entity#cellCall)
- [client/Entity.isPlayer](/api/kbengine/client/Entity#isPlayer)
- [client/Entity.onEnterWorld](/api/kbengine/client/Entity#onEnterWorld)
- [client/Entity.onEnterSpace](/api/kbengine/client/Entity#onEnterSpace)
- [client/Entity.base](/api/kbengine/client/Entity#base)
- [client/Entity.cell](/api/kbengine/client/Entity#cell)
- [client/Entity.clientapp](/api/kbengine/client/Entity#clientapp)
- [client/Entity.isOnGround](/api/kbengine/client/Entity#isOnGround)
- [client/Entity.inWorld](/api/kbengine/client/Entity#inWorld)
- [bots/PyClientApp.player](/api/kbengine/bots/PyClientApp#player)
- [bots/PyClientApp.entities](/api/kbengine/bots/PyClientApp#entities)
- [bots/Entity.isPlayer](/api/kbengine/bots/Entity#isPlayer)
- [bots/Entity.onEnterWorld](/api/kbengine/bots/Entity#onEnterWorld)
- [bots/Entity.onEnterSpace](/api/kbengine/bots/Entity#onEnterSpace)
- [bots/Entity.base](/api/kbengine/bots/Entity#base)
- [bots/Entity.cell](/api/kbengine/bots/Entity#cell)
- [bots/Entity.clientapp](/api/kbengine/bots/Entity#clientapp)
- [bots/Entity.moveToPoint](/api/kbengine/bots/Entity#moveToPoint)
- [bots/Entity.cancelController](/api/kbengine/bots/Entity#cancelController)
- [bots/Entity.isOnGround](/api/kbengine/bots/Entity#isOnGround)
- [client/KBEngine.entity_id](/api/kbengine/client/KBEngine#entity_id)
- [client/KBEngine.spaceID](/api/kbengine/client/KBEngine#spaceID)
- [baseapp/Entity.createCellEntity](/api/kbengine/baseapp/Entity#createCellEntity)
- [baseapp/Entity.onGetCell](/api/kbengine/baseapp/Entity#onGetCell)
- [cellapp/Entity.controlledBy](/api/kbengine/cellapp/Entity#controlledBy)
- [cellapp/Entity.addTimer](/api/kbengine/cellapp/Entity#addTimer)
- [cellapp/Entity.delTimer](/api/kbengine/cellapp/Entity#delTimer)
- [cellapp/Entity.moveToPoint](/api/kbengine/cellapp/Entity#moveToPoint)
- [cellapp/Entity.navigate](/api/kbengine/cellapp/Entity#navigate)
- [cellapp/Entity.cancelController](/api/kbengine/cellapp/Entity#cancelController)
- [baseapp/KBEngine.baseAppData](/api/kbengine/baseapp/KBEngine#baseAppData)
- [baseapp/KBEngine.globalData](/api/kbengine/baseapp/KBEngine#globalData)
- [baseapp/KBEngine.onBaseAppData](/api/kbengine/baseapp/KBEngine#onBaseAppData)
- [baseapp/KBEngine.onBaseAppDataDel](/api/kbengine/baseapp/KBEngine#onBaseAppDataDel)
- [baseapp/KBEngine.onGlobalData](/api/kbengine/baseapp/KBEngine#onGlobalData)
- [baseapp/KBEngine.onGlobalDataDel](/api/kbengine/baseapp/KBEngine#onGlobalDataDel)
- [cellapp/KBEngine.cellAppData](/api/kbengine/cellapp/KBEngine#cellAppData)
- [cellapp/KBEngine.globalData](/api/kbengine/cellapp/KBEngine#globalData)
- [cellapp/KBEngine.onCellAppData](/api/kbengine/cellapp/KBEngine#onCellAppData)
- [cellapp/KBEngine.onCellAppDataDel](/api/kbengine/cellapp/KBEngine#onCellAppDataDel)
- [cellapp/KBEngine.onGlobalData](/api/kbengine/cellapp/KBEngine#onGlobalData)
- [cellapp/KBEngine.onGlobalDataDel](/api/kbengine/cellapp/KBEngine#onGlobalDataDel)
- [Source analysis: `Entity.createCellEntity()` is a Base→Cell handoff, not local object creation](/architecture/source-analysis/entity-system#base-entity-create-cell)
- [Source analysis: relationships among `isPlayer()`, `player()`, and `controlledBy()`](/architecture/source-analysis/networking#client-entity-isplayer-control)
- [Source analysis: client entity world/space callbacks](/architecture/source-analysis/networking#client-entity-world-space-callbacks)
- [Source analysis: real restrictions on `baseCall()` and `cellCall()`](/architecture/source-analysis/networking#client-entity-remote-calls)
- [Source analysis: client `moveToPoint()`, `cancelController()`, and `isOnGround` synchronization](/architecture/source-analysis/networking#client-entity-move-ground-sync)
- [Source analysis: client handle tables and entity-container semantics](/architecture/source-analysis/networking#client-entity-handles-table)
- [baseapp/Proxy.giveClientTo](/api/kbengine/baseapp/Proxy#giveClientTo)
- [Source analysis: `Proxy.giveClientTo()` transfers client binding and the Witness chain](/architecture/source-analysis/space-aoi#proxy-give-client-to)
- [Source analysis: Cell entity `addTimer()` / `delTimer()` operate the `ScriptID -> TimerHandle` mapping](/architecture/source-analysis/scripting#cell-entity-script-timers)
- [Source analysis: Cell `moveToPoint()` / `navigate()` / `cancelController()` share one controller chain](/architecture/source-analysis/space-aoi#cell-entity-move-controllers)
- [Source analysis: synchronization chains and callback dispatch for `baseAppData` / `globalData` / `cellAppData`](/architecture/source-analysis/networking#global-data-dicts-sync)

## Current execution strategy

- First eliminate inconsistent wording for the same term across pages.
- Next, complete the APIs most easily misunderstood in each category as “brief API page + deeper source-analysis page + API-page link.”
- Finally, return to `basetypes` and `keywords` for terminology cleanup.
