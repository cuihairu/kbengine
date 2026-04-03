# KBEngine 数据库UType映射查看指南

## 核心概念

**重要：数据库本身不直接存储UType映射关系！**

UType映射存在于三个地方：
1. **Def文件**：静态定义（`*.def`）
2. **引擎内存**：运行时映射（EntityTable对象）
3. **数据库字段名**：通过命名规范隐式体现

## 映射关系推断

### 字段命名规则
```
def文件属性名 + "sm_" 前缀 = 数据库字段名

示例：
level      → sm_level
name       → sm_name
position   → sm_positionX, sm_positionY, sm_positionZ
direction  → sm_directionX, sm_directionY, sm_directionZ
```

### 表命名规则
```
"tbl_" + 实体名 = 数据库表名

示例：
Avatar     → tbl_Avatar
Account    → tbl_Account
Monster    → tbl_Monster
```

## 快速查看方法

### 方法1：使用SQL脚本查看
```bash
# 查看所有实体表
mysql -u root -p your_database < tools/check_db_schema.sql

# 查看特定实体映射
mysql -u root -p your_database < tools/check_db_mapping.sql
```

### 方法2：直接在MySQL中查询
```sql
-- 查看表结构
DESCRIBE tbl_Avatar;
SHOW CREATE TABLE tbl_Avatar;

-- 查看字段详情
SELECT
    COLUMN_NAME as '字段名',
    COLUMN_TYPE as '类型',
    COLUMN_DEFAULT as '默认值'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = 'tbl_Avatar'
ORDER BY ORDINAL_POSITION;
```

### 方法3：使用Python工具
```bash
# 列出所有实体
python tools/check_db_mapping.py list

# 查看特定实体映射
python tools/check_db_mapping.py Avatar

# 显示SQL示例
python tools/check_db_mapping.py sql Avatar
```

## 实际查看示例

### 1. 查看数据库表
```sql
mysql> SHOW CREATE TABLE tbl_Avatar\G
*************************** 1. row ***************************
       Table: tbl_Avatar
Create Table: CREATE TABLE `tbl_Avatar` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `sm_level` int(10) unsigned DEFAULT '1',
  `sm_name` varchar(32) DEFAULT NULL,
  `sm_positionX` float DEFAULT '0',
  `sm_positionY` float DEFAULT '0',
  `sm_positionZ` float DEFAULT '0',
  `sm_autoLoad` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
```

### 2. 推断UType映射
```
数据库字段        →  def文件属性    →  可能的UType
---------------------------------------------------------
sm_level         →  level         →  1001-1999
sm_name          →  name          →  2000-2999
sm_positionX     →  position.x    →  3000-3999
sm_positionY     →  position.y    →  3000-3999
sm_positionZ     →  position.z    →  3000-3999
sm_autoLoad      →  (系统字段)     →  (固定)
```

### 3. 查看引擎内存中的映射
```python
# 在KBEngine脚本中
import KBEngine

avatar_def = KBEngine.getEntityDef("Avatar")
for prop_name, prop_desc in avatar_def.properties.items():
    if prop_desc.isPersistent():
        print(f"{prop_name} → UType: {prop_desc.utype} → sm_{prop_name}")
```

## 手动维护映射表

为了方便管理，建议创建映射记录表：

```sql
CREATE TABLE IF NOT EXISTS kbe_entity_mapping_log (
    id INT AUTO_INCREMENT PRIMARY KEY,
    entity_name VARCHAR(50) NOT NULL COMMENT '实体名称',
    property_name VARCHAR(50) NOT NULL COMMENT '属性名称',
    utype INT UNSIGNED NOT NULL COMMENT 'UType标识',
    db_field VARCHAR(100) NOT NULL COMMENT '数据库字段名',
    data_type VARCHAR(50) COMMENT '数据类型',
    is_persistent TINYINT(1) DEFAULT 1 COMMENT '是否持久化',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    UNIQUE KEY idx_entity_property (entity_name, property_name),
    KEY idx_utype (utype),
    KEY idx_entity_name (entity_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='KBEngine实体字段映射日志';
```

### 插入映射记录
```sql
-- 根据def文件手动记录映射
INSERT INTO kbe_entity_mapping_log
(entity_name, property_name, utype, db_field, data_type, is_persistent)
VALUES
('Avatar', 'level', 1001, 'sm_level', 'UINT32', 1),
('Avatar', 'name', 1002, 'sm_name', 'STRING', 1),
('Avatar', 'position', 1003, 'sm_positionX', 'VECTOR3', 1);

-- 查询映射
SELECT
    entity_name AS '实体',
    property_name AS '属性',
    utype AS 'UType',
    db_field AS '数据库字段',
    data_type AS '类型'
FROM kbe_entity_mapping_log
WHERE entity_name = 'Avatar'
ORDER BY utype;
```

## 常见问题

### Q: 为什么数据库看不到UType？
A: 数据库只存储数据，UType映射是引擎内部的元数据。通过字段名的规范（sm_前缀）来隐式体现映射关系。

### Q: 如何确定某个字段的UType？
A: 需要查看对应的def文件，或在引擎启动时通过日志查看映射关系。

### Q: 修改UType后数据库会出问题吗？
A: 是的！修改UType会导致数据无法正确读写。必须手动指定UType并保持稳定。

### Q: 如何查看当前系统的完整映射？
A: 使用提供的工具脚本，或查看引擎启动时的日志输出。

## 最佳实践

1. **生产环境必须在def文件中手动指定UType**
2. **维护映射记录表**，方便追踪和调试
3. **定期检查数据库结构与def文件的一致性**
4. **修改属性前先检查UType占用情况**
5. **保留旧属性的UType**，避免数据丢失

## 工具使用说明

- `check_db_schema.sql` - 查看整体数据库结构
- `check_db_mapping.sql` - 查看特定实体的字段映射
- `check_db_mapping.py` - Python脚本查看引擎内存映射

选择合适的工具来查看和维护你的KBEngine数据库映射关系。