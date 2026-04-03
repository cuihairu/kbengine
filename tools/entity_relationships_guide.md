# KBEngine 实体关联机制详解

## 核心概念：KBEngine中的实体关联

KBEngine提供了多种实体关联方式，主要分为**引擎级别**和**业务级别**两个层次。

## 1. 引擎级别的关联机制

### 1.1 ENTITYCALL 类型（实体引用）

**定义：** ENTITYCALL 是KBEngine中专门用于实体间引用的数据类型。

```python
# 在def文件中定义实体引用
<Property>
    <Name>targetEntity</Name>
    <Type>ENTITYCALL</Type>
    <Persistent>true</Persistent>
    <UType>2001</UType>
</Property>

<Property>
    <Name>owner</Name>
    <Type>ENTITYCALL</Type>
    <Persistent>true</Persistent>
    <UType>2002</UType>
</Property>
```

**存储格式：**
```cpp
// ENTITYCALL在数据库中的存储格式
struct EntityCallData {
    ENTITY_ID entityID;     // 实体ID
    ENTITYCALL_TYPE type;   // 调用类型 (BASE/CELL)
    COMPONENT_ID componentID; // 组件ID
};
```

**数据库字段：**
```sql
-- 对应的数据库字段
sm_targetEntity BIGINT  -- 存储ENTITY_ID
sm_owner BIGINT         -- 存储ENTITY_ID
```

### 1.2 ParentID 字段（父子关系）

**定义：** KBEngine内置的父子关系字段。

```cpp
// entity_table.h:40
#define TABLE_PARENTID_CONST_STR "parentID"
```

**数据库结构：**
```sql
CREATE TABLE tbl_Monster (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    parentID BIGINT UNSIGNED DEFAULT 0,    -- 父实体ID
    sm_level INT UNSIGNED DEFAULT 1,
    sm_hp INT UNSIGNED DEFAULT 100
) ENGINE=InnoDB;
```

**使用场景：**
```python
# 创建子实体时指定父实体
monster = createEntity("Monster", params)
monster.parentID = player.id  # 设置父实体为玩家
```

## 2. 业务级别的关联实现

### 2.1 直接ID关联

**最简单直接的关联方式：**

```python
# Avatar.def
<Property>
    <Name>accountID</Name>
    <Type>UINT64</Type>
    <Persistent>true</Persistent>
    <UType>3001</UType>
</Property>

<Property>
    <Name>guildID</Name>
    <Type>UINT64</Type>
    <Persistent>true</Persistent>
    <UType>3002</UType>
</Property>

<Property>
    <Name>teamID</Name>
    <Type>UINT64</Type>
    <Persistent>true</Persistent>
    <UType>3003</UType>
</Property>
```

**业务代码实现：**
```python
class Avatar(KBEngine.Entity):
    def __init__(self):
        KBEngine.Entity.__init__(self)

        # 通过ID关联其他实体
        self.accountID = 0          # 关联Account实体
        self.guildID = 0            # 关联Guild实体
        self.teamID = 0             # 关联Team实体

    def getGuild(self):
        # 查询关联的公会实体
        return KBEngine.getEntity(self.guildID)

    def setGuild(self, guildID):
        self.guildID = guildID
        # 保存到数据库
        self.writeToDB()
```

### 2.2 关联表（关系型数据）

**复杂关系使用关联表：**

```sql
-- 好友关系表
CREATE TABLE tbl_Friends (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    avatarID1 BIGINT UNSIGNED NOT NULL,
    avatarID2 BIGINT UNSIGNED NOT NULL,
    relationType INT UNSIGNED DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uk_avatar_pair (avatarID1, avatarID2),
    KEY idx_avatar1 (avatarID1),
    KEY idx_avatar2 (avatarID2)
) ENGINE=InnoDB;

-- 物品拥有关系表
CREATE TABLE tbl_Items (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    ownerID BIGINT UNSIGNED NOT NULL,      -- 所有者ID
    itemType INT UNSIGNED NOT NULL,        -- 物品类型
    count INT UNSIGNED DEFAULT 1,
    KEY idx_owner (ownerID)
) ENGINE=InnoDB;
```

**业务代码实现：**
```python
class Avatar(KBEngine.Entity):
    def getFriends(self):
        # 查询好友关系
        sql = "SELECT avatarID2 FROM tbl_Friends WHERE avatarID1 = %d" % self.id
        KBEngine.executeRawDatabaseCommand(sql, self.onGetFriendsResult)

    def onGetFriendsResult(self, result):
        friendIDs = [row['avatarID2'] for row in result]
        self.friends = [KBEngine.getEntity(fid) for fid in friendIDs if fid > 0]
```

