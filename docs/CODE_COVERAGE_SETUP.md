# 代码覆盖率配置指南

## 📋 概述

本文档说明 KBEngine 项目的代码覆盖率配置和故障排除。

## 🔧 配置详情

### CMake 配置

在 `CMakeLists.txt` 中添加了覆盖率支持：

```cmake
if(KBE_ENABLE_CODE_COVERAGE)
  # 添加覆盖率编译标志
  add_compile_options(-fprofile-arcs -ftest-coverage)
  add_link_options(-fprofile-arcs)

  # 禁用优化以确保准确覆盖率
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

  # 添加覆盖率定义
  add_compile_definitions(GCOV_PREFIX=$<CONFIG:CFG> __NO_INLINE__)
endif()
```

### GitHub Actions 配置

覆盖率任务分为几个步骤：

1. **安装依赖**: gcc, lcov
2. **配置**: 使用 `-DKBE_ENABLE_CODE_COVERAGE=ON`
3. **构建**: 构建测试目标
4. **运行测试**: 生成 .gcda 文件
5. **收集覆盖率**: 使用 lcov 收集数据
6. **上传报告**: 上传到 Codecov

## 🚨 常见问题

### 问题 1: "no .gcda files found"

**原因**:
- 测试没有运行
- 编译时没有添加覆盖率标志
- .gcda 文件在错误的目录

**解决方案**:
```bash
# 确保测试运行
ctest --preset vcpkg --output-on-failure

# 检查编译标志
cmake --preset vcpkg -DKBE_ENABLE_CODE_COVERAGE=ON

# 查找 .gcda 文件
find build -name "*.gcda"
```

### 问题 2: 覆盖率数据为空

**原因**:
- 只收集了测试代码，没有收集源代码
- 过滤规则太严格

**解决方案**:
```bash
# 检查覆盖率数据
lcov --list coverage.info

# 尝试更宽松的收集
lcov --capture --directory . --output-file coverage.info
```

### 问题 3: vcpkg 依赖库影响覆盖率

**原因**:
- vcpkg 安装的第三方库包含在覆盖率
- 第三方库可能没有编译覆盖率支持

**解决方案**:
```bash
# 只收集项目源代码
lcov --capture --directory build/presets/vcpkg --base-directory . --output-file coverage.info

# 排除第三方库
lcov --remove coverage.info '*/vcpkg/*'
lcov --remove coverage.info 'kbe/src/lib/dependencies/*'
lcov --remove coverage.info 'build/*' --output-file coverage.info
```

## 🎯 最佳实践

### 开发环境测试

```bash
# 配置项目
cmake --preset vcpkg -DKBE_ENABLE_CODE_COVERAGE=ON

# 构建测试
cmake --build --preset vcpkg --target kbe_smoke_tests

# 运行测试
cd build/presets/vcpkg
ctest -C Debug --output-on-failure

# 收集覆盖率
lcov --capture --directory kbe/src --output-file coverage.info
lcov --list coverage.info

# 生成 HTML 报告
genhtml coverage.info --output-directory coverage_html
```

### CI/CD 环境注意事项

1. **测试运行**: 确保 `ctest` 在收集覆盖率之前运行
2. **工作目录**: 注意相对路径，使用正确的工作目录
3. **忽略错误**: 使用 `--ignore-errors empty` 处理空目录

## 📊 覆盖率目标

| 指标 | 目标 | 当前状态 |
|------|------|----------|
| 项目覆盖率 | 80% | 🟡 待建立 |
| 补丁覆盖率 | 75% | 🟡 待建立 |
| CI 集成 | ✅ | 已配置 |

## 🔧 故障排除命令

### 检查编译标志
```bash
# 检查 CMAKE_CXX_FLAGS
cmake --preset vcpkg -DKBE_ENABLE_CODE_COVERAGE=ON --trace | grep -i coverage

# 检查编译器标志
cat build/presets/vcpkg/flags.make | grep COVERAGE
```

### 验证测试运行
```bash
# 运行单个测试
./tests/bin/Debug/kbe_smoke_tests.exe --gtest_filter="OpenSSLRSACompatTest.HandlesBinaryData"

# 运行所有测试
./tests/bin/Debug/kbe_smoke_tests.exe
```

### 调试覆盖率收集
```bash
# 检查 .gcda 文件
find build -name "*.gcda" -ls

# 手动收集覆盖率
lcov --capture --directory build/presets/vcpkg --base-directory . --output-file coverage.info

# 检查编译标志
grep -r "ftest-coverage" build/presets/vcpkg/CMakeFiles/ | head -5
```

### CI/CD 环境故障排除

**问题 4: CI 中覆盖率收集失败**

**原因**:
- 工作目录配置错误
- .gcda 文件生成位置不正确
- CMake 生成器配置问题

**解决方案**:

1. **确认文件位置**:
```yaml
- name: Find .gcda files
  run: |
    find build -name "*.gcda" -type f -ls
    find build -name "*.gcno" -type f | wc -l
```

2. **验证覆盖率标志**:
```yaml
- name: Verify Coverage Flags
  run: |
    grep -i "coverage\|ftest-coverage" build/presets/vcpkg/CMakeCache.txt
    grep -r "ftest-coverage" build/presets/vcpkg/CMakeFiles/ | head -5
```

3. **修正 lcov 命令**:
```yaml
- name: Generate Coverage Report
  run: |
    lcov --capture \
      --directory build/presets/vcpkg \
      --base-directory . \
      --output-file coverage.info \
      --ignore-errors empty \
      --ignore-errors source
```

4. **检查测试是否实际运行**:
```yaml
- name: Run Tests
  run: |
    ctest --output-on-failure || true  # 继续运行以查看覆盖率
    find . -name "*.gcda" -type f -ls
```

## 📈 改进建议

1. **单元测试覆盖率**: 专注于核心业务逻辑
2. **集成测试**: 覆盖关键路径和边界情况
3. **代码质量**: 结合静态分析工具
4. **持续监控**: 定期检查覆盖率趋势

---

**注意**: 覆盖率是代码质量的辅助指标，不应成为唯一目标。重点是编写有意义的测试，而不是单纯追求高覆盖率数字。
