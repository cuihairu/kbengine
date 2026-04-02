# Logger API

Logger API 提供了日志记录和调试功能。

## 概述

KBEngine 的日志系统支持多种日志级别和输出方式，便于开发调试和生产环境监控。

## 日志级别

| 级别 | 说明 | 使用场景 |
|------|------|----------|
| `DEBUG` | 调试信息 | 开发阶段 |
| `INFO` | 一般信息 | 正常运行信息 |
| `WARNING` | 警告信息 | 潜在问题 |
| `ERROR` | 错误信息 | 错误和异常 |
| `CRITICAL` | 严重错误 | 系统故障 |

## 基本用法

### 记录日志

```python
import KBEngine

# 不同级别的日志
KBEngine.DEBUG_MSG("调试信息")
KBEngine.INFO_MSG("一般信息")
KBEngine.WARNING_MSG("警告信息")
KBEngine.ERROR_MSG("错误信息")
```

### 格式化日志

```python
# 使用格式化字符串
player_id = 12345
KBEngine.INFO_MSG(f"玩家 {player_id} 登录成功")

# 使用占位符
KBEngine.INFO_MSG("玩家 %s 登录成功，等级 %d", player_name, level)
```

## 高级功能

### 日志过滤

```python
# 设置日志级别
KBEngine.setLogLevel(KBEngine.LOG_INFO)

# 只显示特定级别的日志
KBEngine.filterLog("network")
```

### 日志输出

```python
# 输出到文件
KBEngine.setLogFile("logs/game.log")

# 输出到控制台
KBEngine.setConsoleOutput(True)

# 输出到网络
KBEngine.setRemoteLog("http://logserver:8080")
```

### 自定义日志处理器

```python
class MyLogHandler(KBEngine.ILogHandler):
    def onLog(self, level, message, timestamp):
        # 自定义处理逻辑
        print(f"[{timestamp}] {level}: {message}")

KBEngine.setLogHandler(MyLogHandler())
```

## 配置选项

### 配置文件

在 `kbengine.xml` 中配置日志：

```xml
<logger>
    <logPath>logs/</logPath>
    <logLevel>INFO</logLevel>
    <logNums>10</logNums>
    <logSize>500M</logSize>
</logger>
```

### 动态配置

```python
# 运行时修改配置
KBEngine.setLogLevel(KBEngine.LOG_DEBUG)
KBEngine.setLogPath("logs/new/")
```

## 性能考虑

### 异步日志

日志系统使用异步写入，避免阻塞主线程：

```python
# 默认启用异步
KBEngine.setAsyncLog(True)
```

### 日志轮转

自动管理日志文件大小：

```xml
<logger>
    <logSize>1G</logSize>
    <logNums>30</logNums>
</logger>
```

## 调试技巧

### 条件日志

```python
# 只在特定条件下记录
if KBEngine.isDebugEnabled():
    KBEngine.DEBUG_MSG("详细调试信息")
```

### 追踪日志

```python
# 追踪函数调用
KBEngine.FUNCTION_TRACE("Entity::onEnter")

# 追踪变量变化
KBEngine.VAR_TRACE("health", self.health)
```

### 性能日志

```python
# 记录性能指标
KBEngine.PERF_MSG("处理时间: %.2fms", elapsed_time)
```

## 最佳实践

### 1. 合理使用日志级别

```python
# ✅ 好的做法
KBEngine.INFO_MSG("玩家登录")
KBEngine.ERROR_MSG("数据库连接失败")

# ❌ 不好的做法
KBEngine.DEBUG_MSG("玩家登录")  # 应该用 INFO
KBEngine.CRITICAL_MSG("网络延迟")  # 不应该用 CRITICAL
```

### 2. 包含上下文信息

```python
# ✅ 好的做法
KBEngine.ERROR_MSG(f"玩家 {self.id} 在位置 {self.position} 发生错误")

# ❌ 不好的做法
KBEngine.ERROR_MSG("发生错误")  # 缺少上下文
```

### 3. 避免过度日志

```python
# ✅ 好的做法
if counter % 100 == 0:
    KBEngine.INFO_MSG(f"已处理 {counter} 个请求")

# ❌ 不好的做法
KBEngine.INFO_MSG(f"已处理 {counter} 个请求")  # 太频繁
```

## 故障排查

### 日志未显示

检查日志级别配置：
```python
current_level = KBEngine.getLogLevel()
print(f"当前日志级别: {current_level}")
```

### 日志文件过大

调整日志轮转配置：
```xml
<logger>
    <logSize>100M</logSize>
    <logNums>5</logNums>
</logger>
```

### 性能问题

减少日志输出：
```python
# 降低日志级别
KBEngine.setLogLevel(KBEngine.LOG_WARNING)
```

## 相关文档

- [Entity API](entity.md)
- [配置说明](../guide/configuration.md)
- [性能优化](performance.md)
