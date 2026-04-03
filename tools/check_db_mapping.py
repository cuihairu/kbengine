#!/usr/bin/env python3
"""
KBEngine 数据库映射查看工具
用于查看数据库表字段与UType的映射关系
"""

import sys
import os

# 添加KBEngine Python路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'kbe', 'src', 'server', 'py'))

def check_entity_mapping(db_interface, entity_name):
    """
    检查实体的UType映射

    Args:
        db_interface: 数据库接口名称
        entity_name: 实体名称
    """
    try:
        import KBEngine

        # 获取实体定义
        entity_def = KBEngine.getEntityDef(entity_name)
        if not entity_def:
            print(f"错误: 找不到实体定义 {entity_name}")
            return

        print(f"\n{'='*60}")
        print(f"实体: {entity_name}")
        print(f"UType: {entity_def.getUType()}")
        print(f"{'='*60}\n")

        # 查看所有持久化属性
        print(f"{'属性名':<20} {'UType':<10} {'数据库字段':<20} {'数据类型':<15}")
        print('-' * 65)

        for prop_name, prop_desc in entity_def.properties.items():
            if prop_desc.isPersistent():  # 只显示持久化属性
                utype = prop_desc.utype
                db_field = f"sm_{prop_name}"  # KBEngine字段命名规则
                data_type = prop_desc.dataType.getName()

                print(f"{prop_name:<20} {utype:<10} {db_field:<20} {data_type:<15}")

        print(f"\n对应的数据库表: tbl_{entity_name}")
        print(f"表前缀: tbl_")
        print(f"字段前缀: sm_")

    except Exception as e:
        print(f"错误: {e}")

def list_all_entities():
    """列出所有实体及其UType"""
    try:
        import KBEngine

        entity_defs = KBEngine.getEntityDef()
        print(f"\n{'='*80}")
        print(f"{'实体名':<25} {'实体UType':<15} {'属性数量':<10}")
        print('-' * 80)

        for entity_name, entity_def in entity_defs.items():
            entity_utype = entity_def.getUType()
            prop_count = len([p for p in entity_def.properties.values() if p.isPersistent()])

            print(f"{entity_name:<25} {entity_utype:<15} {prop_count:<10}")

    except Exception as e:
        print(f"错误: {e}")

def show_sql_mapping(entity_name):
    """显示SQL查询示例"""
    print(f"\n{'='*60}")
    print(f"SQL 查询示例 - {entity_name}")
    print(f"{'='*60}\n")

    print(f"-- 查看表结构")
    print(f"SHOW CREATE TABLE tbl_{entity_name};\n")

    print(f"-- 查看所有字段")
    print(f"DESCRIBE tbl_{entity_name};\n")

    print(f"-- 查询数据示例")
    print(f"SELECT id, sm_level, sm_name FROM tbl_{entity_name} LIMIT 10;\n")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("用法:")
        print("  python check_db_mapping.py list                    # 列出所有实体")
        print("  python check_db_mapping.py <entity_name>           # 查看实体映射")
        print("  python check_db_mapping.py sql <entity_name>       # 显示SQL示例")
        sys.exit(1)

    command = sys.argv[1]

    if command == 'list':
        list_all_entities()
    elif command == 'sql' and len(sys.argv) >= 3:
        show_sql_mapping(sys.argv[2])
    else:
        check_entity_mapping("default", command)