# Entity API

Entity API 提供了实体管理和操作的核心功能。

## 概述

在 KBEngine 中，所有游戏对象都是实体（Entity）。Entity API 提供了创建、销毁、查询和操作实体的方法。

## 实体类型

### 基础实体

| 类型 | 说明 | 位置 |
|------|------|------|
| `Entity` | 所有实体的基类 | BaseApp/CellApp |
| `Avatar` | 玩家实体 | BaseApp/CellApp |
| `Monster` | 怪物实体 | CellApp |
| `NPC` | NPC 实体 | CellApp |

### 自定义实体

```python
class MyEntity(KBEngine.Entity):
    def __init__(self):
        KBEngine.Entity.__init__(self)
        # 初始化代码
```

## 实体生命周期

### 创建实体

```python
# 在本地创建实体
entity = KBEngine.createEntityLocally("MyEntity", {})

# 在远程创建实体
KBEngine.createEntityAnywhere("MyEntity", {}, onEntityCreated)

# 通过空间创建实体
spaceID = 1
entity = KBEngine.createEntityInSpace("MyEntity", spaceID, {})
```

### 销毁实体

```python
# 销毁自己
self.destroy()

# 销毁其他实体
entity.destroy()

# 延迟销毁
self.destroy(delay=5.0)
```

## 实体属性

### 基础属性

```python
# 实体 ID
entity_id = self.id

# 实体类型
entity_type = self.__class__.__name__

# 空间 ID
space_id = self.spaceID

# 位置
position = self.position
direction = self.direction

# 实体状态
is_destroyed = self.isDestroyed()
```

### 自定义属性

```python
class MyEntity(KBEngine.Entity):
    # 定义属性
    health = 100
    maxHealth = 100
    level = 1
    name = ""

    def onHealthChanged(self, old_value):
        KBEngine.INFO_MSG(f"生命值变化: {old_value} -> {self.health}")
```

## 实体方法

### 位置和移动

```python
# 设置位置
self.position = (0, 0, 0)

# 设置朝向
self.direction = (0, 0, 0)

# 移动到位置
self.moveTo((10, 0, 10), speed=5.0)

# 停止移动
self.stopMovement()
```

### 空间操作

```python
# 进入空间
self.onEnterSpace(space_id)

# 离开空间
self.onLeaveSpace(space_id)

# 查询空间内的实体
entities = KBEngine.entitiesInSpace(space_id)
```

### 网络同步

```python
# 同步属性到客户端
self.setClient("position", self.position)

# 调用客户端方法
self.client.onPositionUpdate(self.position)

# 广播消息
self.allClients.onEntityCreated(self.id)
```

## 实体通信

### 实体间消息

```python
# 向其他实体发送消息
other_entity.onMessage("Hello")

# 调用实体方法
other_entity.someMethod(arg1, arg2)
```

### 全局消息

```python
# 广播到所有实体
KBEngine.broadcastMessage("eventName", data)

# 广播到空间内的实体
KBEngine.broadcastInSpace(space_id, "eventName", data)
```

## 实体查询

### 查找实体

```python
# 通过 ID 查找实体
entity = KBEngine.findEntity(entity_id)

# 查找所有实体
all_entities = KBEngine.entities()

# 查找特定类型的实体
avatars = KBEngine.entitiesOfType("Avatar")
```

### 实体关系

```python
# 获取父实体
parent = self.parentEntity

# 获取子实体
children = self.childEntities

# 查找附近的实体
nearby = self.findNearbyEntities(radius=10.0)
```

## 实体事件

### 生命周期事件

```python
class MyEntity(KBEngine.Entity):
    def __init__(self):
        KBEngine.Entity.__init__(self)

    def onEntitiesEnabled(self):
        """实体启用时调用"""
        pass

    def onDestroy(self):
        """实体销毁时调用"""
        pass
```

### 空间事件

```python
def onEnterSpace(self, space_id):
    """进入空间"""
    pass

def onLeaveSpace(self, space_id):
    """离开空间"""
    pass
```

### 属性事件

```python
def onHealthChanged(self, old_value):
    """属性变化"""
    if self.health <= 0:
        self.onDeath()
```

## 性能优化

### 实体池

```python
# 使用对象池
class EntityPool:
    def __init__(self, entity_type, pool_size=100):
        self.pool = []
        for _ in range(pool_size):
            entity = KBEngine.createEntityLocally(entity_type, {})
            self.pool.append(entity)

    def acquire(self):
        return self.pool.pop() if self.pool else None

    def release(self, entity):
        self.pool.append(entity)
```

### 批量操作

```python
# 批量更新
entities = KBEngine.entities()
for entity in entities:
    entity.update()

# 使用并行处理
KBEngine.parallelProcess(entities, lambda e: e.update())
```

## 最佳实践

### 1. 实体设计

```python
# ✅ 好的设计 - 单一职责
class CombatEntity(KBEngine.Entity):
    """战斗相关"""
    pass

class MovementEntity(KBEngine.Entity):
    """移动相关"""
    pass

# ❌ 不好的设计 - 职责混乱
class EverythingEntity(KBEngine.Entity):
    """包含所有功能"""
    pass
```

### 2. 属性管理

```python
# ✅ 好的做法 - 使用属性装饰器
@property
def health(self):
    return self._health

@health.setter
def health(self, value):
    old = self._health
    self._health = max(0, min(value, self.maxHealth))
    if old != self._health:
        self.onHealthChanged(old)

# ❌ 不好的做法 - 直接访问
entity.health = 999
```

### 3. 错误处理

```python
# ✅ 好的做法
def onMessage(self, data):
    try:
        self.processMessage(data)
    except Exception as e:
        KBEngine.ERROR_MSG(f"处理消息失败: {e}")

# ❌ 不好的做法
def onMessage(self, data):
    self.processMessage(data)  # 可能抛出异常
```

## 调试技巧

### 实体监控

```python
# 监控实体数量
KBEngine.INFO_MSG(f"当前实体数量: {len(KBEngine.entities())}")

# 监控特定实体
KBEngine.watchEntity(entity_id)
```

### 实体检查

```python
# 检查实体状态
if entity and not entity.isDestroyed():
    # 安全操作
    entity.doSomething()
```

## 相关文档

- [Logger API](logger.md)
- [Network API](network.md)
- [配置说明](../guide/configuration.md)
