# 覆盖率与 Codecov

> 这页整合原来的 `CODECOV_SETUP.md`、`CODE_COVERAGE_SETUP.md`、`COVERAGE_IMPROVEMENTS.md`，统一记录覆盖率配置、CI 收集方式和历史问题修复。

## 目标

- 本地和 CI 都能稳定生成覆盖率数据。
- `lcov` 从正确的构建目录收集，而不是从源码目录误采集。
- Codecov 上传链路可诊断、可复现、可维护。

## 配置分层

### 1. CMake 侧

- 使用 `KBE_ENABLE_CODE_COVERAGE` 打开覆盖率编译选项。
- 编译时启用 `-fprofile-arcs -ftest-coverage`。
- Debug 构建关闭优化，避免覆盖率行号失真。
- 多线程场景建议使用 `-fprofile-update=atomic`，减少计数器竞态。

### 2. 测试执行侧

- 先构建测试目标，再运行 `ctest` 生成 `.gcda`。
- Ninja 单配置生成器下，不应依赖 `ctest -C Debug` 这类多配置写法。
- 即使测试失败，也应保留必要调试输出，确认 `.gcda` 是否生成。

### 3. 收集与上传侧

- `lcov` 应从 `build/presets/vcpkg` 一类构建目录收集。
- 使用 `--base-directory .` 映射回源码路径。
- 上传到 Codecov 前，先过滤第三方依赖和无关目录。
- 公开仓库通常不需要 `CODECOV_TOKEN`；私有仓库需要单独配置。

## 推荐流程

```bash
cmake --preset vcpkg -DKBE_ENABLE_CODE_COVERAGE=ON
cmake --build --preset vcpkg --target kbe_smoke_tests
ctest --preset vcpkg --output-on-failure
lcov --capture --directory build/presets/vcpkg --base-directory . --output-file coverage.info
lcov --list coverage.info
```

## CI 关键点

- 构建后先验证覆盖率编译标志是否生效。
- 测试后立即检查 `.gcda` / `.gcno` 文件数量和位置。
- `lcov` 收集时显式传入错误忽略参数，避免已知的 C++ 行号映射问题直接打断流程。
- 如果 `coverage.info` 为空，要输出诊断信息，而不是静默失败。

## 常见问题

### 没有 `.gcda` 文件

优先检查：

- 测试是否真的执行了。
- 编译时是否打开 `KBE_ENABLE_CODE_COVERAGE`。
- `lcov` 是否从构建目录而不是源码目录收集。

### `geninfo: ERROR: mismatched end line`

这通常来自模板函数、内联函数或测试框架展开后的行号映射问题。当前处理思路是保留数据收集，同时用 `--ignore-errors mismatch` 等参数降低噪音。

### `geninfo: ERROR: Unexpected negative count`

这通常和多线程覆盖率计数器更新竞态有关。优先使用 `-fprofile-update=atomic`，必要时再用 `--ignore-errors negative` 兜底。

### Codecov 上传失败

- 公开仓库先检查 GitHub Actions 输出，不要默认认为缺 token。
- 私有仓库再补 `CODECOV_TOKEN`。
- 上传前确认 `coverage.info` 不是空文件。

## 当前结论

- 覆盖率链路的核心问题不是 Codecov，而是“编译标志是否生效”和“`lcov` 是否从正确目录收集”。
- `CODECOV_SETUP.md` 更适合作为 Codecov 使用补充，而不应和覆盖率收集实现细节平行存在。
- `COVERAGE_IMPROVEMENTS.md` 属于历史修复记录，其有效内容已经收敛到本页。

## 兼容页面

- [/CODECOV_SETUP.md](/CODECOV_SETUP.md)
- [/CODE_COVERAGE_SETUP.md](/CODE_COVERAGE_SETUP.md)
- [/COVERAGE_IMPROVEMENTS.md](/COVERAGE_IMPROVEMENTS.md)
