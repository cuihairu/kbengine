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

## 阶段 1：构建

### 坑 1：vendored 依赖源码缺失（fmt/hiredis/utf8cpp 被 gitignore）

- 现象：CMake 报 `Cannot find source file .../dependencies/fmt/src/format.cc`、`.../hiredis/hiredis.c`。
- 根因：`kbe/src/lib/dependencies/` 下 `fmt`/`hiredis`/`utf8cpp` 目录被 `.gitignore` 排除（CI 依赖 vcpkg 提供这些包），源码树不含这些文件。
- 解决：按 CMake 预期布局手工放置（这些目录 gitignore，不入库）：
  - `fmt` → 下载 `fmtlib/fmt` v12.1.0，放 `src/{format.cc,os.cc,fmt.cc}` + `include/fmt/`
  - `hiredis` → 下载 `redis/hiredis` v1.2.0，放根目录全部 `.c/.h`
  - `utf8cpp` → 下载 `nemtrif/utfcpp` v3.2.5，把 `source/utf8.h` 放 `dependencies/utf8cpp/utf8.h`（include 路径是 `dependencies/`，代码 `#include "utf8cpp/utf8.h"`）

### 坑 2：FindPython3 忽略 Python3_EXECUTABLE（Development 组件不用解释器）

- 现象：`-DPython3_EXECUTABLE=/usr/bin/python3.7` 无效，CMake 仍找到 Python 3.14，并警告该变量未使用。
- 根因：项目只 `find_package(Python3 COMPONENTS Development)`（不请求 Interpreter），FindPython3 用库文件/头文件确定版本，不读 `Python3_EXECUTABLE`。
- 解决：直接指定库与头文件：
  `-DPython3_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.7m.so -DPython3_INCLUDE_DIR=/usr/include/python3.7m`
  → 成功 `Found Python3: /usr/include/python3.7m (version 3.7.17)`。

### 坑 3：hiredis 1.2.0 需额外源文件

- 现象：`dbmgr` 链接失败 `undefined reference to redisAsyncRead/redisAsyncWrite`。
- 根因：hiredis 1.x 把 `hi_malloc` 等分配函数拆到 `alloc.c`、dict 到 `dict.c`、异步 API 到 `async.c`；KBEngine CMake 原只列 `hiredis.c/net.c/read.c/sds.c`。
- 解决：`cmake/KBEngineDependencies.cmake` 的 `kbe_dependency_hiredis` 追加 `dict.c`、`alloc.c`、`async.c`（本仓库已修改提交）。

### 构建结果

- 编译器：gcc/g++ 15.2 直接通过，**无需切换 g++-12**。
- CMake 4.2.3 + Ninja，`CMAKE_BUILD_TYPE=Release`。
- 配置命令：
  `cmake -S . -B build/presets/linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.7m.so -DPython3_INCLUDE_DIR=/usr/include/python3.7m -DKBE_ENABLE_TESTING=OFF`
- 产物（`kbe/bin/server/`）：`machine` `baseappmgr` `baseapp` `cellappmgr` `cellapp` `dbmgr` `loginapp` `logger` `interfaces` 共 9 个，全部构建成功。
- 链接验证：`dbmgr` 依赖 `libpython3.7m.so.1.0` + `libmysqlclient.so.24`；baseapp/cellapp/loginapp/logger 依赖 `libpython3.7m.so.1.0`。
