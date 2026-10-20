# BigWorld 模块

> 本页是 BigWorld `BigWorld` Python 模块的源码抽取入口，后续会按 BaseApp、CellApp、DBApp、Bots 等组件拆分具体函数。

## 主要来源文件

| 组件 | 源码位置 | 说明 |
|------|----------|------|
| BaseApp | `server/baseapp/bigworld.mpp` | `BigWorld` 模块在 Base 侧的总说明 |
| BaseApp | `server/baseapp/script_bigworld.cpp` | Base 创建、DBID 查询、服务、热更新、关停等模块函数 |
| CellApp | `server/cellapp/bigworld.mpp` | `BigWorld` 模块在 Cell 侧的总说明 |
| CellApp | `server/cellapp/cellapp.cpp` | Cell 侧实体创建、时间、负载、热更新、关停等模块函数 |
| CellApp | `server/cellapp/space.cpp` | SpaceData、空间几何、录制、时间等空间级模块函数 |
| 通用 | `lib/server/app_script_timers.cpp` | App 级 `BigWorld.addTimer()` / `BigWorld.delTimer()` |

## 优先补齐的函数组

- `BigWorld.reloadScript(fullReload=True)`
- `BigWorld.addTimer(initialOffset, repeatOffset, callback, userArg=0)`
- `BigWorld.delTimer(timerID)`
- `BigWorld.createBase*`
- `BigWorld.createEntity*`
- `BigWorld.addSpaceData` / `delSpaceData` / `getSpaceData` / `setSpaceData`
- `BigWorld.globalData` / `baseAppData` / `cellAppData`

## 与 KBEngine 的关系

BigWorld 的模块名是 `BigWorld`，KBEngine 的模块名是 `KBEngine`。两者函数名有不少相似处，但实现边界并不完全相同。尤其是 App 级 timer、`reloadScript`、全局数据、服务注册和 SpaceData，后续应逐项对照，不应只做名字映射。