### 2.3 位置关联（空间关系）

**基于空间的实体关联：**

```python
class Space(KBEngine.Entity):
    def __init__(self):
        KBEngine.Entity.__init__(self)
        self.avatars = []  # 当前空间内的实体列表

    def onEnter(self, entity):
        self.avatars.append(entity)
        entity.spaceID = self.id

    def onLeave(self, entity):
        if entity in self.avatars:
            self.avatars.remove(entity)
```

## 3. 常见关联模式

### 3.1 一对一关系

```python
# Account ← → Avatar (一个账号对应一个角色)
class Account(KBEngine.Proxy):
    def __init__(self):
        self.activeAvatarID = 0  # 当前活跃角色ID

    def getActiveAvatar(self):
        return KBEngine.getEntity(self.activeAvatarID)
```

### 3.2 一对多关系

```python
# Avatar ← → Items (一个角色拥有多个物品)
class Avatar(KBEngine.Entity):
    def __init__(self):
        self.bagItems = {}  # {itemID: itemCount}

    def getItems(self):
        # 从数据库查询物品
        sql = "SELECT * FROM tbl_Items WHERE ownerID = %d" % self.id
        KBEngine.executeRawDatabaseCommand(sql, self.onGetItemsResult)
```

### 3.3 多对多关系

```python
# Avatar ← → Avatar (好友关系)
class Avatar(KBEngine.Entity):
    def addFriend(self, friendID):
        sql = "INSERT INTO tbl_Friends (avatarID1, avatarID2) VALUES (%d, %d)" % (self.id, friendID)
        KBEngine.executeRawDatabaseCommand(sql, None)
```

## 4. 性能优化建议

### 4.1 缓存关联实体

```python
class Avatar(KBEngine.Entity):
    def __init__(self):
        self.guild = None  # 缓存的公会实体
        self.guildID = 0   # 数据库中的ID

    def getGuild(self):
        if self.guild is None and self.guildID > 0:
            self.guild = KBEngine.getEntity(self.guildID)
        return self.guild
```

### 4.2 批量查询优化

```python
# 不好的做法：N+1查询
for memberID in guild.memberIDs:
    member = KBEngine.getEntity(memberID)  # 逐个查询

# 好的做法：批量查询
sql = "SELECT * FROM tbl_Avatar WHERE id IN (%s)" % ','.join(map(str, guild.memberIDs))
KBEngine.executeRawDatabaseCommand(sql, self.onBatchLoadMembers)
```

### 4.3 索引优化

```sql
-- 为关联字段添加索引
CREATE INDEX idx_accountID ON tbl_Avatar(accountID);
CREATE INDEX idx_guildID ON tbl_Avatar(guildID);
CREATE INDEX idx_teamID ON tbl_Avatar(teamID);
```

## 5. 实体关联的最佳实践

### 5.1 优先使用ID关联

```python
# ✅ 推荐：使用ID关联
self.ownerID = 12345

# ❌ 避免：直接存储实体引用（ ENTITYCALL开销较大）
self.owner = entityCall
```

### 5.2 懒加载关联实体

```python
class Avatar(KBEngine.Entity):
    def __init__(self):
        self.guildID = 0
        self._guild = None  # 延迟加载

    @property
    def guild(self):
        if self._guild is None and self.guildID > 0:
            self._guild = GuildManager.getGuild(self.guildID)
        return self._guild
```

### 5.3 处理实体销毁

```python
class Avatar(KBEngine.Entity):
    def onDestroy(self):
        # 清理关联关系
        if self.guildID > 0:
            guild = KBEngine.getEntity(self.guildID)
            if guild:
                guild.removeMember(self.id)

        # 清空所有关联ID
        self.guildID = 0
        self.teamID = 0
```

## 6. 总结

**KBEngine实体关联的三种层次：**

```
┌─────────────────────────────────────────────────────────────┐
│                   KBEngine 实体关联层次                        │
├─────────────────────────────────────────────────────────────┤
│  1. 引擎级：ENTITYCALL类型、parentID字段（自动管理）          │
│  2. 业务级：ID关联、关联表（手动管理）                        │
│  3. 空间级：Space实体、位置关系（场景管理）                   │
└─────────────────────────────────────────────────────────────┘
```

**选择建议：**
- **简单引用** → 使用ID关联
- **强关联** → 使用ENTITYCALL
- **复杂关系** → 使用关联表
- **空间关系** → 基于Space实体

大部分情况下，**业务级别的ID关联是最灵活和高效的选择**。