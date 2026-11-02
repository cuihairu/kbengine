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

### 坑 4：SignalHandlers 单例从未实例化 → machine 启动段错误

- 现象：`machine` 启动立即 SIGSEGV，gdb 回溯在 `SignalHandlers::attachApp`，寄存器 `rdi=0`（`this` 为 NULL）。
- 根因：`g_kbeSignalHandlers = SignalHandlers::getSingleton()`，但 `SignalHandlers` 单例指针只在构造函数里赋值，全仓库无任何 `SignalHandlers` 对象被构造（上游 KBEngine 亦然）；仓库现代化后该实例化缺失，`getSingleton()` 返回空引用。
- 解决：`kbe/src/lib/server/serverapp.cpp` 的 `ServerApp::installSignals()` 首用前补 `new SignalHandlers();`（与仓库其它单例 `new Resmgr()`/`new DebugHelper()` 惯例一致）。此修复覆盖所有组件。

### 坑 5：NetworkInterface 断言"no available port"

- 现象：修好坑 4 后再起 machine，`NetworkInterface` 构造 `myassert` 中止。
- 根因：**并非代码 bug**——前一个 `machine` 实例已正常绑定 external 端口 20099 并存活，第二个实例绑定失败属正常端口冲突。
- 结论：machine 单实例即可；起服前先 `pkill -f kbe/bin/server` 清理残留。

### 坑 6：char2wchar 把字节数当宽字符数传给 mbstowcs（FORTIFY 检测溢出）

- 现象：`logger`/`interfaces` 启动立即 `*** buffer overflow detected ***`（glibc FORTIFY）并 abort。
- 根因：`kbe/src/lib/common/strutil_core.cpp` 的 `char2wchar()`：
  ```cpp
  int len = (int)((strlen(cs) + 1) * sizeof(wchar_t));  // 字节数
  wchar_t* ccattr = (wchar_t*)malloc(len);              // 缓冲区 len 字节
  size_t slen = mbstowcs(ccattr, cs, len);              // BUG: 第三参是"最大宽字符数"，却传字节数
  ```
  `mbstowcs` 第三参数语义是"最多写多少个宽字符"。传 `len`（字节数）意味着最多写 `len` 个宽字符 = `len*4` 字节，超出缓冲区 4 倍 → 溢出。
- 注意：`strutil.cpp` 同名函数**不参与编译**（`kbe_common_strutil` 目标只编 `strutil_core.cpp`），改错文件无效。
- 解决：`strutil_core.cpp` 改为 `mbstowcs(ccattr, cs, strlen(cs) + 1)`（缓冲区恰好容纳 `strlen+1` 个宽字符）。已提交 `4f81d38`。

### 坑 7：嵌入式 Python 3.7 找不到 encodings（stdlib 未进入模块搜索路径）

- 现象：坑 6 修复后 `logger` 又 abort：`Fatal Python error: initfsencoding: Unable to get the locale encoding / ModuleNotFoundError: No module named 'encodings'`。
- 根因（两层）：
  1. 引擎 `Script::install()`（`lib/pyscript/script.cpp`）在 Python 3.7 分支设 `Py_IgnoreEnvironmentFlag=1` 并调用 `Py_SetPath(pyPaths)`——**Python 的模块搜索路径被完全替换为引擎脚本目录**，`sys.path` 里没有任何含 stdlib 的目录，`import encodings` 必然失败。
  2. `SCRIPT_PATH` 宏（`lib/pyscript/script.h`）把 `kbe/res/scripts/common`（及 `lib-dynload`/`DLLs`/`Lib` 等子目录）替换真实 res 路径后并入 `pyPaths`。即**KBEngine 期望 `kbe/res/scripts/common/` 本身就是 stdlib 根目录**（Windows 式捆绑 Python 布局：顶层 .py + `encodings/` + `lib-dynload/` 直接在该目录下）。系统发行版 Python（deadsnakes python3.7，stdlib 在 `/usr/lib/python3.7`）不符合该布局。
