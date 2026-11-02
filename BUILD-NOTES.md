# KBEngine Linux 构建与起服测试记录

- 仓库 commit: `a3c481d`（用户修改前版本，worktree: `/home/cui/workspaces/kbengine-build`）
- 目标引擎 Python: 3.7
- 日期: 2026-11-02

## 阶段 0：环境准备

### 本机初始环境（Ubuntu 26.04.1 LTS resolute）

| 组件 | 初始状态 | 处理 |
|---|---|---|
| gcc / g++ | 15.2.0 | 暂保留，报错则装 g++-12 切换 |
| cmake | 4.2.3 | 保留（CMakeLists 要求 >=3.21，兼容） |
| ninja | 已装 `/usr/bin/ninja` | 无需处理 |
| python3 | 3.14.9（默认） | 需换 python3.7 |
| python3.7 | 未装 | **已装 deadsnakes: `python3.7 3.7.17-1+resolute1` + `python3.7-dev`** |
| tinyxml2 | 未装 | **已装 `libtinyxml2-dev`**（`find_package(tinyxml2 CONFIG REQUIRED)` 必需） |
| zlib / curl / openssl | 已装 | 无需处理 |
| libmysqlclient | 已装 8.4.11 dev | 无需处理 |

### Python 链接方式（kbe/src 确认）

- 顶层 `CMakeLists.txt` → `include(KBEngineDependencies)` → `find_package(Python3 REQUIRED COMPONENTS Development)`。
- `kbe/src/lib/CMakeLists.txt` 中 `kbe_pyscript` 链接 `Python3::Python`（第 1838 行）。
- 故用 `-DPython3_EXECUTABLE=/usr/bin/python3.7` 强制 CMake 使用 Python 3.7。
- 源码 `lib/pyscript/script.cpp` 含 `PY_VERSION_HEX >= 0x030B0000` 条件分支，对 3.11+ 有兼容处理，但本任务按 Python 3.7 目标执行。

### 构建目标（kbe/src/CMakeLists.txt 定义的 executable）

`machine` `baseappmgr` `baseapp` `cellappmgr` `cellapp` `loginapp` `dbmgr` `interfaces` `logger` `kbcmd` `bots`

任务要求：machine/loginapp/dbmgr/baseapp/cellapp/cellappmgr/logger。

### 非 vcpkg 模式依赖说明（cmake/KBEngineDependencies.cmake）

- 非 vcpkg 模式使用 vendored `fmt`、`utf8cpp`(头文件)、`tmxparser`、`hiredis`。
- 系统需提供：`tinyxml2`（CONFIG 包）、`ZLIB`、`CURL`、`OpenSSL`（KBE_USE_OPENSSL=ON 默认）、`Python3`、MySQL 客户端（`find_library` 找 `mariadb/mysqlclient`）。
