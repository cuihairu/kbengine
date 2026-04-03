# Windows IOCP 网络模式实现说明

## 📋 当前实现状态

### ✅ 已完成
1. **创建 IOCP Poller 基础框架**
   - `poller_iocp.h` - IOCP Poller 头文件
   - `poller_iocp.cpp` - IOCP Poller 实现
   - 修改 `event_poller.cpp` - 支持 Windows IOCP 路径
   - 更新 `CMakeLists.txt` - 编译配置

2. **平台优先级**
   - Windows → IOCP (新增)
   - Linux → Epoll (已有)
   - 其他 → Select (已有)

### ⚠️ 当前限制（未完成部分）

#### 1. **异步 Socket 创建（重要）**
当前 IOCP 实现假设 Socket 已经使用异步模式创建，但实际创建 Socket 的代码可能在其他地方。

**需要修改的位置：**
```cpp
// 可能在 channel.cpp 或 packet_receiver.cpp 中
// Socket 创建需要改为：
SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                      NULL, 0, WSA_FLAG_OVERLAPPED);
```

#### 2. **Overlapped 结构管理**
当前实现中的 `cleanupOverlappedOperations()` 只是占位符。

**需要实现：**
- Overlapped 结构的生命周期管理
- 异步操作完成后的资源释放
- Socket 关闭时的清理逻辑

#### 3. **完成队列处理优化**
当前的 `processPendingEvents()` 实现比较简陋。

**需要优化：**
- 正确解析 completionKey（用于区分读/写/错误操作）
- 错误处理和重试机制
- 性能优化（减少轮询开销）

#### 4. **与现有 Channel 的集成**
需要确保 KBEngine 的网络通道层能正确使用 IOCP。

**需要验证的文件：**
- `channel.cpp` - 通道实现
- `tcp_packet_receiver.cpp` - TCP 接收
- `tcp_packet_sender.cpp` - TCP 发送
- `packet_reader.cpp` - 数据包读取

---

## 🚧 实现步骤（待完成）

### 阶段 1：基础验证（当前阶段）
- [x] 创建 IOCP Poller 类
- [x] 修改平台判断逻辑
- [x] 更新 CMake 配置
- [ ] 编译测试
- [ ] 基础功能测试

### 阶段 2：异步 Socket 集成
- [ ] 修改 Socket 创建代码使用 `WSASocket`
- [ ] 确保 `WSA_FLAG_OVERLAPPED` 标志
- [ ] 测试异步 Socket 创建

### 阶段 3：Overlapped I/O 实现
- [ ] 定义 Per-Operation Data 结构
- [ ] 实现异步读（WSARecv）
- [ ] 实现异步写（WSASend）
- [ ] 处理完成通知

### 阶段 4：错误处理和清理
- [ ] 实现取消逻辑（CancelIoEx）
- [ ] Socket 关闭时的资源清理
- [ ] 错误重试机制

### 阶段 5：性能优化
- [ ] 优化完成队列处理
- [ ] 线程池管理
- [ ] 性能测试和对比

---

## 🔧 编译和测试

### 编译
```bash
# Windows (Visual Studio)
cmake --preset vcpkg
cmake --build build/presets/vcpkg

# 或者使用构建脚本
build.bat
```

### 验证 IOCP 被使用
```cpp
// 在启动时添加日志
INFO_MSG(fmt::format("Using poller: {}",
#ifdef HAS_IOCP
    "IOCP"
#elif defined(HAS_EPOLL)
    "EPOLL"
#else
    "SELECT"
#endif
));
```

---

## 📊 性能预期

### Select vs IOCP

| 指标 | Select | IOCP (预期) |
|------|--------|-------------|
| 连接数限制 | ~1000 | 无限制 |
| CPU 使用率（高并发） | 高 | 低 |
| 延迟 | 中等 | 低 |
| 内存占用 | 低 | 中等 |

---

## ⚠️ 注意事项

1. **仅在 Windows 平台启用**
   - IOCP 是 Windows 特有的 API
   - Linux 继续使用 epoll

2. **需要 Windows SDK**
   - 确保安装了 Windows SDK
   - 链接 `ws2_32.lib`

3. **兼容性**
   - Windows XP+ 都支持 IOCP
   - 但建议 Windows 7+

4. **调试**
   - IOCP 调试比 select/epoll 更复杂
   - 建议使用少量连接进行初期测试

---

## 🔗 参考资料

- [Microsoft IOCP Documentation](https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports)
- [Windows Sockets 2.0](https://docs.microsoft.com/en-us/windows/win32/winsock/sockets-2)
- [KBEngine 网络架构](https://github.com/kbengine/kbengine)

---

## 🐛 已知问题

1. **当前代码未经测试**
   - 框架代码已实现
   - 需要实际编译和测试

2. **与 Channel 层的集成未知**
   - 可能需要修改 Channel 实现
   - 需要验证数据包收发流程

3. **Overlapped 结构管理未实现**
   - 当前只是占位符
   - 可能导致内存泄漏

---

## 📝 下一步

1. **编译测试** - 确保代码能编译通过
2. **基础功能测试** - 测试简单的连接和收发
3. **性能测试** - 对比 select 模式的性能
4. **完善实现** - 补充缺失的部分

---

**创建日期**: 2026-04-03
**分支**: feature/windows-iocp-network
**状态**: 🚧 开发中