- 解决（运行期部署物，不涉及源码）：
  `kbe/res/scripts/common/` 下对 `/usr/lib/python3.7` 的每个顶层条目建立软链（`encodings`、`os.py`、`codecs.py`、`lib-dynload`、`Lib` 等）。该目录的 `.gitignore` 本就忽略除自身外的所有内容，故软链不入库。
- 验证：`PYTHONHOME=<kbe/res/scripts/common> python3.7 -c "import encodings"` 成功；`logger` 修复后正常启动。

### 构建结果

- 编译器：gcc/g++ 15.2 直接通过，**无需切换 g++-12**。
- CMake 4.2.3 + Ninja，`CMAKE_BUILD_TYPE=Release`。
- 配置命令：
  `cmake -S . -B build/presets/linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.7m.so -DPython3_INCLUDE_DIR=/usr/include/python3.7m -DKBE_ENABLE_TESTING=OFF`
- 产物（`kbe/bin/server/`）：`machine` `baseappmgr` `baseapp` `cellappmgr` `cellapp` `dbmgr` `loginapp` `logger` `interfaces` 共 9 个，全部构建成功。
- 链接验证：`dbmgr` 依赖 `libpython3.7m.so.1.0` + `libmysqlclient.so.24`；baseapp/cellapp/loginapp/logger 依赖 `libpython3.7m.so.1.0`。

## 阶段 2：起服测试

### 启动方式

- 工作目录：`/home/cui/workspaces/kbengine-build/assets`（`python_assets` 拷贝，`res/server/kbengine.xml` 已配置 DB：`192.168.5.5:3306` root / `kbengine_root_pw_2026`，`databaseName=kbengine`，encrypt=false；各组件 `internalInterface=192.168.5.188`）。
- 环境：`KBE_RES_PATH=kbe/res/:assets:assets/res:assets/scripts/`、`KBE_BIN_PATH=kbe/bin/server/`、`KBE_ROOT=kbengine-build`。
- 顺序：`machine → logger → interfaces → dbmgr → baseappmgr → cellappmgr → baseapp → cellapp → loginapp`（均后台 `nohup`，日志 `/tmp/kbelogs/*.log`）。

### 结果

- **9 个进程全部存活**（pid 见下），连续观察稳定无退出：
  `machine 2755810`、`logger 2844876`、`interfaces 2848458`、`dbmgr 2850246`、`baseappmgr 2853859`、`cellappmgr 2853860`、`baseapp 2855761`、`cellapp 2858322`、`loginapp 2860375`。
- **MySQL 表已创建**（192.168.5.5:3306 库 `kbengine`）：`kbe_accountinfos`、`kbe_email_verification`、`kbe_entitylog`、`kbe_serverlog`、`tbl_Account`。
- **日志管线打通**：`kbe_serverlog` 已有心跳行（`heartbeatTime=1793610046, serverGroupID=1000`），证明 dbmgr→logger→MySQL 写入正常。
- 本地日志文件无 ERROR/CRITICAL 内容（仅调试用 strace 文件含预期 abort 记录）。

### 阻塞点（全部已解决）

| 阻塞 | 根因 | 解决 |
|---|---|---|
| machine SIGSEGV | SignalHandlers 单例未实例化（已提交 `ada6418`） | `new SignalHandlers()` in `installSignals()` |
| logger/interfaces buffer overflow | char2wchar 传字节数给 mbstowcs（已提交 `4f81d38`） | `mbstowcs(ccattr, cs, strlen(cs)+1)` |
| logger "No module named encodings" | Py_SetPath 限定搜索路径，stdlib 不在其中（坑 7） | stdlib 软链入 `kbe/res/scripts/common/` |
| 起服前端口冲突 | 旧 machine 实例仍占 20099 | 先 `pkill -f kbe/bin/server` 清理 |
