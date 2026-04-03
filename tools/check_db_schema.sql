-- KBEngine 数据库映射查看SQL脚本
-- 用于在MySQL中直接查看数据库结构

-- 1. 查看所有KBEngine表
SELECT
    TABLE_NAME as '表名',
    TABLE_ROWS as '行数',
    CREATE_TIME as '创建时间',
    UPDATE_TIME as '更新时间'
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE()
  AND (TABLE_NAME LIKE 'tbl_%' OR TABLE_NAME LIKE 'kbe_%')
ORDER BY TABLE_NAME;

-- 2. 查看特定实体的表结构
-- 将 Avatar 替换为你要查看的实体名
DESCRIBE tbl_Avatar;

-- 或者使用更详细的信息
SELECT
    COLUMN_NAME as '字段名',
    COLUMN_TYPE as '类型',
    IS_NULLABLE as '可空',
    COLUMN_KEY as '键',
    COLUMN_DEFAULT as '默认值',
    EXTRA as '额外信息'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = 'tbl_Avatar'
ORDER BY ORDINAL_POSITION;

-- 3. 查看所有字段的前缀分布
SELECT
    SUBSTRING_INDEX(COLUMN_NAME, '_', 1) as '字段前缀',
    COUNT(*) as '字段数量',
    GROUP_CONCAT(COLUMN_NAME ORDER BY COLUMN_NAME SEPARATOR ', ') as '字段列表'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME LIKE 'tbl_%'
GROUP BY SUBSTRING_INDEX(COLUMN_NAME, '_', 1)
ORDER BY FIELD_PREFIX;

-- 4. 对比不同实体的字段结构
SELECT
    TABLE_NAME as '实体表',
    COUNT(*) as '字段数量',
    GROUP_CONCAT(
        CASE WHEN COLUMN_NAME LIKE 'sm_%' THEN COLUMN_NAME END
        ORDER BY COLUMN_NAME
        SEPARATOR ', '
    ) as 'sm_字段列表'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME LIKE 'tbl_%'
GROUP BY TABLE_NAME
ORDER BY TABLE_NAME;

-- 5. 查找特定字段在哪些表中存在
-- 将 sm_level 替换为你要查找的字段名
SELECT
    TABLE_NAME as '表名',
    COLUMN_TYPE as '类型',
    COLUMN_DEFAULT as '默认值'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND COLUMN_NAME = 'sm_level';

-- 6. 查看系统表结构
SELECT
    TABLE_NAME as '系统表',
    TABLE_COMMENT as '注释'
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME LIKE 'kbe_%'
ORDER BY TABLE_NAME;

-- 7. 查看所有表的创建语句
SHOW CREATE TABLE tbl_Avatar;
SHOW CREATE TABLE kbe_accountinfos;
SHOW CREATE TABLE kbe_entitylog;

-- 8. 检查字段命名规范
SELECT
    TABLE_NAME,
    COLUMN_NAME,
    CASE
        WHEN COLUMN_NAME = 'id' THEN '主键'
        WHEN COLUMN_NAME = 'sm_autoLoad' THEN '系统字段'
        WHEN COLUMN_NAME LIKE 'sm_%' THEN '实体字段'
        WHEN COLUMN_NAME LIKE 'parent%' THEN '父级字段'
        ELSE '其他字段'
    END as '字段类型'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME LIKE 'tbl_%'
ORDER BY TABLE_NAME, COLUMN_NAME;

-- 9. 查看字段的数据类型分布
SELECT
    COLUMN_TYPE as '数据类型',
    COUNT(*) as '字段数量',
    GROUP_CONCAT(DISTINCT CONCAT(TABLE_NAME, '.', COLUMN_NAME) SEPARATOR '; ') as '使用位置'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME LIKE 'tbl_%'
  AND COLUMN_NAME LIKE 'sm_%'
GROUP BY COLUMN_TYPE
ORDER BY COUNT(*) DESC;

-- 10. 生成实体字段映射表（手动维护）
/*
创建一个映射记录表，用于记录UType与数据库字段的对应关系：

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
    KEY idx_utype (utype)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='KBEngine实体字段映射日志';
*/