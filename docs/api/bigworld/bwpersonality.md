# BigWorld BWPersonality

> `BWPersonality` 是 BigWorld 服务端脚本的进程级回调入口，角色接近 KBEngine 各组件模块中的 `onInit`、`onReadyForLogin`、`on*AppReady`、`on*ShutDown` 等回调集合。

## 主要来源文件

| 组件 | 源码位置 |
|------|----------|
| 通用 | `lib/server/bwpersonality.mpp` |
| BaseApp | `server/baseapp/bwpersonality.mpp` |
| CellApp | `server/cellapp/bwpersonality.mpp` |

## 已识别的回调分组

- 通用生命周期：`onInit`、`onFini`
- BaseApp：`onBaseAppReady`、`onAppReady`、`onBaseAppShuttingDown`、`onBaseAppShutDown`
- CellApp：`onCellAppReady`、`onAppReady`、`onCellAppShuttingDown`
- 组件死亡通知：`onBaseAppDeath`、`onCellAppDeath`、`onServiceAppDeath`
- 全局数据：`onBaseAppData`、`onDelBaseAppData`、`onCellAppData`、`onDelCellAppData`、`onGlobalData`、`onDelGlobalData`
- 空间事件：`onSpaceData`、`onSpaceDataDeleted`、`onSpaceGeometryLoaded`

## 后续处理

BigWorld `BWPersonality` 应作为进程级脚本入口单独整理，不应混进 `BigWorld` 模块函数页。这样才能清楚区分“主动调用的 API”和“引擎回调脚本的 hook”。
