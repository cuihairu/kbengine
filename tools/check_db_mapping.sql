-- 快速查询当前数据库的UType映射状态
-- 使用方法：mysql -u root -p your_database < check_db_mapping.sql

-- 设置显示格式
-- \G 垂直显示结果

-- === 1. 查看所有实体表概览 ===
SELECT CONCAT('=== ', DATABASE(), ' 数据库实体表概览 ===') AS '';
SELECT
    TABLE_NAME AS '表名',
    TABLE_ROWS AS '数据行数',
    ROUND((DATA_LENGTH + INDEX_LENGTH) / 1024 / 1024, 2) AS '大小(MB)',
    CREATE_TIME AS '创建时间',
    UPDATE_TIME AS '最后更新'
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME LIKE 'tbl_%'
ORDER BY TABLE_NAME;

-- === 2. 查看具体实体表结构（修改实体名） ===
SET @entity_name = 'Avatar'; -- 修改这里查看不同实体

SELECT CONCAT('=== 实体表: tbl_', @entity_name, ' 结构详情 ===') AS '';

SELECT
    COLUMN_NAME AS '字段名',
    COLUMN_TYPE AS '类型',
    IS_NULLABLE AS '可空',
    COLUMN_KEY AS '键类型',
    COLUMN_DEFAULT AS '默认值',
    EXTRA AS '额外信息'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = CONCAT('tbl_', @entity_name)
ORDER BY ORDINAL_POSITION;

-- === 3. 分析字段命名模式 ===
SELECT CONCAT('=== 字段命名模式分析 ===') AS '';

SELECT
    CASE
        WHEN COLUMN_NAME = 'id' THEN '主键ID'
        WHEN COLUMN_NAME = 'sm_autoLoad' THEN '系统-自动加载标记'
        WHEN COLUMN_NAME LIKE 'sm\_position%' THEN '位置坐标(VECTOR3)'
        WHEN COLUMN_NAME LIKE 'sm\_direction%' THEN '方向向量(VECTOR3)'
        WHEN COLUMN_NAME LIKE 'sm\_%' THEN '实体属性字段'
        WHEN COLUMN_NAME LIKE 'parent%' THEN '父级引用'
        ELSE '其他字段'
    END AS '字段分类',
    COUNT(*) AS '数量',
    GROUP_CONCAT(COLUMN_NAME ORDER BY COLUMN_NAME SEPARATOR ', ') AS '字段列表'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = CONCAT('tbl_', @entity_name)
GROUP BY
    CASE
        WHEN COLUMN_NAME = 'id' THEN '主键ID'
        WHEN COLUMN_NAME = 'sm_autoLoad' THEN '系统-自动加载标记'
        WHEN COLUMN_NAME LIKE 'sm\_position%' THEN '位置坐标(VECTOR3)'
        WHEN COLUMN_NAME LIKE 'sm\_direction%' THEN '方向向量(VECTOR3)'
        WHEN COLUMN_NAME LIKE 'sm\_%' THEN '实体属性字段'
        WHEN COLUMN_NAME LIKE 'parent%' THEN '父级引用'
        ELSE '其他字段'
    END
ORDER BY
    FIELD(
        CASE
            WHEN COLUMN_NAME = 'id' THEN '主键ID'
            WHEN COLUMN_NAME = 'sm_autoLoad' THEN '系统-自动加载标记'
            WHEN COLUMN_NAME LIKE 'sm\_position%' THEN '位置坐标(VECTOR3)'
            WHEN COLUMN_NAME LIKE 'sm\_direction%' THEN '方向向量(VECTOR3)'
            WHEN COLUMN_NAME LIKE 'sm\_%' THEN '实体属性字段'
            WHEN COLUMN_NAME LIKE 'parent%' THEN '父级引用'
            ELSE '其他字段'
        END,
        '主键ID', '系统-自动加载标记', '位置坐标(VECTOR3)', '方向向量(VECTOR3)', '实体属性字段', '父级引用', '其他字段'
    );

