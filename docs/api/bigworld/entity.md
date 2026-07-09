# BigWorld Cell Entity

> BigWorld Cell Entity 是运行在 CellApp 上的空间实体对象。它和 KBEngine Cell Entity 同源，但 BigWorld 侧包含更多 AOI、控制器、导航和录制相关能力。

## 主要来源文件

| 源码位置 | 说明 |
|----------|------|
| `server/cellapp/entity.cpp` | Cell Entity 通用方法和属性 |
| `server/cellapp/real_entity.cpp` | real entity 特有属性和方法 |
| `server/cellapp/witness.cpp` | Witness、AOI、客户端可见性相关方法 |
| `server/cellapp/timer_controller.cpp` | 实体级 timer |
| `server/cellapp/proximity_controller.cpp` | proximity controller |
| `server/cellapp/turn_controller.cpp` | yaw rotator |
| `server/cellapp/visibility_controller.cpp` | visibility controller |

## 已识别的 API 分组

- 生命周期与销毁：`destroy()`
- timer 与控制器：`addTimer()`、`cancel()`、`addProximity()`、`addYawRotator()`
- Witness / AOI：`dumpAoI()`、`setAoIRadius()`、`entitiesInAoI()`、`setAoIUpdateScheme()`
- 客户端调用入口：`client`、`ownClient`、`otherClients`、`allClients`
- real entity 属性：`controlledBy`、`isWitnessed`、`hasWitness`、`shouldAutoBackup`

## 后续处理

本页后续应先补 Witness/AOI 与 timer，因为它们最容易和 KBEngine 的 [CellApp Entity](/api/kbengine/cellapp/Entity.md) 发生误读。
