# API 参考

KBEngine 提供了丰富的 API 用于游戏逻辑开发。

## 概述

KBEngine 的 API 主要分为以下几类：

- **Logger API** - 日志记录和调试
- **Entity API** - 实体管理和操作
- **Network API** - 网络通信
- **Database API** - 数据库操作
- **Utility API** - 工具函数

## Python API

### Logger API

详见 [Logger API 文档](logger.md)

### Entity API

详见 [Entity API 文档](entity.md)

## C++ API

KBEngine 底层使用 C++ 实现，提供了高性能的核心功能。

### 核心类

- `KBEngine::CellApp` - Cell 应用
- `KBEngine::BaseApp` - Base 应用
- `KBEngine::Logger` - 日志系统
- `KBEngine::Network` - 网络通信

## 快速索引

### 按功能分类

- [实体系统](entity.md)
- [网络通信](network.md)
- [数据库操作](database.md)
- [日志系统](logger.md)
- [工具函数](utility.md)

### 按模块分类

- [BaseApp 模块](baseapp.md)
- [CellApp 模块](cellapp.md)
- [Logger 模块](logger.md)
- [DBMgr 模块](dbmgr.md)

## API 使用示例

### 创建实体

```python
import KBEngine

# 创建实体
entity = KBEngine.createEntityLocally("Avatar", params)
```

### 发送消息

```python
# 发送到客户端
self.client.onLogin(success=True)

# 广播消息
KBEngine.broadcastMessage("system", "Welcome!")
```

### 数据库操作

```python
# 查询数据
def onCallback(result):
    print(result)

KBEngine.executeRawDatabaseCommand("SELECT * FROM users", onCallback)
```

## 版本兼容性

| API 版本 | KBEngine 版本 | 状态 |
|----------|---------------|------|
| 1.0 | 0.x.x | 已弃用 |
| 2.0 | 1.x.x | 稳定版 |
| 2.1 | 1.2.x+ | 当前版本 |

## 迁移指南

### 从 1.0 迁移到 2.0

主要变更：
- 实体系统重构
- 网络接口优化
- 数据库 API 改进

详见迁移指南。

## 参考文档

- [Python API](https://docs.python.org/3/)
- [C++ API](https://en.cppreference.com/)
- [KBEngine 源码](https://github.com/cuihairu/kbengine)