-- === 4. UType映射推断（基于字段名） ===
SELECT CONCAT('=== UType映射推断 ===') AS '';

SELECT
    CONCAT('sm_', COLUMN_NAME) AS 'def文件属性名',
    COLUMN_NAME AS '数据库字段名',
    COLUMN_TYPE AS '数据类型',
    CASE
        WHEN COLUMN_NAME LIKE '%position%' THEN 'VECTOR3 → 3个字段'
        WHEN COLUMN_NAME LIKE '%direction%' THEN 'VECTOR3 → 3个字段'
        WHEN COLUMN_NAME LIKE '%level%' THEN '可能是 UType: 1001-1999 (数值属性)'
        WHEN COLUMN_NAME LIKE '%name%' THEN '可能是 UType: 2000-2999 (字符串属性)'
        WHEN COLUMN_NAME LIKE '%exp%' THEN '可能是 UType: 3000-3999 (经验值)'
        ELSE '需要查看def文件确定UType'
    END AS 'UType推断'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = CONCAT('tbl_', @entity_name)
  AND COLUMN_NAME LIKE 'sm\_%'
  AND COLUMN_NAME NOT LIKE '%\_position%'
  AND COLUMN_NAME NOT LIKE '%\_direction%'
ORDER BY COLUMN_NAME;

-- === 5. 与系统表对比 ===
SELECT CONCAT('=== 系统表结构对比 ===') AS '';

SELECT '系统表' AS '表类型', TABLE_NAME AS '表名', TABLE_ROWS AS '行数'
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME LIKE 'kbe_%'
UNION ALL
SELECT '实体表' AS '表类型', TABLE_NAME AS '表名', TABLE_ROWS AS '行数'
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME LIKE 'tbl_%'
ORDER BY '表类型', TABLE_NAME;

-- === 6. 推荐的查询命令 ===
SELECT CONCAT('=== 推荐的查询命令 ===') AS '';

SELECT
    '-- 查看数据' AS '',
    CONCAT('SELECT * FROM tbl_', @entity_name, ' LIMIT 10;') AS '命令'
UNION ALL
SELECT
    '-- 查看特定字段' AS '',
    CONCAT('SELECT id, sm_level, sm_name FROM tbl_', @entity_name, ' WHERE id > 0;') AS '命令'
UNION ALL
SELECT
    '-- 统计数据' AS '',
    CONCAT('SELECT COUNT(*) as total, MAX(id) as max_id FROM tbl_', @entity_name, ';') AS '命令';

-- === 7. 字段类型统计 ===
SELECT CONCAT('=== 字段类型统计 ===') AS '';

SELECT
    COLUMN_TYPE AS '字段类型',
    COUNT(*) AS '字段数量',
    GROUP_CONCAT(CONCAT(TABLE_NAME, '.', COLUMN_NAME) SEPARATOR ', ') AS '使用位置'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = CONCAT('tbl_', @entity_name)
GROUP BY COLUMN_TYPE
ORDER BY COUNT(*) DESC;

-- === 8. 检查字段完整性 ===
SELECT CONCAT('=== 字段完整性检查 ===') AS '';

SELECT
    CASE
        WHEN COUNT(*) = COUNT(CASE WHEN COLUMN_NAME LIKE 'sm\_%' END) THEN '所有字段都有sm_前缀'
        ELSE '存在字段没有sm_前缀'
    END AS '前缀检查',
    COUNT(*) AS '总字段数',
    SUM(CASE WHEN COLUMN_NAME LIKE 'sm\_%' THEN 1 ELSE 0 END) AS 'sm_字段数',
    SUM(CASE WHEN COLUMN_NAME = 'sm_autoLoad' THEN 1 ELSE 0 END) AS 'autoLoad字段'
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
  AND TABLE_NAME = CONCAT('tbl_', @entity_name);

SELECT CONCAT('=== 查询完成 ===') AS '';