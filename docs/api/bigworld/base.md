# BigWorld Base

> BigWorld Base 是运行在 BaseApp 上的实体脚本对象，和 KBEngine 的 Base 实体同源，但 API 数量与运行时语义更丰富。

## 主要来源文件

| 源码位置 | 说明 |
|----------|------|
| `server/baseapp/base.hpp` | `/*~ class BigWorld.Base */` 类说明与属性声明 |
| `server/baseapp/base.cpp` | `Base.destroy`、`createCellEntity`、`addTimer`、`writeToDB` 等方法注释与实现 |

## 已识别的 API 分组

- 生命周期：`destroy()`、`onDestroy`
- Cell 创建：`createCellEntity()`、`createInDefaultSpace()`、`createInNewSpace()`、`destroyCellEntity()`
- timer：`addTimer()`、`delTimer()`
- 持久化：`writeToDB()`
- 全局注册：`registerGlobally()`、`deregisterGlobally()`
- 属性：`id`、`databaseID`、`baseType`、`className`、`cell`、`hasCell`、`cellData`、`isDestroyed`

## 后续处理

本页后续应从 `/*~ function Base.* */` 和 `/*~ attribute Base.* */` 注释中抽取签名、参数、返回值和注意事项，并与 KBEngine 的 [BaseApp Entity](/api/kbengine/baseapp/Entity.md) 对照。
