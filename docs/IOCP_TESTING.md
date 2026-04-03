# IOCP Poller 测试用例说明

## 📋 测试概述

为 Windows IOCP 网络模式添加了完整的测试套件，覆盖功能性、性能、压力测试等多个方面。

---

## 🧪 测试文件

### 1. `test_network_event_poller.cpp`（扩展）

**跨平台测试：**
- ✅ **平台特定测试**：
  - Windows: IOCP 创建和基本操作
  - Linux: Epoll 创建和基本操作
  - 其他: Select 创建和基本操作

- ✅ **跨平台一致性测试**：
  - 多个 poller 创建/销毁
  - poller 类型一致性验证
  - 基础功能验证

- ✅ **性能基准测试**：
  - 延迟测试（平均延迟 < 100μs）
  - 吞吐量测试（> 100 次/秒）

---

### 2. `test_network_iocp_poller.cpp`（新增）

**Windows IOCP 专用测试：**

#### **压力测试**
- ✅ **高并发创建测试**：10 个 IOCP poller 同时存在
- ✅ **快速创建销毁测试**：100 次创建/销毁循环
- ✅ **长时间运行测试**：1 秒持续事件处理
- ✅ **多线程并发测试**：4 线程并发访问

#### **性能测试**
- ✅ **延迟测试**：
  - 平均延迟 < 100μs
  - 最大延迟 < 1ms
  - 1000 次调用采样

- ✅ **吞吐量测试**：
  - 目标：> 1000 事件/秒
  - 1 秒测试窗口

#### **内存测试**
- ✅ **内存泄漏检测框架**：
  - 1000 次创建/销毁循环
  - 检测泄漏和资源释放

#### **边界测试**
- ✅ **零超时**
- ✅ **负超时**
- ✅ **长超时**

---

## 🚀 运行测试

### Windows (Visual Studio)

```bash
# 构建
cmake --preset vcpkg
cmake --build build/presets/vcpkg

# 运行所有测试
ctest --preset vcpkg --output-on-failure

# 只运行网络相关测试
ctest --preset vcpkg -R "Network.*" --output-on-failure

# 只运行 IOCP 测试
ctest --preset vcpkg -R "Iocp" --verbose
```

### Windows (命令行)

```bash
cd build/presets/vcpkg/tests/bin
./kbe_smoke_tests --gtest_filter="*Iocp*"
```

---

## 📊 测试分类

### 按类型分类

| 类别 | 测试数量 | 覆盖范围 |
|------|---------|---------|
| **Bootstrap** | 3 | 基本功能验证 |
| **Platform-Specific** | 4 | 平台特定行为 |
| **Cross-Platform** | 2 | 跨平台一致性 |
| **Performance** | 2 | 性能基准 |
| **Stress** | 4 | 高并发和压力 |
| **Boundary** | 3 | 边界条件 |
| **Memory** | 1 | 内存泄漏 |

### 按平台分类

| 平台 | 测试文件 | 测试数量 |
|------|---------|---------|
| **Windows** | test_network_iocp_poller.cpp | 10+ |
| **所有** | test_network_event_poller.cpp | 10 |

---

## 🎯 测试覆盖的功能点

### 1. IOCP 创建和初始化
- ✅ 完成端口创建
- ✅ 并发线程数配置
- ✅ 系统资源分配

### 2. 事件处理
- ✅ 完成队列获取
- ✅ 超时处理
- ✅ 错误处理

### 3. Socket 集成
- ⚠️ Socket 关联到完成端口
- ⚠️ 异步 I/O 操作
- ⚠️ Overlapped 结构管理

### 4. 性能特性
- ✅ 低延迟
- ✅ 高吞吐量
- ✅ 可扩展性

---

## ⚠️ 当前限制和已知问题

### 1. Socket 创建集成（未完成）
**问题：**
- 当前测试假设 Socket 已经正确创建
- 实际 Socket 需要使用 `WSASocket` + `WSA_FLAG_OVERLAPPED`

**后续：**
- 需要修改 `channel.cpp` 中的 Socket 创建代码
- 验证异步 Socket 创建

### 2. 真实网络场景（未完成）
**问题：**
- 当前测试使用空的完成队列
- 没有真实的网络 I/O

**后续：**
- 需要创建模拟 Socket
- 测试真实的异步收发

### 3. 资源清理（未完成）
**问题：**
- `cleanupOverlappedOperations()` 只是占位符
- 可能存在 Overlapped 结构泄漏

**后续：**
- 实现完整的资源清理逻辑
- 添加内存泄漏检测

---

## 📈 性能基准

### 预期性能指标

| 指标 | 目标 | 当前状态 |
|------|------|---------|
| **平均延迟** | < 100μs | ✅ 测试已实现 |
| **最大延迟** | < 1ms | ✅ 测试已实现 |
| **吞吐量** | > 1000 事件/秒 | ✅ 测试已实现 |
| **并发连接** | > 1000 | ⚠️ 需要真实网络测试 |
| **内存泄漏** | 无 | ⚠️ 框架已实现，待验证 |

### 与 Select 模式对比

| 场景 | Select | IOCP（预期） |
|------|--------|-------------|
| 100 连接 | 良好 | 良好 |
| 1000 连接 | ⚠️ 性能下降 | ✅ 良好 |
| 5000 连接 | ❌ 不可用 | ✅ 良好 |
| 10000+ 连接 | ❌ 不可用 | ✅ 优秀 |

---

## 🔧 调试建议

### 1. 编译时启用诊断
```cmake
# CMakeLists.txt 添加
target_compile_definitions(kbe_smoke_tests PRIVATE
    _DEBUG
    CRTDBG_MAP_ALLOC
)
```

### 2. 运行时内存检测
```bash
# Windows 调试模式
ctest --preset vcpkg -R "Iocp" --verbose

# 使用 Visual Studio 的内存泄漏检测
# 在 VS 中启用 CRT 调试堆
```

### 3. 日志输出
```cpp
// 在测试中添加日志
INFO_MSG(fmt::format("IOCP Poller created: {}", (void*)poller));
INFO_MSG(fmt::format("Completion port handle: {}", (void*)hCompletionPort_));
```

---

## 🐛 已知问题和解决方案

### 问题 1：测试失败 - 创建失败
**症状：** `ASSERT_NE(poller, nullptr)` 失败

**可能原因：**
- IOCP 创建失败（`CreateIoCompletionPort` 失败）
- 系统资源不足

**解决：**
- 检查系统资源
- 查看错误日志
- 验证 Windows SDK 版本

### 问题 2：性能测试超时
**症状：** 测试运行时间过长

**可能原因：**
- 超时设置不合理
- 系统负载高

**解决：**
- 减少测试迭代次数
- 检查系统负载

### 问题 3：多线程测试崩溃
**症状：** 并发测试时崩溃

**可能原因：**
- 线程同步问题
- IOCP 线程安全问题

**解决：**
- 检查 `cleanupOverlappedOperations()` 实现
- 添加线程同步机制

---

## 📝 下一步工作

1. ✅ **基础测试框架** - 已完成
2. ⚠️ **Socket 集成测试** - 待实现
3. ⚠️ **真实网络场景测试** - 待实现
4. ⚠️ **Overlapped I/O 实现** - 待完成
5. ⚠️ **资源清理实现** - 待完成

---

**创建日期**: 2026-04-03
**分支**: feature/windows-iocp-network
**提交**: 67bad6c6b
